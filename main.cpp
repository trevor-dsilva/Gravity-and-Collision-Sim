#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "CommonValues.h"

#include "GLWindow.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Material.h"

#include "Model.h"

#include "Skybox.h"

#include "PhysicsObject.h"

//#include <Eigen/Dense>

const float toRadians = 3.14159265f / 180.0f;

GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
uniformSpecularIntensity = 0, uniformShininess = 0,
uniformDirectionalLightTransform = 0, uniformOmniLightPos = 0, uniformFarPlane = 0;

GLWindow mainWindow;
std::vector<Mesh*> meshList;
std::vector<PhysicsObject*> physObjList;

std::vector<Eigen::Vector3f> yk;


std::vector<Shader> shaderList;
Shader directionalShadowShader;
Shader omniShadowShader;

Camera camera;

Texture brickTexture;
Texture dirtTexture;
Texture plainTexture;
Texture tileTexture;

Material shinyMaterial;
Material dullMaterial;

Model xwing;
Model blackhawk;
Model crate;


DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

Skybox skybox;

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

float timeStep = 0.0f;

GLfloat blackhawkAngle = 0.0f;


// Vertex Shader
static const char* vShader = "Shaders/shader.vert";

// Fragment Shader
static const char* fShader = "Shaders/shader.frag";

void calcAverageNormals(unsigned int* indices, unsigned int indiceCount, GLfloat* vertices, unsigned int verticeCount,
	unsigned int vLength, unsigned int normalOffset)
{
	for (size_t i = 0; i < indiceCount; i += 3)
	{
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x; vertices[in0 + 1] += normal.y; vertices[in0 + 2] += normal.z;
		vertices[in1] += normal.x; vertices[in1 + 1] += normal.y; vertices[in1 + 2] += normal.z;
		vertices[in2] += normal.x; vertices[in2 + 1] += normal.y; vertices[in2 + 2] += normal.z;
	}

	for (size_t i = 0; i < verticeCount / vLength; i++)
	{
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects()
{
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[] = {
		//	x      y      z			u	  v			nx	  ny    nz
			-1.0f, -1.0f, -0.6f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 1.0f,		0.5f, 0.0f,		0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, -0.6f,		1.0f, 0.0f,		0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,		0.5f, 1.0f,		0.0f, 0.0f, 0.0f
	};

	unsigned int floorIndices[] = {
		0, 2, 1,
		1, 2, 3
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f,	0.0f, 0.0f,		0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,	10.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,	0.0f, 10.0f,	0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,		10.0f, 10.0f,	0.0f, -1.0f, 0.0f
	};

	calcAverageNormals(indices, 12, vertices, 32, 8, 5);


	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);

	/*Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);*/
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);

	directionalShadowShader.CreateFromFiles("Shaders/directional_shadow_map.vert", "Shaders/directional_shadow_map.frag");
	omniShadowShader.CreateFromFiles("Shaders/omni_shadow_map.vert", "Shaders/omni_shadow_map.geom", "Shaders/omni_shadow_map.frag");
}


// ***************************************** NEW STUFF *****************************************



glm::vec3 eigenToGlmVector(Eigen::Vector3f v)
{
	return glm::vec3(v.x(), v.y(), v.z());
}

Eigen::Vector3f glmToEigenVector(glm::vec3 v)
{
	return Eigen::Vector3f(v.x, v.y, v.z);
}

void CreateTowerCube(int dimension)
{
	//glm::mat4 model(1.0f);
	glm::vec3 origin(-3.0f, 0.0f, -3.0f);
	for (unsigned int k = 0; k < 5; k++)
	{
		for (unsigned int i = 0; i < 5; i++)
		{
			for (unsigned int j = 0; j < 5; j++)
			{
				Eigen::Vector3f p = glmToEigenVector(glm::vec3(origin.x + i, origin.y + k, origin.z + j));
				physObjList.push_back(new PhysicsObject(p, Eigen::Vector3f::Zero(), 1.0f));
				/*model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(origin.x + i, origin.y + k, origin.z + j));
				model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
				glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
				shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
				crate.RenderModel();*/
			}
		}
	}

}

void CreateTowerSquare(int dimension)
{
	//glm::mat4 model(1.0f);
	glm::vec3 origin(-3.0f, 8.0f, -3.0f);
	for (unsigned int i = 0; i < 5; i++)
	{
		for (unsigned int j = 0; j < 5; j++)
		{
			Eigen::Vector3f p = glmToEigenVector(glm::vec3(origin.x + i, origin.y, origin.z + j));
			physObjList.push_back(new PhysicsObject(p, Eigen::Vector3f::Zero(), 1.0f));
			/*model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(origin.x + i, origin.y + k, origin.z + j));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
			glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
			shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
			crate.RenderModel();*/
		}
	}

}

