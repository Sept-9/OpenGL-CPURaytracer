#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "ThreeDModel.h"
#include "Raytracer.h"
#include <array>
using namespace std;
// add this include
#include <array>
RenderParameters renderParameters;
Raytracer *myRaytracer;
GLFWwindow *window;
bool launchRaytracer;
std::byte movementKeys;
int window_width = 500;
int window_height = 250;
bool debug_on = false;
double mouseXpos;
double mouseYpos;
GLuint RaytracerTextureID;
void ThreeDToGL(const ThreeDModel &model,
				vector<array<float, 3>> &out_vertices,
				vector<array<float, 2>> &out_uvs,
				vector<array<float, 3>> &out_normals)
{
	for (unsigned int face = 0; face < model.faceVertices.size(); face++)
	{
		for (unsigned int triangle = 0; triangle < model.faceVertices[face].size() - 2;
			 triangle++)
		{
			for (unsigned int vertex = 0; vertex < 3; vertex++)
			{
				unsigned int faceVertex = 0;
				if (vertex != 0)
					faceVertex = triangle + vertex;
				out_normals.push_back(array<float, 3>{
					model.normals[model.faceNormals[face][faceVertex]].x,
					model.normals[model.faceNormals[face][faceVertex]].y,
					model.normals[model.faceNormals[face][faceVertex]].z});
				out_uvs.push_back(array<float, 2>{
					model.textureCoords[model.faceTexCoords[face][faceVertex]].x,
					model.textureCoords[model.faceTexCoords[face][faceVertex]].y});
				out_vertices.push_back(array<float, 3>{
					model.vertices[model.faceVertices[face][faceVertex]].x,
					model.vertices[model.faceVertices[face][faceVertex]].y,
					model.vertices[model.faceVertices[face][faceVertex]].z});
			} // per vertex
		} // per triangle
	} // per face
}
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	// Raytracing keys
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		launchRaytracer = true;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		renderParameters.interpolationRendering = !renderParameters.interpolationRendering;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		renderParameters.phongEnabled = !renderParameters.phongEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		renderParameters.shadowsEnabled = !renderParameters.shadowsEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		renderParameters.reflectionEnabled = !renderParameters.reflectionEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		renderParameters.orthoProjection = !renderParameters.orthoProjection;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
		renderParameters.refractionEnabled = !renderParameters.refractionEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_7 && action == GLFW_PRESS)
	{
		renderParameters.monteCarloEnabled = !renderParameters.monteCarloEnabled;
		renderParameters.printSettings();
	}
	if (key == GLFW_KEY_8 && action == GLFW_PRESS)
	{
		renderParameters.aaEnabled = !renderParameters.aaEnabled;
		renderParameters.printSettings();
	}
	// Move forward
	if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			movementKeys |= byte{BIT_FW};
		if (action == GLFW_RELEASE)
			movementKeys &= ~byte{BIT_FW};
	}
	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			movementKeys |= byte{BIT_BACK};
		if (action == GLFW_RELEASE)
			movementKeys &= ~byte{BIT_BACK};
	}
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			movementKeys |= byte{BIT_RIGHT};
		if (action == GLFW_RELEASE)
			movementKeys &= ~byte{BIT_RIGHT};
	}
	if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			movementKeys |= byte{BIT_LEFT};
		if (action == GLFW_RELEASE)
			movementKeys &= ~byte{BIT_LEFT};
	}
	if (key == GLFW_KEY_Q)
	{
		if (action == GLFW_PRESS)
			movementKeys |= byte{BIT_UP};
		if (action == GLFW_RELEASE)
			movementKeys &= ~byte{BIT_UP};
	}
	if (key == GLFW_KEY_E)
	{
		if (action == GLFW_PRESS)
			movementKeys |= byte{BIT_DOWN};
		if (action == GLFW_RELEASE)
			movementKeys &= ~byte{BIT_DOWN};
	}
}
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	float scaledX = float(2.0f * mouseXpos - window_width) / float(window_width);
	float scaledY = float(window_height - 2.0f * mouseYpos) / float(window_height);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		renderParameters.CameraArcball.BeginDrag(scaledX, scaledY);
		movementKeys |= byte{BIT_LEFTMOUSE};
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		renderParameters.ModelArcball.BeginDrag(scaledX, scaledY);
		movementKeys |= byte{BIT_RIGHTMOUSE};
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		renderParameters.CameraArcball.EndDrag(scaledX, scaledY);
		movementKeys &= ~byte{BIT_LEFTMOUSE};
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		renderParameters.ModelArcball.EndDrag(scaledX, scaledY);
		movementKeys &= ~byte{BIT_RIGHTMOUSE};
	}
}
void mousePos_callback(GLFWwindow *window, double x, double y)
{
	// Get mouse position
	mouseXpos = x;
	mouseYpos = y;
	float scaledX = float(2.0f * mouseXpos - window_width) / float(window_width);
	float scaledY = float(window_height - 2.0f * mouseYpos) / float(window_height);
	if ((movementKeys & byte{BIT_LEFTMOUSE}) != byte{0})
		renderParameters.CameraArcball.ContinueDrag(scaledX, scaledY);
	if ((movementKeys & byte{BIT_RIGHTMOUSE}) != byte{0})
		renderParameters.ModelArcball.ContinueDrag(scaledX, scaledY);
}
bool readAndCompileShader(const char *shader_path, const GLuint &id)
{
	string shaderCode;
	ifstream shaderStream(shader_path, std::ios::in);
	if (shaderStream.is_open())
	{
		stringstream sstr;
		sstr << shaderStream.rdbuf();
		shaderCode = sstr.str();
		shaderStream.close();
	}
	else
	{
		cout << "Impossible to open " << shader_path << ". Are you in the rightdirectory? " << endl;
		return false;
	}
	cout << "Compiling shader :" << shader_path << endl;
	char const *sourcePointer = shaderCode.c_str();
	glShaderSource(id, 1, &sourcePointer, NULL);
	glCompileShader(id);
	GLint Result = GL_FALSE;
	int InfoLogLength;
	glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0)
	{
		vector<char> shaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(id, InfoLogLength, NULL, &shaderErrorMessage[0]);
		cout << &shaderErrorMessage[0] << endl;
	}
	cout << "Compilation of Shader: " << shader_path << " " << (Result == GL_TRUE ? "Success" : "Failed!") << endl;
	return Result == 1;
}
void LoadShaders(GLuint &program, const char *vertex_file_path, const char *fragment_file_path)
{
	// Create the shaders - tasks 1 and 2
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	bool vok = readAndCompileShader(vertex_file_path, VertexShaderID);
	bool fok = readAndCompileShader(fragment_file_path, FragmentShaderID);
	if (vok && fok)
	{
		GLint Result = GL_FALSE;
		int InfoLogLength;
		cout << "Linking program" << endl;
		program = glCreateProgram();
		glAttachShader(program, VertexShaderID);
		glAttachShader(program, FragmentShaderID);
		glLinkProgram(program);
		glGetProgramiv(program, GL_LINK_STATUS, &Result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			cout << &ProgramErrorMessage[0];
		}
		std::cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl;
	}
	else
	{
		std::cout << "Program will not be linked: one of the shaders has an error" << std::endl;
	}
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
}
void loadModelGL(const std::vector<ThreeDModel> &objects,
				 std::vector<GLuint> &vaoIDS,
				 std::vector<GLuint> &vbIDS,
				 std::vector<GLuint> &nbIDS,
				 std::vector<GLuint> &tbIDS,
				 std::vector<unsigned int> &count)
{
	for (const auto &to : objects)
	{
		GLuint VertexArrayID;
		GLuint vertexbuffer;
		GLuint uvbuffer;
		GLuint normalbuffer;
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);
		std::vector<std::array<float, 3>> n;
		std::vector<std::array<float, 2>> t;
		std::vector<std::array<float, 3>> v;
		ThreeDToGL(to, v, t, n);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(std::array<float, 3>),
					 &v[0], GL_STATIC_DRAW);
		glVertexAttribPointer(
			0,		  // attribute
			3,		  // size (we have x y z)
			GL_FLOAT, // type of each individual element
			GL_FALSE, // normalized
			0,		  // stride
			(void *)0 // array buffer offset
		);
		glEnableVertexAttribArray(1);
		glGenBuffers(1, &uvbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glBufferData(GL_ARRAY_BUFFER, t.size() * sizeof(std::array<float, 2>), &t[0],
					 GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(2);
		glGenBuffers(1, &normalbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glBufferData(GL_ARRAY_BUFFER, n.size() * sizeof(std::array<float, 3>), &n[0],
					 GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		vaoIDS.push_back(VertexArrayID);
		vbIDS.push_back(vertexbuffer);
		tbIDS.push_back(uvbuffer);
		nbIDS.push_back(normalbuffer);
		count.push_back(GLuint(v.size()));
	}
}
void window_resized(GLFWwindow *window, int width, int height)
{
	window_width = width;
	window_height = height;
	if (myRaytracer != NULL)
	{
		myRaytracer->stopRaytracer();
		myRaytracer->resize(int(width / 2.0f), height);
	}
}
bool initializeGL()
{
	// Initialise GLFW
	if (!glfwInit())
	{
		cerr << "Failed to initialize GLFW" << endl;
		return false;
	}
	glfwWindowHint(GLFW_SAMPLES, 1); // no anti-aliasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy;
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(window_width, window_height, "OpenGLRenderer", NULL, NULL);
	if (window == NULL)
	{
		cerr << "Failed to open GLFW window. If you have an Intel GPU, they may not be 4.5 compatible." << endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW" << endl;
		glfwTerminate();
		return false;
	}
	if (!GLEW_ARB_debug_output)
		return false;
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mousePos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, window_resized);
	glfwPollEvents();
	return true;
}

void loadScreenspaceTexture()
{
	glGenTextures(1, &RaytracerTextureID);
	glBindTexture(GL_TEXTURE_2D, RaytracerTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, GLsizei(window_width / 2.0f), window_height, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, myRaytracer->frameBuffer.block);
	glBindTexture(GL_TEXTURE_2D, -1);
}
int main(int argc, char **argv)
{
	// check the args to make sure there's an input file
	if (argc != 3 && argc != 4)
	{ // bad arg count
		// print an error message
		std::cout << "Usage: " << argv[0] << " geometry material" << std::endl;
		// and leave
		return 0;
	} // bad arg coun
	if (argc == 4 && string(argv[3]) == "debug")
	{
		debug_on = true;
		std::cout << std::endl;
		std::cout << "#######################################################" << std::endl;
		std::cout << "#Debugging when you press R. Did you use triangle.obj?#" << std::endl;
		std::cout << "#######################################################" << std::endl;
		std::cout << std::endl;
	}
	if (!initializeGL())
		return -1;

	std::vector<ThreeDModel> objects;
	std::ifstream geometryFile(argv[1]);
	std::ifstream materialFile(argv[2]);
	// try reading it
	if (!(geometryFile.good()) || !(materialFile.good()))
	{
		std::cout << "Read failed for object " << argv[1] << " or material " << argv[2] << std::endl;
		return 0;
	} // object read failed
	std::string s = argv[2];
	// if is actually passing a material. This will trigger the modified obj read code.
	if (s.find(".mtl") != std::string::npos)
	{
		objects = ThreeDModel::ReadObjectStreamMaterial(geometryFile, materialFile);
	}
	if (objects.size() == 0)
	{
		std::cout << "Read failed for object " << argv[1] << " or material " << argv[2] << std::endl;
		return 0;
	} // object read failed
	std::vector<GLuint> vaoIDS;
	std::vector<GLuint> vbIDS;
	std::vector<GLuint> nbIDS;
	std::vector<GLuint> tbIDS;
	std::vector<unsigned int> counts;
	// setting up opengl
	loadModelGL(objects, vaoIDS, vbIDS, nbIDS, tbIDS, counts);

	renderParameters.findLights(objects);
	std::cout << renderParameters.lights.size() << std::endl;
	GLuint programID = glCreateProgram();
	LoadShaders(programID, "Basic.vert", "Phong.frag");

	// raytracing part
	myRaytracer = new Raytracer(&objects, &renderParameters);
	myRaytracer->resize(int(window_width / 2.0f), window_height);

	GLuint RaytracerVAO;
	glGenVertexArrays(1, &RaytracerVAO);
	glBindVertexArray(RaytracerVAO);
	GLuint ssProgramID;
	LoadShaders(ssProgramID, "screenspace.vert", "screenspace.frag");
	loadScreenspaceTexture();

	glClearColor(0, 0, 0, 0.0f);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE); // For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do
	{
		// Measure speed
		double currentTime = glfwGetTime();
		float deltaTime = float(currentTime - lastTime);
		nbFrames++;
		if (deltaTime >= 1.0)
		{ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderParameters.computeMatricesFromInputs(deltaTime, movementKeys);
		Matrix4 ModelMatrix = renderParameters.getModelMatrix();
		Matrix4 ViewMatrix = renderParameters.getViewMatrix();
		Matrix4 ProjectionMatrix =
			renderParameters.getProjectionMatrix(float(window_width) / 2.0f, float(window_height));
		Matrix4 MVP = ProjectionMatrix * (ViewMatrix * ModelMatrix);
		// will add code here

		glUseProgram(programID);
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
		GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
		std::vector<GLuint> PosLightIDs;
		std::vector<GLuint> ColLightIDs;
		// Set the light position and color
		int currentLight = 0;
		for (Light *l : renderParameters.lights)
		{
			PosLightIDs.push_back(glGetUniformLocation(programID,
													   ("lights[" + std::to_string(currentLight) + "].position").c_str()));
			ColLightIDs.push_back(glGetUniformLocation(programID,
													   ("lights[" + std::to_string(currentLight) + "].color").c_str()));
			currentLight++;
		}
		GLuint nLightsID = glGetUniformLocation(programID, "nLights");
		GLuint emissiveID = glGetUniformLocation(programID, "meshMaterial.emissiveColor");
		GLuint diffuseID = glGetUniformLocation(programID, "meshMaterial.diffuseColor");
		GLuint ambientID = glGetUniformLocation(programID, "meshMaterial.ambientColor");
		GLuint specularID = glGetUniformLocation(programID, "meshMaterial.specularColor");
		GLuint shininessID = glGetUniformLocation(programID, "meshMaterial.shininess");
		// First pass: Rasterisation
		glUseProgram(programID);
		// Send our transformation to the currently bound shader,
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP.columnMajor().coordinates);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, ModelMatrix.columnMajor().coordinates);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, ViewMatrix.columnMajor().coordinates);
		// Set the light position and color
		int cl = 0;
		for (Light *l : renderParameters.lights)
		{
			Homogeneous4 lp = l->GetPositionCenter();
			Homogeneous4 c = l->GetColor();
			glUniform3f(PosLightIDs[cl], lp.x, lp.y, lp.z);
			glUniform3f(ColLightIDs[cl], c.x, c.y, c.z);
			cl++;
		}
		glUniform1i(nLightsID, currentLight); // This will draw on the left side;
		glViewport(0, 0, GLsizei(window_width / 2.0f), window_height);
		for (int i = 0; i < vaoIDS.size(); i++)
		{
			// Setting material properties
			Cartesian3 d = objects[i].material->diffuse;
			Cartesian3 a = objects[i].material->ambient;
			Cartesian3 s = objects[i].material->specular;
			Cartesian3 e = objects[i].material->emissive;
			float shin = objects[i].material->shininess;
			glUniform3f(diffuseID, d.x, d.y, d.z);
			glUniform3f(ambientID, a.x, a.y, a.z);
			glUniform3f(specularID, s.x, s.y, s.z);
			glUniform3f(emissiveID, e.x, e.y, e.z);
			glUniform1f(shininessID, shin);
			glBindVertexArray(vaoIDS[i]);
			// Draw the triangles !
			glDrawArrays(GL_TRIANGLES, 0, counts[i]);
		}

		// raytracing part
		if (launchRaytracer)
		{
			if (debug_on)
				myRaytracer->RaytraceDebug();
			myRaytracer->Raytrace();
			launchRaytracer = false;
		}
		glUseProgram(ssProgramID);
		glViewport(GLint(window_width / 2.0f), 0, GLsizei(window_width / 2.0f), window_height);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RaytracerTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, GLsizei(window_width / 2.0), window_height, 0,
					 GL_RGBA, GL_UNSIGNED_BYTE, myRaytracer->frameBuffer.block);
		glBindVertexArray(RaytracerVAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0); // Get a handle for our uniforms
	glDeleteProgram(programID);
	glDeleteProgram(ssProgramID);
	glDeleteTextures(1, &RaytracerTextureID);
	for (auto vaoID : vaoIDS)
		glDeleteVertexArrays(1, &vaoID);
	for (auto vbID : vbIDS)
		glDeleteBuffers(1, &vbID);
	for (auto nbID : nbIDS)
		glDeleteBuffers(1, &nbID);
	for (auto tbID : tbIDS)
		glDeleteBuffers(1, &tbID);
	myRaytracer->stopRaytracer();
	glfwTerminate();
	return 0;
}
