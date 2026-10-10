#include <Viewer.hpp>
#include <ShaderProgram.hpp>

#include <lighting/LightedMeshRenderable.hpp>
#include <lighting/LightedCubeRenderable.hpp>
#include <lighting/LightedCylinderRenderable.hpp>
#include <lighting/DirectionalLightRenderable.hpp>
#include <lighting/PointLightRenderable.hpp>
#include <lighting/SpotLightRenderable.hpp>
#include <FrameRenderable.hpp>

#include <iostream>

void initialize_scene( Viewer& viewer )
{
    //Default shader
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    viewer.addShaderProgram( flatShader );

    //Define a shader that encode an illumination model
    ShaderProgramPtr phongShader = std::make_shared<ShaderProgram>( "../../sfmlGraphicsPipeline/shaders/phongVertex.glsl", 
                                                                    "../../sfmlGraphicsPipeline/shaders/phongFragment.glsl");
    viewer.addShaderProgram( phongShader );

    //Add a 3D frame to the viewer
    FrameRenderablePtr frame = std::make_shared<FrameRenderable>(flatShader);
    viewer.addRenderable(frame);

    //Define a transformation
    glm::mat4 globalTransformation, localTransformation;

    //Define a directional light for the whole scene
    glm::vec3 d_direction = glm::normalize(glm::vec3(0.0,-1.0,-1.0));
    glm::vec3 d_ambient(0.0,0.0,0.0), d_diffuse(0.3,0.3,0.1), d_specular(0.3,0.3,0.1);
    //glm::vec3 d_ambient(0.0,0.0,0.0), d_diffuse(0.0,0.0,0.0), d_specular(0.0,0.0,0.0);
    glm::vec3 lightPosition(0.0,5.0,8.0);
    DirectionalLightPtr directionalLight = std::make_shared<DirectionalLight>(d_direction, d_ambient, d_diffuse, d_specular);
    directionalLight->setGlobalTransform(getTranslationMatrix(lightPosition) * directionalLight->getGlobalTransform());
    
    //Add a renderable to display the light and control it via mouse/key event
    viewer.addDirectionalLight(directionalLight);
    directionalLight->addGlobalTransformKeyframe(getRotationMatrix( 0 * 2 * M_PI, 1, 1, 1)    , 0);
    directionalLight->addGlobalTransformKeyframe(getRotationMatrix( 0.25 * 2 * M_PI, 1, 1, 1) , 1);
    directionalLight->addGlobalTransformKeyframe(getRotationMatrix( 0.5 * 2 * M_PI, 1, 1, 1)  , 2);
    directionalLight->addGlobalTransformKeyframe(getRotationMatrix( 0.75 * 2 * M_PI, 1, 1, 1) , 3);
    directionalLight->addGlobalTransformKeyframe(getRotationMatrix( 1 * 2 * M_PI, 1, 1, 1)    , 4);

    auto directionalLightRenderable = std::make_shared<DirectionalLightRenderable>(flatShader, directionalLight);
    localTransformation = getScaleMatrix(0.5);
    directionalLightRenderable->setLocalTransform(localTransformation);
    directionalLight->setGlobalTransform(getTranslationMatrix(lightPosition) * directionalLight->getGlobalTransform());
    viewer.addRenderable(directionalLightRenderable);

    //Define a point light
    glm::vec3 p_position(0.0,0.0,0.0), p_ambient(0.0,0.0,0.0), p_diffuse(0.0,0.0,0.0), p_specular(0.0,0.0,0.0);
    float p_constant=0.0, p_linear=0.0, p_quadratic=0.0;

    p_position = glm::vec3(-8, 5.0, 5.0);
    p_ambient = glm::vec3(0.0,0.0,0.0);
    p_diffuse = glm::vec3(1.0,0.0,0.0);
    p_specular = glm::vec3(1.0,0.0,0.0);
    p_constant=1.0;
    p_linear=5e-1;
    p_quadratic=0;
    PointLightPtr pointLight1 = std::make_shared<PointLight>(p_position, p_ambient, p_diffuse, p_specular, p_constant, p_linear, p_quadratic);
    PointLightRenderablePtr pointLightRenderable1 = std::make_shared<PointLightRenderable>(flatShader, pointLight1);
    localTransformation = glm::scale(glm::mat4(1.0), glm::vec3(0.5,0.5,0.5));
    pointLightRenderable1->setLocalTransform(localTransformation);
    viewer.addPointLight(pointLight1);
    viewer.addRenderable(pointLightRenderable1);

    p_position = glm::vec3(8, 5.0, 5.0);
    p_ambient = glm::vec3(0.0,0.0,0.0);
    p_diffuse = glm::vec3(0.0,0.0,1.0);
    p_specular = glm::vec3(0.0,0.0,1.0);
    p_constant=1.0;
    p_linear=5e-1;
    p_quadratic=0;
    PointLightPtr pointLight2 = std::make_shared<PointLight>(p_position, p_ambient, p_diffuse, p_specular, p_constant, p_linear, p_quadratic);
    PointLightRenderablePtr pointLightRenderable2 = std::make_shared<PointLightRenderable>(flatShader, pointLight2);
    localTransformation = glm::scale(glm::mat4(1.0), glm::vec3(0.5,0.5,0.5));
    pointLightRenderable2->setLocalTransform(localTransformation);
    viewer.addPointLight(pointLight2);
    viewer.addRenderable(pointLightRenderable2);

    //Define a spot light
    glm::vec3 s_position(0.0,5.0,-8.0), s_spotDirection = glm::normalize(glm::vec3(0.0,-1.0,1.0));
    //glm::vec3 s_ambient(0.0,0.0,0.0), s_diffuse(0.0,0.0,0.0), s_specular(0.0,0.0,0.0);
    glm::vec3 s_ambient(0.0,0.0,0.0), s_diffuse(0.5,0.5,0.5), s_specular(0.5,0.5,0.5);
    float s_constant=1.0, s_linear=0.0, s_quadratic=0.0;
    float s_innerCutOff=std::cos(glm::radians(20.0f)), s_outerCutOff=std::cos(glm::radians(40.0f));
    SpotLightPtr spotLight = std::make_shared<SpotLight>(s_position, s_spotDirection,
                                                         s_ambient, s_diffuse, s_specular,
                                                         s_constant, s_linear, s_quadratic,
                                                         s_innerCutOff, s_outerCutOff);
    viewer.addSpotLight(spotLight);

    spotLight->addGlobalTransformKeyframe(lookAtModel(glm::vec3(3,5,5), glm::vec3(0), Light::base_forward), 0);
    spotLight->addGlobalTransformKeyframe(lookAtModel(glm::vec3(-3,5,3), glm::vec3(0), Light::base_forward), 3);
    spotLight->addGlobalTransformKeyframe(lookAtModel(glm::vec3(-3,5,-3), glm::vec3(0), Light::base_forward), 6);
    spotLight->addGlobalTransformKeyframe(lookAtModel(glm::vec3(3,5,-3), glm::vec3(0), Light::base_forward), 9);
    spotLight->addGlobalTransformKeyframe(lookAtModel(glm::vec3(3,5,3), glm::vec3(0), Light::base_forward), 12);

    auto spotLightRenderable = std::make_shared<SpotLightRenderable>(flatShader, spotLight);
    viewer.addRenderable(spotLightRenderable);

    //Define materials
    glm::vec3 mAmbient(0.0), mDiffuse(0.0), mSpecular(0.0);
    float mShininess=0.0;
    MaterialPtr myMaterial = std::make_shared<Material>(mAmbient, mDiffuse, mSpecular, mShininess);

    //Lighted Cube
    MaterialPtr pearl = Material::Pearl();
    LightedCubeRenderablePtr ground = std::make_shared<LightedCubeRenderable>(phongShader, false, pearl);
    globalTransformation = glm::translate( glm::mat4(1.0), glm::vec3(0.0,-1.2,0.0) );
    localTransformation = glm::scale(glm::mat4(1.0), glm::vec3(20.0,0.2,20.0));
    ground->setGlobalTransform(globalTransformation);
    ground->setLocalTransform(localTransformation);
    viewer.addRenderable( ground );
    
    //Lighted Mesh 1
    std::string suzanne_path = "./../../sfmlGraphicsPipeline/meshes/suzanne.obj";
    MaterialPtr bronze = Material::Bronze();
    LightedMeshRenderablePtr suzanne1 = std::make_shared<LightedMeshRenderable>(phongShader, suzanne_path, bronze);
    globalTransformation = glm::translate( glm::mat4(1.0), glm::vec3(-2.0,0.0,0.0) );
    localTransformation = glm::mat4(1.0);
    suzanne1->setGlobalTransform(globalTransformation);
    suzanne1->setLocalTransform(localTransformation);
    viewer.addRenderable( suzanne1 );

    //Lighted Mesh 2
    MaterialPtr emerald = Material::Emerald();
    LightedMeshRenderablePtr suzanne2 = std::make_shared<LightedMeshRenderable>(phongShader, suzanne_path, emerald);
    globalTransformation = glm::translate( glm::mat4(1.0), glm::vec3(2.0,0.0,0.0) );
    localTransformation = glm::mat4(1.0);
    suzanne2->setGlobalTransform(globalTransformation);
    suzanne2->setLocalTransform(localTransformation);

    viewer.addRenderable( suzanne2 );

    viewer.startAnimation();
    viewer.setAnimationLoop(true, 4.0);
}

int main() 
{
	Viewer viewer(1280,720, glm::vec4(0.8,0.8,0.8,1));
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
