#include <ShaderProgram.hpp>
#include <Viewer.hpp>

#include <ShaderProgram.hpp>

#include <QuadMeshRenderable.hpp>
#include <FrameRenderable.hpp>

#include <dynamics/DynamicSystemRenderable.hpp>
#include <dynamics/DampingForceField.hpp>
#include <dynamics/ConstantForceField.hpp>
#include <dynamics/SpringForceField.hpp>
#include <dynamics/EulerExplicitSolver.hpp>

#include <dynamics/ParticleRenderable.hpp>
#include <dynamics/ParticleListRenderable.hpp>
#include <dynamics/ConstantForceFieldRenderable.hpp>
#include <dynamics/SpringForceFieldRenderable.hpp>
#include <dynamics/SpringListRenderable.hpp>
#include <dynamics/ControlledForceFieldRenderable.hpp>

void particles(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr &systemRenderable);
void springs(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr &systemRenderable);
void playPool(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr& systemRenderable);
void collisions(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr &systemRenderable);

void initialize_scene( Viewer& viewer )
{
    //Set up a shader and add a 3D frame.
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl", 
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    viewer.addShaderProgram( flatShader );
    FrameRenderablePtr frame = std::make_shared<FrameRenderable>(flatShader);
    viewer.addRenderable(frame);

    //Initialize a dynamic system (Solver, Time step, Restitution coefficient)
    DynamicSystemPtr system = std::make_shared<DynamicSystem>();
    EulerExplicitSolverPtr solver = std::make_shared<EulerExplicitSolver>();
    system->setSolver(solver);
    system->setDt(0.01);

    //Create a renderable associated to the dynamic system
    //This renderable is responsible for calling DynamicSystem::computeSimulationStep() in the animate() function
    //It is also responsible for some of the key/mouse events
    DynamicSystemRenderablePtr systemRenderable = std::make_shared<DynamicSystemRenderable>(system);
    viewer.addRenderable(systemRenderable);

    //Populate the dynamic system with particles, forcefields and create renderables associated to them for visualization.
    //Uncomment only one of the following line

    //particles(viewer, system, systemRenderable);
    //springs(viewer, system, systemRenderable);
    //collisions(viewer, system, systemRenderable);
    playPool(viewer, system, systemRenderable);

    //Finally activate animation
    viewer.startAnimation();
    // Set up a shader and add a 3D frame.
}

int main() 
{
    glm::vec4 background_color(0.7,0.7,0.7,1);
	Viewer viewer(1280,720,background_color);
	initialize_scene(viewer);

	while( viewer.isRunning() )
	{
		viewer.handleEvent();
		viewer.animate();
		viewer.draw();
		viewer.display();
	}	

	return EXIT_SUCCESS;
}

void particles(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr &systemRenderable)
{
    //Initialize a shader for the following renderables
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    viewer.addShaderProgram( flatShader );

    //We diminish the time step to be able to see what happens before particles go too far
    system->setDt(5e-4);

    //Deactivate collision detection
    system->setCollisionsDetection(false);

    glm::vec3 px,pv;
    float pm, pr;
    //Particles with gravity and damping
    {
        //Initialize a particle with position, velocity, mass and radius and add it to the system
        px = glm::vec3(0.0,1.0,0.0);
        pv = glm::vec3(3.0,0.0,0.0);
        pr = 0.1;
        pm = 1.0;
        ParticlePtr particle1 = std::make_shared<Particle>( px, pv, pm, pr);
        system->addParticle( particle1 );

        px = glm::vec3(0.0,1.5,0.0);
        pv = glm::vec3(6.0,0.0,0.0);
        pr = 0.1;
        pm = 1.0;
        ParticlePtr particle2 = std::make_shared<Particle>( px, pv, pm, pr);
        system->addParticle( particle2 );

        //Initialize a force field that apply to all the particles of the system to simulate gravity
        //Add it to the system as a force field
        ConstantForceFieldPtr gravityForceField = std::make_shared<ConstantForceField>(system->getParticles(), DynamicSystem::gravity );
        system->addForceField( gravityForceField );

        //Create a particleRenderable for each particle of the system
        //DynamicSystemRenderable act as a hierarchical renderable
        //This which allows to easily apply transformation on the visualiazation of a dynamicSystem
        ParticleRenderablePtr particleRenderable1 = std::make_shared<ParticleRenderable>(flatShader, particle1);
        HierarchicalRenderable::addChild( systemRenderable, particleRenderable1 );
        ParticleRenderablePtr particleRenderable2 = std::make_shared<ParticleRenderable>(flatShader, particle2);
        HierarchicalRenderable::addChild( systemRenderable, particleRenderable2 );
    }
}


