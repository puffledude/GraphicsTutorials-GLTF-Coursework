#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Extra/GLTFLoader.h"

/*
* Finish deferred lighting. Move sun using arrow keys.

Objectives
Leaves falling off of trees
Trees..
Sea water and pond water.
Maybe fish in pond?

Then for winter.
Snow particles.
Snow on ground and extra specular on snow.
Frozen pond.

Plus need to set up some form of camera trail.*/

Renderer::Renderer(Window& parent) : OGLRenderer(parent)	{
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);
	camera = new Camera(0.0f, 180.0f, Vector3(50, 40, 30.0f));

	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	if (!sphere) {
		return;
	}
	quad = Mesh::GenerateQuad();
	cone = Mesh::LoadFromMeshFile("Cone.msh");
	this->SetupDeferred();
	this->SetupShadow();
	this->LoadEnvironment();
	this->LoadSkyBox();
	this->LoadWater();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	init = true;
}

void Renderer::SetupDeferred() {
	environmentShader = new Shader("bumpVertex.glsl", "bufferFragment.glsl");
	pointLightShader = new Shader("pointLightVertex.glsl", "pointLightFragment.glsl");
	combineShader = new Shader("combineVertex.glsl", "combineFragment.glsl");
	if (!environmentShader->LoadSuccess() ||
		!pointLightShader->LoadSuccess() ||
		!combineShader->LoadSuccess()){
		return;
	}

	glGenFramebuffers(1, &gBufferFBO);
	glGenFramebuffers(1, &pointLightFBO);

	GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);

	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, bufferNormalTex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);

	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupShadow() {
	shadowShader = new Shader("shadowVertex.glsl", "shadowFragment.glsl");
	if (!shadowShader->LoadSuccess()) {
		return;
	}

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::LoadEnvironment() {
	GLTFLoader::Load("../GLTF/Environment/CourseWorkProject.gltf", Environment);
	if (Environment.meshes.size() == 0) {
		return;
	}
	GLTFLoader::Load("../GLTF/Tree/Tree.gltf", Tree);
	if (Environment.meshes.size() == 0) {
		return;
	}
	this->root = SceneNode();
	SceneNode* ground = new SceneNode(&Environment, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for environment
	ground->SetModelScale(Vector3(75.0f, 75.0f, 75.0f));
	root.AddChild(ground);
	Vector3 treePositions[] = {
		Vector3(44.5433, 36.0f, 26.3742),
		Vector3(23.1988,36.5f,32.4063),
		Vector3(50.0999,37.3,33.0351),
		Vector3(45.7918,37.0f,40.43),
		Vector3(37.3799,37.4,51.1133),
		Vector3(33.1454,36.44,43.647),
		Vector3(44.4436,36.4546,48.1839),
		Vector3(39.4295,37.386,41.8542),
		Vector3(22.9848,36.4,42.3077),
		Vector3(27.4219,36.6f,20.9763),
		Vector3(40.1285,36.8668,16.355)
	};
	for (Vector3 pos : treePositions) {
		SceneNode* treeNode = new SceneNode(&Tree, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for trees
		treeNode->SetTransform(Matrix4::Translation(pos));
		treeNode->SetModelScale(Vector3(0.1, 0.1, 0.1));
		ground->AddChild(treeNode);
	}
	//SceneNode* treeNode = new SceneNode(&Tree, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for trees
	//treeNode->SetTransform(Matrix4::Translation(Vector3(44.5433, 36.0f, 26.3742)));
	//treeNode->SetModelScale(Vector3(0.1, 0.1, 0.1));
	//ground->AddChild(treeNode);
	sun = new Light(Vector3(23.6744, 58.4126, 3.97436), Vector4(1, 1, 1, 1), 100.0f);
	Light* pointLight1 = new Light(Vector3(30.0f, 40.0f, 30.0f), Vector4(0.5, 0.5, 0, 1), 15.0f);

	pointLights.push_back(sun);
	pointLights.push_back(pointLight1);
}
void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	gameFrameTime = dt;
	root.Update(dt);
}

void Renderer::LoadSkyBox() { //Actual load order is right, left, up, down , front, back. Need to either rename or just keep it.
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	if (!skyboxShader->LoadSuccess()) {
		return;
	}
	cubeMap = OGLTexture::LoadCubemap(
		TEXTUREDIR"CubeMapLeft.jpg",

		TEXTUREDIR"CubeMapRight.jpg",
		TEXTUREDIR"CubeMapUp.jpg",

		TEXTUREDIR"CubeMapDown.jpg",
		TEXTUREDIR"CubeMapBack.jpg",
		TEXTUREDIR"CubeMapFront.jpg"
		);
	if (!this->cubeMap) {
		return;
	}
}

void Renderer::LoadWater() {
	waterShader = new Shader("waterVertex.glsl", "waterFragment.glsl");
	if (!waterShader->LoadSuccess()) {
		return;
	}
	waterTex = OGLTexture::TextureFromFile(
		TEXTUREDIR"waterTex.png");
	if (!waterTex) {
		return;
	}
	/*Matrix4 seaModelMatrix = Matrix4::Translation(Vector3(0, 32, 0)) *
		Matrix4::Scale(Vector3(75.0f, 1.0f, 75.0f))*
		Matrix4::Rotation(-90, Vector3(1, 0, 0));
	waterData.emplace_back(seaModelMatrix, waterShader);*/

}


Renderer::~Renderer(void)	{
	delete environmentShader;
	delete camera;

	delete pointLightShader;
	delete combineShader;
	delete shadowShader;
	delete skyboxShader;
	delete waterShader;
	delete sphere;
	delete quad;
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteFramebuffers(1, &gBufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
}



/// <summary>
/// Renders Scene. Skybox -> Shadow Scene -> Environment fill -> Lighting calc -> Combine Buffers.
/// </summary>
void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();  
	DrawShadowScene();

	DrawEnvironment();

	DrawLights();

	CombineBuffers();

	std::cout << "Camera location is : " << camera->GetPosition()<< std::endl;
	//DrawPostProcessing();
}

void Renderer::DrawEnvironment(bool shadow) {
	//BindShader(environmentShader);
	/*modelMatrix = root.GetWorldTransform() * Matrix4::Scale(root.GetModelScale());
	root.Draw(*this);*/
	if (!shadow) {
		glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		
	}

	// Removed skybox draw from here so it won't be rendered into the g-buffer.
	// Skybox will be drawn after combine with the scene depth copied into the default framebuffer.

	DrawNode(&root, shadow);
	DrawWater(shadow);
	/*for (Light* l : pointLights) {

		sphere->Draw();
	}*/
	if (!shadow){ 
		
	glBindFramebuffer(GL_FRAMEBUFFER, 0); }
}
void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(sun->GetPosition(),
		Vector3(0, 0, 0));
	projMatrix = Matrix4::Perspective(1.0f, 1000, 1.0f, 45.0f);
	shadowMatrix = projMatrix * viewMatrix;

	DrawEnvironment(true);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::DrawLights() {
	glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBO);
	// Use the point light shader before setting uniforms
	BindShader(pointLightShader);



	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glBlendFunc(GL_ONE, GL_ONE);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);


	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(),
		"depthTex"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(),
		"normalTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(),
		"shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	const Vector3 camPos = camera->GetPosition();
	glUniform3fv(glGetUniformLocation(pointLightShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

	glUniform2f(glad_glGetUniformLocation(pointLightShader->GetProgram(),
		"pixelSize"), 1.0f / width, 1.0f / (float)height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(pointLightShader->GetProgram(),
		"invProjViewMatrix"), 1, false, invViewProj.values);

	UpdateShaderMatrices();
	for(Light* l : pointLights) {
		SetShaderLight(*l);
		sphere->Draw();
	}


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawSkybox(bool shadow) {
	// Draw skybox using LEQUAL so it will render correctly when depth already exists
	GLint prevDepthFunc = 0;
	glGetIntegerv(GL_DEPTH_FUNC, &prevDepthFunc);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);

	if (!shadow) {
		BindShader(skyboxShader);
		glUniform1i(glGetUniformLocation(skyboxShader->GetProgram(),
			"cubeTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->GetObjectID());

		// Use a view matrix without translation so the skybox appears infinitely far away
		Matrix4 camView = camera->BuildViewMatrix();
		// zero out translation components (values 12,13,14 hold translation)
		camView.values[12] = 0.0f;
		camView.values[13] = 0.0f;
		camView.values[14] = 0.0f;
		viewMatrix = camView;
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
			(float)width / (float)height, 45.0f);
	}
	UpdateShaderMatrices();
	quad->Draw();
	glDepthMask(GL_TRUE);

	// restore previous depth func
	glDepthFunc(prevDepthFunc);
}


void Renderer::CombineBuffers() {
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(),
		"diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(),
		"diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(),
		"specularLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(),
		"depthTex"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);

	quad->Draw();
}



void Renderer::DrawNode(SceneNode* n, bool shadow) {
	if (n->GetMesh()) {
		Shader* nodeShader = n->GetShader();
		// Bind the node's shader only for the normal (non-shadow) pass.
		if (!shadow && nodeShader) {
			BindShader(nodeShader);
		}

		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();

		// Set per-node uniforms only when the node's shader is bound (non-shadow).
		if (!shadow && nodeShader) {
			Vector4 nodeCol = n->GetColour();
			glUniform4fv(glGetUniformLocation(nodeShader->GetProgram(), "nodeColour"), 1, (float*)&nodeCol);
			glUniform1i(glGetUniformLocation(nodeShader->GetProgram(), "useTexture"), 0);
		}

		n->Draw(*this, shadow);
	}
	else if (n->GetGLTFScene()) {
		Shader* nodeShader = n->GetShader();
		// Bind the node's shader for the normal pass only.
		if (!shadow && nodeShader) {
			BindShader(nodeShader);
		}

		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());

		// IMPORTANT: don't overwrite the current view/proj when rendering the shadow map.
		// The shadow pass sets viewMatrix/projMatrix to the light's matrices before calling DrawNode(..., true).
		if (!shadow) {
			viewMatrix = camera->BuildViewMatrix();
			projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
				(float)width / (float)height, 45.0f);
		}

		UpdateShaderMatrices();
		n->Draw(*this, shadow);
	}

	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i) {
		DrawNode(*i, shadow);
	}
}

void Renderer::DrawWater(bool shadow) {
	if (!shadow) {
	BindShader(waterShader);
	const Vector3 camPos = camera->GetPosition();
	glUniform3fv(glGetUniformLocation(waterShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex->GetObjectID());

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->GetObjectID());
	}
	

	//Want to cover entrie size of the scene at height 32. Go from x,z(0,0) to x,z (75,75)

	modelMatrix = Matrix4::Translation(Vector3(0,32,0)) *
		Matrix4::Scale(Vector3(75.0f, 1.0f, 75.0f))*
		Matrix4::Rotation(-90, Vector3(1, 0, 0));
	UpdateShaderMatrices();
	quad->Draw();

	modelMatrix = Matrix4::Translation(Vector3(37.5f, 34.5, 29.0)) *
		Matrix4::Scale(Vector3(5.0f, 3.0f, 5.0f))*
		Matrix4::Rotation(90, Vector3(1, 0, 0));
	UpdateShaderMatrices();
	cone->Draw();
}


void Renderer::GenerateScreenTexture(GLuint& into, bool depth) {
	glGenTextures(1, &into);
	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;
	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
		type, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

}