void TestTwoCubes()
{
	Eigen::Vector3f c1 = glmToEigenVector(glm::vec3(0.0f, 10.0f, 0.0f));
	physObjList.push_back(new PhysicsObject(c1, Eigen::Vector3f::Zero(), 1.0f));

	Eigen::Vector3f c2 = glmToEigenVector(glm::vec3(0.0f, 12.0f, 1.0f));
	physObjList.push_back(new PhysicsObject(c2, Eigen::Vector3f::Zero(), 1.0f));
}

void SetUpYk()
{
	for (unsigned int i = 0; i < physObjList.size(); i++)
	{
		yk.push_back(physObjList.at(i)->GetPosition());
		yk.push_back(physObjList.at(i)->GetVelocity());
	}
}

Eigen::Vector3f CalcGravity(PhysicsObject* obj, float g)
{
	float m = obj->GetMass();
	return Eigen::Vector3f(0.0f, -m * g, 0.0f);
}

Eigen::Vector3f CalcAcceleration(Eigen::Vector3f forces, PhysicsObject* obj)
{
	float m = obj->GetMass();
	Eigen::Vector3f acc = 1 / m * forces;
	return acc;
}

std::vector<Eigen::Vector3f> CalcVerlet()
{
	std::vector<Eigen::Vector3f> yk_prime;

	for (unsigned int i = 0; i < physObjList.size(); i++)
	{
		int index = i * 2;
		int indexVel = index + 1;
		Eigen::Vector3f newPos = yk[index] + yk[indexVel] + CalcAcceleration(CalcGravity(physObjList[i], 9.80f), physObjList[i]) * (timeStep * timeStep);
		Eigen::Vector3f newVel = newPos - yk[index];

		if (newPos.y() < -1.5f)
		{
			newPos.y() = -1.5f;
			yk_prime.push_back(newPos);
			yk_prime.push_back(newVel);
		}
		else
		{
			yk_prime.push_back(newPos);
			yk_prime.push_back(newVel);
		}
		
	}

	return yk_prime;
}

void UpdateVisuals()
{
	yk = CalcVerlet();
	glm::mat4 model(1.0f);

	for (unsigned int i = 0; i < physObjList.size(); i++)
	{
		int index = i * 2;
		int indexVel = index + 1;
		physObjList[i]->SetPosition(yk[index]);
		physObjList[i]->SetVelocity(yk[indexVel]);
		
		model = glm::mat4(1.0f);
		model = glm::translate(model, eigenToGlmVector(physObjList[i]->GetPosition()));
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		crate.RenderModel();
	}
}

// ***************************************** NEW STUFF END *****************************************


void RenderScene()
{
	// Create floor mesh and render it
	glm::mat4 model(1.0f);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	tileTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();


	UpdateVisuals();
}