void springs(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr &systemRenderable)
{
    //Initialize a shader for the following renderables
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    ShaderProgramPtr instancedShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/instancedVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/instancedFragment.glsl");
    viewer.addShaderProgram( flatShader );
    viewer.addShaderProgram( instancedShader );

    //Initialize particle attributes (radius, mass)
    float pr = 0.1, pm = 10.0;
    glm::vec3 px(0.0,0.0,0.0), pv(0.0,0.0,0.0);

    //Create particles on a squared uniform grid starting at origin
    std::vector<ParticlePtr> particles;
    glm::vec3 origin(0.25,0,0.25), displacement(0.0,0.0,0.0);
    int particlePerLine = 11;
    float gridWidth=4.0, gridHeight=4.0;
    float xstep = gridWidth / (float)(particlePerLine-1);
    float zstep = gridHeight / (float)(particlePerLine-1);
    particles.resize(particlePerLine*particlePerLine);
    for( size_t i = 0; i < particlePerLine; ++ i )
    {
        for( size_t j = 0; j < particlePerLine; ++ j )
        {
            displacement = glm::vec3(i*xstep, 0.0, j*zstep);
            px = origin + displacement;
            particles[i*particlePerLine+j] = std::make_shared<Particle>( px, pv, pm, pr );
            system->addParticle( particles[i*particlePerLine+j] );
        }
    }

    //Fix particles on the y-borders
    for( size_t j = 0; j < particlePerLine; ++ j )
    {
        particles[0*particlePerLine+j]->setFixed( true );
        particles[10*particlePerLine+j]->setFixed( true );
    }

    //Initialize springs attributes (stiffness, rest length, damping)
    float stiffness = 4e3, l0 = gridWidth / (particlePerLine-1), damping =10.0;

    //Create springs between particles of the grid, horizontally and vertically
    //Store them in a list
    std::list<SpringForceFieldPtr> springForceFields;
    //Interior
    for( size_t i = 1; i < particlePerLine; ++ i )
    {
        for( size_t j = 1; j < particlePerLine; ++ j )
        {
            SpringForceFieldPtr spring1 = std::make_shared<SpringForceField>( particles[(i-1)*particlePerLine+j], particles[i*particlePerLine+j], stiffness, l0, damping );
            springForceFields.push_back(spring1);
            system->addForceField( spring1 );
            SpringForceFieldPtr spring2 = std::make_shared<SpringForceField>( particles[i*particlePerLine+(j-1)], particles[i*particlePerLine+j], stiffness, l0, damping );
            springForceFields.push_back(spring2);
            system->addForceField( spring2 );
        }
    }
    //Border case 1
    for( size_t j = 1; j < particlePerLine; ++ j )
    {
        SpringForceFieldPtr spring = std::make_shared<SpringForceField>( particles[0*particlePerLine+j], particles[0*particlePerLine+(j-1)], stiffness, l0, damping );
        springForceFields.push_back(spring);
        system->addForceField( spring );
    }
    //Border case 2
    for( size_t i = 1; i < particlePerLine; ++ i )
    {
        SpringForceFieldPtr spring = std::make_shared<SpringForceField>( particles[(i-1)*particlePerLine+0], particles[i*particlePerLine+0], stiffness, l0, damping );
        springForceFields.push_back(spring);
        system->addForceField( spring );
    }

    //Initialize a force field that apply to all the particles of the system to simulate gravity
    //Add it to the system as a force field
    ConstantForceFieldPtr gravityForceField = std::make_shared<ConstantForceField>(system->getParticles(), DynamicSystem::gravity  );
    system->addForceField( gravityForceField );

    //Initialize a force field that apply to all the particles of the system to simulate vicosity (air friction)
    float dampingCoefficient = 5.0;
    DampingForceFieldPtr dampingForceField = std::make_shared<DampingForceField>(system->getParticles(), dampingCoefficient);
    system->addForceField(dampingForceField);


    //Create a particleListRenderable to efficiently visualize the particles of the system
    ParticleListRenderablePtr particleListRenderable = std::make_shared<ParticleListRenderable>( instancedShader, particles);
    HierarchicalRenderable::addChild(systemRenderable, particleListRenderable);

    //Create a springListRenderable to efficiently visualize the springs of the system
    SpringListRenderablePtr springsRenderable = std::make_shared<SpringListRenderable>(flatShader, springForceFields);
    HierarchicalRenderable::addChild( systemRenderable, springsRenderable );

    //Display gravity
    ConstantForceFieldRenderablePtr gravityRenderable = std::make_shared<ConstantForceFieldRenderable>(flatShader, gravityForceField);
    HierarchicalRenderable::addChild( systemRenderable, gravityRenderable );
}

