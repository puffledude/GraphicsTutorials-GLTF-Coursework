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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);
	camera = new Camera(0.0f, 180.0f, Vector3(50, 40, 30.0f));
	sun = new Light(Vector3(100.0f, 100.0f, 100.0f), Vector4(1, 1, 1, 1), 200.0f);
	pointLights.push_back(sun);
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	quad = Mesh::GenerateQuad();


	GLTFLoader::Load("../GLTF/Environment/CourseWorkProject.gltf", Environment);
	if (Environment.meshes.size() == 0) {
		return;
	}
	SetupDeferred();

	

	

	root = SceneNode();
	SceneNode* ground = new SceneNode(&Environment, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for environment
	ground->SetModelScale(Vector3(75.0f, 75.0f, 75.0f));
	root.AddChild(ground);

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


	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

Renderer::~Renderer(void)	{
	delete environmentShader;
	delete camera;

	delete pointLightShader;
	delete combineShader;
	delete sphere;
	delete quad;
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);

	glDeleteFramebuffers(1, &gBufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	gameFrameTime = dt;
	root.Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawShadowScene();
	DrawEnvironment();
	DrawLights();
	CombineBuffers();
	//DrawPostProcessing();
}

void Renderer::DrawEnvironment() {
	BindShader(environmentShader);
	/*modelMatrix = root.GetWorldTransform() * Matrix4::Scale(root.GetModelScale());
	root.Draw(*this);*/
	DrawNode(&root);
}

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(sun->GetPosition(),
		Vector3(0, 0, 0));
	projMatrix = Matrix4::Perspective(1.0f, 100.0f, 1.0f, 45.0f);
	shadowMatrix = projMatrix * viewMatrix;

	DrawNode(&root);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Shader* nodeShader = n->GetShader();
		//BindShader(nodeShader);
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();

		Vector4 nodeCol = n->GetColour();
		glUniform4fv(glGetUniformLocation(nodeShader->GetProgram(), "nodeColour"), 1, (float*)&nodeCol);
		glUniform1i(glGetUniformLocation(nodeShader->GetProgram(), "useTexture"), 0);
		n->Draw(*this);
	}
	else if (n->GetGLTFScene()) {
		Shader* nodeShader = n->GetShader();
		//BindShader(nodeShader);
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();

		n->Draw(*this);
	}
	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i) {
		DrawNode(*i);
	}
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