void DirectionalShadowMapPass(DirectionalLight* light)
{
	directionalShadowShader.UseShader();

	glViewport(0, 0, light->getShadowMap()->GetShadowWidth(), light->getShadowMap()->GetShadowHeight());

	light->getShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = directionalShadowShader.GetModelLocation();
	auto temp = light->CalculateLightTransform();
	directionalShadowShader.SetDirectionalLightTransform(&temp);

	directionalShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OmniShadowMapPass(PointLight* light)
{
	omniShadowShader.UseShader();

	glViewport(0, 0, light->getShadowMap()->GetShadowWidth(), light->getShadowMap()->GetShadowHeight());

	light->getShadowMap()->Write();
	glClear(GL_DEPTH_BUFFER_BIT);

	uniformModel = omniShadowShader.GetModelLocation();
	uniformOmniLightPos = omniShadowShader.GetOmniLightPosLocation();
	uniformFarPlane = omniShadowShader.GetFarPlaneLocation();

	glUniform3f(uniformOmniLightPos, light->GetPosition().x, light->GetPosition().y, light->GetPosition().z);
	glUniform1f(uniformFarPlane, light->GetFarPlane());
	omniShadowShader.SetLightMatrices(light->CalculateLightTransform());

	omniShadowShader.Validate();

	RenderScene();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPass(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	glViewport(0, 0, 1366, 768);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	skybox.DrawSkybox(viewMatrix, projectionMatrix);

	shaderList[0].UseShader();

	uniformModel = shaderList[0].GetModelLocation();
	uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();
	uniformModel = shaderList[0].GetModelLocation();
	uniformEyePosition = shaderList[0].GetEyePositionLocation();
	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

	shaderList[0].SetDirectionalLight(&mainLight);
	shaderList[0].SetPointLights(pointLights, pointLightCount, 3, 0);
	shaderList[0].SetSpotLights(spotLights, spotLightCount, 3 + pointLightCount, pointLightCount);
	auto temp = mainLight.CalculateLightTransform();
	shaderList[0].SetDirectionalLightTransform(&temp);

	mainLight.getShadowMap()->Read(GL_TEXTURE2);
	shaderList[0].SetTexture(1);
	shaderList[0].SetDirectionalShadowMap(2);

	glm::vec3 lowerLight = camera.getCameraPosition();
	lowerLight.y -= 0.3f;
	spotLights[0].SetFlash(lowerLight, camera.getCameraDirection());

	shaderList[0].Validate();

	RenderScene();
}

int main()
{
	mainWindow = GLWindow(1366, 768); // 1280, 1024 or 1024, 768
	mainWindow.Initialize();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(6.0f, 10.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 120.0f, -45.0f, 5.0f, 0.5f);

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();
	tileTexture = Texture("Textures/tile.jpg");
	tileTexture.LoadTexture();

	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	xwing = Model();
	xwing.LoadModel("Models/x-wing.obj");

	blackhawk = Model();
	blackhawk.LoadModel("Models/uh60.obj");

	crate = Model();
	crate.LoadModel("Models/Crate1.obj");

	mainLight = DirectionalLight(2048, 2048,
		1.0f, 1.0f, 1.0f,
		0.1f, 0.9f,
		-10.0f, -12.0f, 18.5f);

	/*pointLights[0] = PointLight(1024, 1024,
		0.01f, 100.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 2.0f, 0.0f,
		0.3f, 0.2f, 0.1f);
	pointLightCount++;

	spotLights[0] = SpotLight(1024, 1024,
		0.01f, 100.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 2.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLightCount++;
	spotLights[0] = SpotLight(1024, 1024,
		0.01f, 100.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, -1.5f, 0.0f,
		-100.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		20.0f);
	spotLightCount++;
	*/
	std::vector<std::string> skyboxFaces;
	skyboxFaces.push_back("Textures/Skybox2/miramar_rt.tga");
	skyboxFaces.push_back("Textures/Skybox2/miramar_lf.tga");
	skyboxFaces.push_back("Textures/Skybox2/miramar_up.tga");
	skyboxFaces.push_back("Textures/Skybox2/miramar_dn.tga");
	skyboxFaces.push_back("Textures/Skybox2/miramar_bk.tga");
	skyboxFaces.push_back("Textures/Skybox2/miramar_ft.tga");

	skybox = Skybox(skyboxFaces);

	//TestTwoCubes();
	CreateTowerSquare(5);
	SetUpYk();

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformEyePosition = 0,
		uniformSpecularIntensity = 0, uniformShininess = 0;
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (GLfloat)mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime(); // SDL_GetPerformanceCounter();
		deltaTime = now - lastTime; // (now - lastTime)*1000/SDL_GetPerformanceFrequency();
		timeStep = 1.0f / 30.0f * deltaTime;//0.01f * deltaTime;
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		/*if (mainWindow.getsKeys()[GLFW_KEY_L])
		{
			spotLights[0].Toggle();
			mainWindow.getsKeys()[GLFW_KEY_L] = false;
		}*/

		DirectionalShadowMapPass(&mainLight);
		/*for (size_t i = 0; i < pointLightCount; i++)
		{
			OmniShadowMapPass(&pointLights[i]);
		}
		for (size_t i = 0; i < spotLightCount; i++)
		{
			OmniShadowMapPass(&spotLights[i]);
		}*/
		RenderPass(camera.calculateViewMatrix(), projection);

		mainWindow.swapBuffers();
	}

	return 0;
}
/*model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	brickTexture.UseTexture();
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[0]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 4.0f, -2.5f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	dirtTexture.UseTexture();
	dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	meshList[1]->RenderMesh();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 10.0f));
	model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	xwing.RenderModel();

	blackhawkAngle += 0.1f;
	if (blackhawkAngle > 360.0f)
	{
		blackhawkAngle = 0.1f;
	}

	model = glm::mat4(1.0f);
	model = glm::rotate(model, -blackhawkAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(-8.0f, 2.0f, 0.0f));
	model = glm::rotate(model, -20.0f * toRadians, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::rotate(model, -90.0f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	blackhawk.RenderModel();
	*/

	/*model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
	model = glm::rotate(model, -blackhawkAngle * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
	crate.RenderModel();*/