void collisions(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr &systemRenderable)
{
    //Initialize a shader for the following renderables
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    viewer.addShaderProgram( flatShader );

    //Activate collision detection
    system->setCollisionsDetection(true);

    //Initialize the restitution coefficient for collision
    //1.0 = full elastic response
    //0.0 = full absorption
    system->setRestitution(1.0f);

    //Initialize a plane from 3 points and add it to the system as an obstacle
    glm::vec3 p1(1.0,0.0,1.0),p2(1.0,0.0,-1.0), p3(-1.0,0.0,-1.0), p4(-1.0,0.0,1.0);
    PlanePtr plane = std::make_shared<Plane>(p1, p2, p3);
    system->addPlaneObstacle(plane);

    //Create a plane renderable to display the obstacle
    QuadMeshRenderablePtr planeRenderable = std::make_shared<QuadMeshRenderable>(flatShader, p1, p2, p3, p4);
    HierarchicalRenderable::addChild( systemRenderable, planeRenderable );

    glm::vec3 px,pv;
    float pm, pr;
    //Particle vs Plane collision
    {

        //Initialize a particle with position, velocity, mass and radius and add it to the system
        px = glm::vec3(0.0,1.0,0.0);
        pv = glm::vec3(0.0,-5.0,0.0);
        pr = 0.1;
        pm = 1.0;
        ParticlePtr particle = std::make_shared<Particle>( px, pv, pm, pr);
        system->addParticle( particle );

        //Create a particleRenderable for each particle of the system
        //DynamicSystemRenderable act as a hierarchical renderable
        //This which allows to easily apply transformation on the visualiazation of a dynamicSystem
        ParticleRenderablePtr particleRenderable = std::make_shared<ParticleRenderable>(flatShader, particle);
        HierarchicalRenderable::addChild( systemRenderable, particleRenderable );
    }

    //Particle vs Particle collision
    {
        //Initialize two particles with position, velocity, mass and radius and add it to the system
        //One of the particle is fixed
        px = glm::vec3(0.5,0.1,0.0);
        pv = glm::vec3(0.0,0.0,0.0);
        pr = 0.1;
        pm = 1000.0;
        ParticlePtr particle1 = std::make_shared<Particle>( px, pv, pm, pr);
        particle1->setFixed(true);
        system->addParticle( particle1 );

        px = glm::vec3(0.5,1.0,0.0);
        pv = glm::vec3(0.0,-0.5,0.0);
        pr = 0.1;
        pm = 1.0;
        ParticlePtr particle2 = std::make_shared<Particle>( px, pv, pm, pr);
        system->addParticle( particle2 );

        //Create a particleRenderable for each particle of the system
        ParticleRenderablePtr particleRenderable1 = std::make_shared<ParticleRenderable>(flatShader, particle1);
        HierarchicalRenderable::addChild( systemRenderable, particleRenderable1 );
        ParticleRenderablePtr particleRenderable2 = std::make_shared<ParticleRenderable>(flatShader, particle2);
        HierarchicalRenderable::addChild( systemRenderable, particleRenderable2 );
    }

    //Initialize a force field that apply to all the particles of the system to simulate gravity
    //Add it to the system as a force field
    ConstantForceFieldPtr gravityForceField = std::make_shared<ConstantForceField>(system->getParticles(), DynamicSystem::gravity );
    system->addForceField( gravityForceField );
}

