#include <Viewer.hpp>
#include <glm/glm.hpp>
# include "./../include/ShaderProgram.hpp"
# include "./../include/FrameRenderable.hpp"
# include "./../include/CubeRenderable.hpp"
#include "./../include/IndexedCubeRenderable.hpp"

#define SCR_WIDTH 1024
#define SCR_HEIGHT 768

int main( int argc, char* argv[] )
{
	// Stage 1: Create the window and its OpenGL context
    glm::vec4 background_color = glm::vec4(0.8,0.8,0.8,1.0);
	Viewer viewer(SCR_WIDTH, SCR_HEIGHT, background_color);

	// Stage 2: Load resources like shaders, meshes... and make them part of the virtual scene
	// Path to the vertex shader glsl code
	std::string vShader = "./../../sfmlGraphicsPipeline/shaders/defaultVertex.glsl";
	// Path to the fragment shader glsl code
	std::string fShader = "./../../sfmlGraphicsPipeline/shaders/defaultFragment.glsl";
	// Compile and link the shaders into a program
	ShaderProgramPtr defaultShader = std::make_shared<ShaderProgram>(vShader, fShader);
	// Add the shader program to the Viewer
	viewer.addShaderProgram(defaultShader);

	// Compile and link the flat shaders into a shader program
	vShader = "./../../sfmlGraphicsPipeline/shaders/flatVertex.glsl";
	fShader = "./../../sfmlGraphicsPipeline/shaders/flatFragment.glsl";
	ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(vShader, fShader);
	// Add the shader to the Viewer
	viewer.addShaderProgram(flatShader);
	
	// Renderable instantiation
	// When instantiating a renderable ,
	// you must specify the shader program used to draw it .
	FrameRenderablePtr frame = std::make_shared<FrameRenderable>(defaultShader);
	viewer.addRenderable(frame);


	// Instantiate a CubeRenderable while specifying its shader program
	CubeRenderablePtr cube = std::make_shared<CubeRenderable>(flatShader);
	// Add the renderable to the Viewer
	viewer.addRenderable ( cube );

	IndexedCubeRenderablePtr indexedCube = std::make_shared<IndexedCubeRenderable>(flatShader);
	viewer.addRenderable( indexedCube );
	
	glm::mat4 translatedCubeModel = glm::translate(glm::mat4(1.0), glm::vec3(1.5, 0.0, 0.0));
    cube->setModelMatrix(translatedCubeModel);

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(45.0f), glm::vec3(0.5, 1.0, 0.5));
    translatedCubeModel = translatedCubeModel * rotation;
    cube->setModelMatrix(translatedCubeModel);

    glm::mat4 scaledIndexedCubeModel = glm::scale(glm::mat4(1.0), glm::vec3(0.5, 2.0, 1.0));
    indexedCube->setModelMatrix(scaledIndexedCubeModel);
	
	// Stage 3: Our program loop
	while( viewer.isRunning() )
	{
	    viewer.handleEvent(); 	// user interactivity (keyboard/mouse)
	    viewer.draw();		// rasterization (write in framebuffer)
	    viewer.display();		// refresh window
	}
	
	return EXIT_SUCCESS;
}