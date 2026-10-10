#include <Viewer.hpp>
#include <ShaderProgram.hpp>

#include <texturing/TexturedPlaneRenderable.hpp>
#include <texturing/TexturedCubeRenderable.hpp>
#include <texturing/TexturedMeshRenderable.hpp>
#include <texturing/MultiTexturedCubeRenderable.hpp>
#include <texturing/MipMapCubeRenderable.hpp>
#include <texturing/BillBoardPlaneRenderable.hpp>
#include <lighting/DirectionalLightRenderable.hpp>
#include <texturing/TexturedTriangleRenderable.hpp>
#include <texturing/CubeMapRenderable.hpp>
#include <FrameRenderable.hpp>

#include <iostream>

void initialize_scene( Viewer& viewer )
{
    //Position the camera
    
    //Default shader
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    viewer.addShaderProgram( flatShader );

    //Add a 3D frame to the viewer
    FrameRenderablePtr frame = std::make_shared<FrameRenderable>(flatShader);
    viewer.addRenderable(frame);

    //Textured shader
    //    ShaderProgramPtr texShader = std::make_shared<ShaderProgram>("../shaders/textureVertex.glsl","../shaders/textureFragment.glsl");
    ShaderProgramPtr texShader = std::make_shared<ShaderProgram>(   "../../sfmlGraphicsPipeline/shaders/simpleTextureVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/simpleTextureFragment.glsl");
    viewer.addShaderProgram( texShader );


    { // Exercice 1 : Textured bunny
        viewer.getCamera().setViewMatrix( glm::lookAt( glm::vec3(0, 0, 2 ), glm::vec3(0, 0, 0), glm::vec3( 0, 1, 0 ) ) );
        std::string bunny_mesh_path = "./../../sfmlGraphicsPipeline/meshes/tortue2v2.obj";
        std::string bunny_texture_path = "./../../sfmlGraphicsPipeline/meshes/tortue2v2.jpg";
        auto bunny = std::make_shared<TexturedMeshRenderable>(texShader, bunny_mesh_path, bunny_texture_path);
        viewer.addRenderable(bunny);
    }

    
    { // Exercice 2 & 3 : wrapping & filtering options
        /*
        viewer.getCamera().setViewMatrix( glm::lookAt( glm::vec3(0, 0, 8 ), glm::vec3(0, 0, 0), glm::vec3( 0, 1, 0 ) ) );
        std::string filename = "./../../sfmlGraphicsPipeline/textures/grass_texture.png";
        TexturedPlaneRenderablePtr texPlane = std::make_shared<TexturedPlaneRenderable>(texShader, filename);
        texPlane->setGlobalTransform(getScaleMatrix(10));
        viewer.addRenderable(texPlane);
        */
    }

    { // Exercice 4 : multi-resolution (mipmapping)
        /*
        viewer.getCamera().setViewMatrix( glm::lookAt( glm::vec3(1, 1, 2 ), glm::vec3(1, 0, 0), glm::vec3( 0, 1, 0 ) ) );
        std::vector<std::string> mipmaps_filenames = {
            "./../../sfmlGraphicsPipeline/textures/mipmap1.png",
            "./../../sfmlGraphicsPipeline/textures/mipmap2.png",
            "./../../sfmlGraphicsPipeline/textures/mipmap3.png",
            "./../../sfmlGraphicsPipeline/textures/mipmap4.png",
            "./../../sfmlGraphicsPipeline/textures/mipmap5.png",
        };
        auto mmcube = std::make_shared<MipMapCubeRenderable>(texShader, mipmaps_filenames);
        viewer.addRenderable(mmcube);
        auto tcube = std::make_shared<TexturedCubeRenderable>(texShader, mipmaps_filenames[0]);
        viewer.addRenderable(tcube);
        mmcube->setGlobalTransform(getTranslationMatrix(2,0,0));
        */
    }
    { // Exercice 5 : multi-texturing
        
        /*
        viewer.getCamera().setViewMatrix( glm::lookAt( glm::vec3(1, 1, 2 ), glm::vec3(1, 0, 0), glm::vec3( 0, 1, 0 ) ) );
        ShaderProgramPtr multiTexShader = std::make_shared<ShaderProgram>(
            "../../sfmlGraphicsPipeline/shaders/multiTextureVertex.glsl",
            "../../sfmlGraphicsPipeline/shaders/multiTextureFragment.glsl");
        ShaderProgramPtr multiTexNormalShader = std::make_shared<ShaderProgram>(
            "../../sfmlGraphicsPipeline/shaders/multiTextureVertex.glsl",
            "../../sfmlGraphicsPipeline/shaders/multiTextureNormalFragment.glsl");
        viewer.addShaderProgram( multiTexShader );
        viewer.addShaderProgram( multiTexNormalShader );
        
        std::string texture1_path = "./../../sfmlGraphicsPipeline/textures/crate.jpg";
        std::string texture2_path = "./../../sfmlGraphicsPipeline/textures/halflife.png";
        auto mtcube = std::make_shared<MultiTexturedCubeRenderable>(multiTexShader, texture1_path, texture2_path);
        viewer.addRenderable(mtcube);

        std::string normal_map_path = "./../../sfmlGraphicsPipeline/textures/crate_normal.jpg";
        auto mtcube2 = std::make_shared<MultiTexturedCubeRenderable>(multiTexNormalShader, texture1_path, normal_map_path);
        viewer.addRenderable(mtcube2);
        mtcube2->setGlobalTransform(getTranslationMatrix(2,0,0));
        */
    }
    { // Exercice 6 : cubemap
        /*
        viewer.getCamera().setViewMatrix( glm::lookAt( glm::vec3(1, 1, 1 ), glm::vec3(0, 0, 0), glm::vec3( 0, 1, 0 ) ) );
        ShaderProgramPtr cubeMapShader = std::make_shared<ShaderProgram>(  "../../sfmlGraphicsPipeline/shaders/cubeMapVertex.glsl",
                                                                    "../../sfmlGraphicsPipeline/shaders/cubeMapFragment.glsl");
        viewer.addShaderProgram(cubeMapShader);
        
        std::string cubemap_dir = "../../sfmlGraphicsPipeline/textures/skybox";
        auto cubemap = std::make_shared<CubeMapRenderable>(cubeMapShader, cubemap_dir);

        viewer.addRenderable(cubemap);
        */
    }
}

int main() 
{
	Viewer viewer(1280,720, glm::vec4(0.8, 0.8, 0.8, 1.0));
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