void playPool(Viewer& viewer, DynamicSystemPtr& system, DynamicSystemRenderablePtr& systemRenderable)
{
    viewer.getCamera().setBehavior(Camera::CAMERA_BEHAVIOR::ARCBALL_BEHAVIOR);
    //Initialize a shader for the following renderables
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    viewer.addShaderProgram( flatShader );

    //Initialize two particles with position, velocity, mass and radius and add it to the system
    glm::vec3 px(0.0,0.0,0.0),pv(0.0,0.0,0.0);
    float pm=1.0, pr=0.5;
    px = glm::vec3(0.0,pr,0.0);
    ParticlePtr mobile = std::make_shared<Particle>( px, pv, pm, pr);
    system->addParticle( mobile );
    px = glm::vec3(0.0,pr,5.0);
    ParticlePtr other = std::make_shared<Particle>( px, pv, pm, pr);
    system->addParticle( other );

    //Create a particleRenderable for each particle of the system
    //Add them to the system renderable
    ParticleRenderablePtr mobileRenderable = std::make_shared<ParticleRenderable>( flatShader, mobile );
    HierarchicalRenderable::addChild(systemRenderable, mobileRenderable);
    ParticleRenderablePtr otherRenderable = std::make_shared<ParticleRenderable>( flatShader, other );
    HierarchicalRenderable::addChild(systemRenderable, otherRenderable);
    float pool_size(5.0), pool_height(2.5);
    //Initialize four planes to create walls arround the particles
    glm::vec3 planeNormal, planePoint;
    planeNormal = glm::vec3(-1,0,0);
    planePoint = glm::vec3(pool_size,0,0);
    PlanePtr p0 = std::make_shared<Plane>( planeNormal, planePoint);
    system->addPlaneObstacle( p0 );

    planeNormal = glm::vec3(1,0,0);
    planePoint = glm::vec3(-pool_size,0,0);
    PlanePtr p1 = std::make_shared<Plane>( planeNormal, planePoint);
    system->addPlaneObstacle( p1 );

    planeNormal = glm::vec3(0,0,-1);
    planePoint = glm::vec3(0,0,pool_size);
    PlanePtr p2 = std::make_shared<Plane>( planeNormal, planePoint);
    system->addPlaneObstacle( p2 );

    planeNormal = glm::vec3(0,0,1);
    planePoint = glm::vec3(0,0,-pool_size);
    PlanePtr p3 = std::make_shared<Plane>( planeNormal, planePoint);
    system->addPlaneObstacle( p3 );

    planeNormal = glm::vec3(0,1,0);
    planePoint = glm::vec3(0,0,0);
    PlanePtr floor = std::make_shared<Plane>( planeNormal, planePoint);
    system->addPlaneObstacle( floor );

    //Create  plane renderables to display each obstacle
    //Add them to the system renderable
    glm::vec3 x1, x2, x3, x4;
    glm::vec4 color;
    x1 = glm::vec3( pool_size, pool_height,  pool_size );
    x2 = glm::vec3( pool_size, 0,  pool_size );
    x3 = glm::vec3( pool_size, 0, -pool_size );
    x4 = glm::vec3( pool_size, pool_height, -pool_size );
    color = glm::vec4( 0.4, 0.6, 0.8, 1.0);
    QuadMeshRenderablePtr p1Renderable = std::make_shared<QuadMeshRenderable>( flatShader, x1, x2, x3, x4, color);
    HierarchicalRenderable::addChild(systemRenderable, p1Renderable);

    x1 = glm::vec3( -pool_size, pool_height, pool_size );
    x2 = glm::vec3( -pool_size, 0, pool_size );
    x3 = glm::vec3( pool_size, 0, pool_size );
    x4 = glm::vec3( pool_size, pool_height, pool_size );
    color = glm::vec4( 0.2, 0.8, 0.6, 1.0);
    QuadMeshRenderablePtr p2Renderable = std::make_shared<QuadMeshRenderable>( flatShader, x1, x2, x3, x4, color);
    HierarchicalRenderable::addChild(systemRenderable, p2Renderable);

    x1 = glm::vec3( -pool_size, pool_height, -pool_size );
    x2 = glm::vec3( -pool_size, 0, -pool_size );
    x3 = glm::vec3( -pool_size, 0, pool_size );
    x4 = glm::vec3( -pool_size, pool_height, pool_size );
    color = glm::vec4( 0.8, 0.2, 0.4, 1.0 );
    QuadMeshRenderablePtr p3Renderable = std::make_shared<QuadMeshRenderable>( flatShader, x1, x2, x3, x4, color);
    HierarchicalRenderable::addChild(systemRenderable, p3Renderable);

    x1 = glm::vec3( pool_size, pool_height, -pool_size );
    x2 = glm::vec3( pool_size, 0, -pool_size );
    x3 = glm::vec3( -pool_size, 0, -pool_size );
    x4 = glm::vec3( -pool_size, pool_height, -pool_size );
    color = glm::vec4(0.6, 0.8, 0.1, 1.0);
    QuadMeshRenderablePtr p4Renderable = std::make_shared<QuadMeshRenderable>( flatShader, x1, x2, x3, x4, color);
    HierarchicalRenderable::addChild(systemRenderable, p4Renderable);

    x1 = glm::vec3( pool_size, 0, pool_size );
    x2 = glm::vec3( pool_size, 0, -pool_size );
    x3 = glm::vec3( -pool_size, 0, -pool_size );
    x4 = glm::vec3( -pool_size, 0, pool_size );
    color = glm::vec4(0.3, 0.5, 0.9, 1.0);
    QuadMeshRenderablePtr floorRenderable = std::make_shared<QuadMeshRenderable>( flatShader, x1, x2, x3, x4, color);
    HierarchicalRenderable::addChild(systemRenderable, floorRenderable);

    //Initialize a force field that apply only to the mobile particle
    glm::vec3 nullForce(0.0,0.0,0.0);
    std::vector<ParticlePtr> vParticle;
    vParticle.push_back(mobile);
    ConstantForceFieldPtr force = std::make_shared<ConstantForceField>(vParticle, nullForce);
    system->addForceField( force );

    //Initialize a renderable for the force field applied on the mobile particle.
    //This renderable allows to modify the attribute of the force by key/mouse events
    //Add this renderable to the systemRenderable.
    ControlledForceFieldRenderablePtr forceRenderable = std::make_shared<ControlledForceFieldRenderable>( flatShader, force );
    HierarchicalRenderable::addChild(systemRenderable, forceRenderable);

    ConstantForceFieldPtr gravityForceField = std::make_shared<ConstantForceField>(system->getParticles(), DynamicSystem::gravity );
    system->addForceField( gravityForceField );

    //Add a damping force field to the mobile.
    DampingForceFieldPtr dampingForceField = std::make_shared<DampingForceField>(vParticle, 0.9);
    system->addForceField( dampingForceField );

    //Activate collision and set the restitution coefficient to 1.0
    system->setCollisionsDetection(true);
    system->setRestitution(1.0f);
}
