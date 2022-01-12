//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;


float birdsZ = 0.1f;
float birdsAngle = 0.1f;
float birdsAngleSpeed = 0.1f;

float birdsAngle2 = 0.0f;
float birdsAngleSpeed2 = -0.1f;
bool cameraPreview = false;

gps::Model3D birds1;
gps::Model3D birds2;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

GLfloat near_plane = 0.03f;
GLfloat far_plane = 88.4f;
GLfloat left = -33.0f;
GLfloat right = 31.0f;
GLfloat front = -35.0f;
GLfloat back = 33.0f;
GLfloat coeff = 18.0f;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.04f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

GLuint textureID;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D scene;

double cameraXpos = 0;
double cameraYpos = 0;

gps::Shader myCustomShader;
gps::Shader depthMapShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	glfwGetFramebufferSize(window, &glWindowWidth, &glWindowHeight);

	myCustomShader.useShaderProgram();

	// set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)glWindowWidth / (float)glWindowHeight, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//lightShader.useShaderProgram();

	//glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// set Viewport transform
	glViewport(0, 0, glWindowHeight, glWindowHeight);

}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	float cameraXspeed = xpos > cameraXpos ? cameraSpeed : (xpos < cameraXpos ? -cameraSpeed : 0);
	float cameraYspeed = ypos > cameraYpos ? cameraSpeed : (ypos < cameraYpos ? -cameraSpeed : 0);

	myCamera.rotate(-cameraYspeed, -cameraXspeed);
	view = myCamera.getViewMatrix();
	myCustomShader.useShaderProgram();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

	cameraXpos = xpos;
	cameraYpos = ypos;
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (glfwGetMouseButton(glWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	if (glfwGetMouseButton(glWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (pressedKeys[GLFW_KEY_P]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_I]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_O]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		printf("x = %f   y = %f   z = %f \n", myCamera.getCameraPos().x, myCamera.getCameraPos().y, myCamera.getCameraPos().z);
		printf("x = %f   y = %f   z = %f \n", myCamera.getCameraTarget().x, myCamera.getCameraTarget().y, myCamera.getCameraTarget().z);
		printf("x = %f   y = %f   z = %f \n", myCamera.getCameraUpDirection().x, myCamera.getCameraUpDirection().y, myCamera.getCameraUpDirection().z);
	}

	if (pressedKeys[GLFW_KEY_V]) {
		glfwSetTime(0.0f);
		myCamera.setCameraPos(glm::vec3(0.338150f, 1.382484f, 4.325485f));
		myCamera.setCameraTarget(glm::vec3(1.574606f, 1.307926f, 2.609866f));
		myCamera.setCameraUpDirection(glm::vec3(0.0f, 1.0f, 0.0f));
		myCamera.computeCameraUpDirection();
		myCamera.computeCameraRightDirection();
		cameraPreview = true;
	}

	if (pressedKeys[GLFW_KEY_B]) {
		cameraPreview = false;
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {

	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	scene.LoadModel("objects/scene/gameScene.obj");
	birds1.LoadModel("objects/birds/birds.obj");
	birds2.LoadModel("objects/birds/birds.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
}

void initSkybox() {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	std::vector<const GLchar*> faces;
	faces.push_back("textures/skybox/Left+X.png");
	faces.push_back("textures/skybox/Right-X.png");
	faces.push_back("textures/skybox/Up+Y.png");
	faces.push_back("textures/skybox/Down-Y.png");
	faces.push_back("textures/skybox/Front+Z.png");
	faces.push_back("textures/skybox/Back-Z.png");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir * coeff, glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//const GLfloat near_plane = 1.0f, far_plane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(left, right, front, back, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void animateBirds(gps::Shader shader)
{
	model = glm::mat4(1.0f);
	if (birdsZ < -15)
		birdsZ = 0;
	model = glm::translate(model, glm::vec3(0, 5, birdsZ)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)) * glm::rotate(glm::mat4(1.0f), glm::radians(birdsAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	birdsZ -= 0.007;
	if (birdsAngle > 8)
		birdsAngleSpeed = -0.1;
	if (birdsAngle < -8)
		birdsAngleSpeed = 0.1;
	birdsAngle += birdsAngleSpeed;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	shader.useShaderProgram();
	//draw birds
	birds1.Draw(shader);
	model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(0, 5, birdsZ - 0.5)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)) * glm::rotate(glm::mat4(1.0f), glm::radians(birdsAngle2), glm::vec3(0.0f, 0.0f, 1.0f));
	if (birdsAngle2 > 8)
		birdsAngleSpeed2 = -0.1;
	if (birdsAngle2 < -8)
		birdsAngleSpeed2 = 0.1;
	birdsAngle2 += birdsAngleSpeed2;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	shader.useShaderProgram();
	//draw birds
	birds2.Draw(shader);
}

bool between(float x, float t1, float t2)
{
	return x > t1 && x < t2;
}

void animateCamera() {

	if (between(glfwGetTime(), 0 , 10)) {

		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * 0.25);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (between(glfwGetTime(), 10, 10.7)) {

		myCamera.rotate(0.01f, 0.0f);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (between(glfwGetTime(), 10.7, 16)) {

		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * 0.25);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (between(glfwGetTime(), 16, 16.7)) {

		myCamera.rotate(-0.01f, 0.0f);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (between(glfwGetTime(), 17, 19.36)) {

		myCamera.rotate(0.0f, 0.01f);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (between(glfwGetTime(), 19.36, 26)) {

		myCamera.move(gps::MOVE_FORWARD, cameraSpeed * 0.25);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}

	if (between(glfwGetTime(), 26, 26.4)) {

		myCamera.rotate(-0.01f, 0.0f);

		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	}
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scene.Draw(shader);
	animateBirds(shader);
	//render skybox
	mySkyBox.Draw(skyboxShader, view, projection);

}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//lightCube.Draw(lightShader);
	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		if (cameraPreview)
			animateCamera();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
