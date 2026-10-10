#include <Viewer.hpp>
#include <ShaderProgram.hpp>
#include <FrameRenderable.hpp>
#include <HierarchicalRenderable.hpp>
#include <CylinderMeshRenderable.hpp>
#include <MeshRenderable.hpp>
#include <SphereMeshRenderable.hpp>

void initialize_scene( Viewer& viewer )
{
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");

    viewer.addShaderProgram( flatShader );

    //Frame
    FrameRenderablePtr frame = std::make_shared<FrameRenderable>(flatShader);
    viewer.addRenderable(frame);


	std::string vShader = "./../../sfmlGraphicsPipeline/shaders/flatVertex.glsl";
	std::string fShader = "./../../sfmlGraphicsPipeline/shaders/flatFragment.glsl";
	ShaderProgramPtr prog = std::make_shared<ShaderProgram>(vShader,fShader);
	viewer.addShaderProgram(prog);

	std::shared_ptr<CylinderMeshRenderable> trunk = std::make_shared<CylinderMeshRenderable>(prog,true);
	glm::mat4 trunkGlobalTransform;
	trunk -> setGlobalTransform(trunkGlobalTransform* getScaleMatrix(0.2f, 2.0f, 0.2f)*getRotationMatrix(glm::half_pi<float>(), glm::vec3(1, 0, 0)));

	std :: shared_ptr < SphereMeshRenderable > joint = std::make_shared<SphereMeshRenderable>(prog, true);
    joint->setLocalTransform(getTranslationMatrix(0.0f, 1.0f, 0.0f) * getScaleMatrix(0.3f));

    std::shared_ptr<CylinderMeshRenderable> branchLeft = std::make_shared<CylinderMeshRenderable>(prog, true);
    branchLeft->setLocalTransform(
        getTranslationMatrix(0.0f, 1.0f, 0.0f) *
        getRotationMatrix(glm::radians(45.0f), glm::vec3(0, 0, 1))*getScaleMatrix(0.1f, 1.0f, 0.1f)
    );

    std::shared_ptr<CylinderMeshRenderable> branchRight = std::make_shared<CylinderMeshRenderable>(prog, true);
    branchRight->setLocalTransform(
        getTranslationMatrix(0.0f, 1.0f, 0.0f) *
        getRotationMatrix(glm::radians(-45.0f), glm::vec3(0, 0, 1))*getScaleMatrix(0.1f, 1.0f, 0.1f)
    );

    std :: shared_ptr < SphereMeshRenderable > jointLeft = std::make_shared<SphereMeshRenderable>(prog, true);
    jointLeft->setLocalTransform(getTranslationMatrix(0.0f, 1.0f, 0.0f) * getScaleMatrix(0.3f));

    std :: shared_ptr < SphereMeshRenderable > jointRight = std::make_shared<SphereMeshRenderable>(prog, true);
    jointRight->setLocalTransform(getTranslationMatrix(0.0f, 1.0f, 0.0f) * getScaleMatrix(0.3f));

	HierarchicalRenderable::addChild(trunk, joint);
	HierarchicalRenderable::addChild(trunk, branchLeft);
    HierarchicalRenderable::addChild(trunk, branchRight);
    HierarchicalRenderable::addChild(branchLeft, jointLeft);
    HierarchicalRenderable::addChild(branchRight, jointRight);

	viewer.addRenderable(trunk);
}

int main() 
{
    Viewer viewer(1280,720);
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
