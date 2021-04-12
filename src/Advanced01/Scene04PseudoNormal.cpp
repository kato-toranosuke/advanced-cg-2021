#include "Scene04PseudoNormal.h"
#include <iostream>
#include "PathFinder.h"
#include "Image2OGLTexture.h"

using namespace std;

string Scene04PseudoNormal::s_VertexShaderFilename = "scene04_pseudo_normal.vert";
string Scene04PseudoNormal::s_FragmentShaderFilename = "scene04_pseudo_normal.frag";
GLSLProgramObject* Scene04PseudoNormal::s_pShader = 0;

bool Scene04PseudoNormal::s_RenderWireframe = false;
int Scene04PseudoNormal::s_RenderType = Render_Pseudo_Normal;

TriMesh Scene04PseudoNormal::s_TriMesh;

glm::vec2 Scene04PseudoNormal::s_PrevMouse = glm::vec2(0,0);
ArcballCamera Scene04PseudoNormal::s_Camera(glm::vec3(2.f), glm::vec3(0.f), glm::vec3(0, 1, 0));

GLuint Scene04PseudoNormal::s_VAO = 0;

void Scene04PseudoNormal::Init()
{
	PathFinder finder;
	finder.addSearchPath("Resources");
	finder.addSearchPath("../Resources");
	finder.addSearchPath("../../Resources");

	s_TriMesh.loadObj(finder.find("duck.obj").c_str());
	s_TriMesh.loadTexture(finder.find("duckCM.jpg").c_str());

	ReloadShaders();
}

void Scene04PseudoNormal::ReloadShaders()
{
	if (s_pShader) delete s_pShader;
	s_pShader = new GLSLProgramObject();

	PathFinder finder;
	finder.addSearchPath("GLSL");
	finder.addSearchPath("../GLSL");
	finder.addSearchPath("../../GLSL");

	s_pShader->attachShaderSourceFile(finder.find(s_VertexShaderFilename).c_str(), GL_VERTEX_SHADER);
	s_pShader->attachShaderSourceFile(finder.find(s_FragmentShaderFilename).c_str(), GL_FRAGMENT_SHADER);

	s_pShader->link();

	if (!s_pShader->linkSucceeded())
	{
		cerr << __FUNCTION__ << ": shader link failed" << endl;
		s_pShader->printProgramLog();
		return;
	}

	if (!s_TriMesh.getVertexVBO() || !s_TriMesh.getVertexNormalVBO())
	{
		cerr << __FUNCTION__ << ": mesh VBOs not ready" << endl;
		return;
	}

	if (!s_VAO) glGenVertexArrays(1, &s_VAO);
	glBindVertexArray(s_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, s_TriMesh.getVertexVBO());
	glEnableVertexAttribArray(s_pShader->getAttributeLocation("vertexPosition"));
	glVertexAttribPointer(s_pShader->getAttributeLocation("vertexPosition"), 3, GL_FLOAT, GL_FALSE, 0, (const void*)0); // vertices

	glBindBuffer(GL_ARRAY_BUFFER, s_TriMesh.getVertexNormalVBO());
	glEnableVertexAttribArray(s_pShader->getAttributeLocation("vertexNormal"));
	glVertexAttribPointer(s_pShader->getAttributeLocation("vertexNormal"), 3, GL_FLOAT, GL_FALSE, 0, (const void*)0); // vertices

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Scene04PseudoNormal::Draw()
{
	if (!s_pShader || !s_pShader->linkSucceeded())
		return;

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_DEPTH_TEST);

	const float aspect = s_WindowWidth / float(s_WIndowHeight);
	auto projMatrix = glm::perspective(45.f, aspect, 0.01f, 100.f);
	auto modelViewMatrix = s_Camera.transform() * s_TriMesh.getModelMatrix();

	if (s_RenderType == Render_Texture)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(projMatrix));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(modelViewMatrix));

		s_TriMesh.renderTexturedMesh();
	}
	else if (s_RenderType == Render_Pseudo_Normal)
	{
		glm::mat3 modelViewInvTransposed = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));

		s_pShader->use();
		s_pShader->sendUniformMatrix4fv("modelViewMatrix", glm::value_ptr(modelViewMatrix));
		s_pShader->sendUniformMatrix4fv("projMatrix", glm::value_ptr(projMatrix));
		// TODO: uncomment these lines
		//s_pShader->sendUniformMatrix3fv("modelViewInvTransposed", glm::value_ptr(modelViewInvTransposed));

		glBindVertexArray(s_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3 * s_TriMesh.getNumTriangles());
		glBindVertexArray(0);

		s_pShader->disable();
	}

	if (s_RenderWireframe)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(projMatrix));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(modelViewMatrix));

		s_TriMesh.renderWireframeMesh();
	}

	glPopAttrib();
}

void Scene04PseudoNormal::Cursor(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE &&
		glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE &&
		glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		return;
	}

	glm::vec2 currPos(xpos / s_WindowWidth, (s_WIndowHeight - ypos) / s_WIndowHeight);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		s_Camera.rotate(s_PrevMouse, currPos);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
	{
		s_Camera.zoom(currPos.y - s_PrevMouse.y);
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		s_Camera.pan(currPos - s_PrevMouse);
	}

	s_PrevMouse = currPos;
}

void Scene04PseudoNormal::Mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		s_PrevMouse = glm::vec2(xpos / s_WindowWidth, (s_WIndowHeight - ypos) / s_WIndowHeight);
	}
}

void Scene04PseudoNormal::Resize(GLFWwindow* window, int w, int h)
{
	AbstractScene::Resize(window, w, h);
}

void Scene04PseudoNormal::ImGui()
{
	ImGui::Text("Scene04PseudoNormal Menu:");

	ImGui::Checkbox("Wireframe", &s_RenderWireframe);

	const char* renderTypes[] = { "(None)", "Texture", "Pseudo Normal" };
	ImGui::ListBox("Render Type", &s_RenderType, renderTypes, sizeof(renderTypes)/sizeof(const char*));

	if (ImGui::Button("Reload Shaders"))
	{
		ReloadShaders();
	}
}

void Scene04PseudoNormal::Destroy()
{
	if (s_pShader) delete s_pShader;
	if (s_VAO) glDeleteVertexArrays(1, &s_VAO);
}
