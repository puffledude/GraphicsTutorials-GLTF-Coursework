#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Extra/GLTFLoader.h"


Renderer::Renderer(Window& parent) : OGLRenderer(parent)	{
	
	
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);
	camera = new Camera(0.0f, 180.0f, Vector3(50, 40, 30.0f));
	
	sphere = Mesh::LoadFromMeshFile("Sphere.msh");
	if (!sphere) {
		return;
	}
	quad = Mesh::GenerateQuad();

	//LoadEnvironment();
	
	environmentShader = new Shader("bumpVertex.glsl", "bufferFragment.glsl");
	pointLightShader = new Shader("pointLightVertex.glsl", "pointLightFragment.glsl");
	combineShader = new Shader("combineVertex.glsl", "combineFragment.glsl");
	if (!environmentShader->LoadSuccess() ||
		!pointLightShader->LoadSuccess()||
		!combineShader->LoadSuccess()) {
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

	this->LoadEnvironment();

	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	init = true;
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

void Renderer::LoadEnvironment() {
	GLTFLoader::Load("../GLTF/Environment/CourseWorkProject.gltf", Environment);
	if (Environment.meshes.size() == 0) {
		return;
	}
	this->root = SceneNode();
	SceneNode* ground = new SceneNode(&Environment, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for environment
	ground->SetModelScale(Vector3(75.0f, 75.0f, 75.0f));
	root.AddChild(ground);
	//Vector3 groundLocation = ground->GetWo();
	sun = new Light(Vector3(0.0f, 70.0f, 0.0f), Vector4(1, 1, 1, 1), 200.0f);
	pointLights.push_back(sun);

}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	gameFrameTime = dt;
	root.Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawEnvironment();
	DrawLights();
	CombineBuffers();
	std::cout << "Camera location is : " << camera->GetPosition()<< std::endl;
	//DrawPostProcessing();
}

void Renderer::DrawEnvironment() {
	//BindShader(environmentShader);
	/*modelMatrix = root.GetWorldTransform() * Matrix4::Scale(root.GetModelScale());
	root.Draw(*this);*/
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	DrawNode(&root);
	/*for (Light* l : pointLights) {
		
		sphere->Draw();
	}*/

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
	/*for (int i = 0; i < pointLights.size(); ++i) {
		Light& l = pointLights[i];
		SetShaderLight(l);
		sphere->Draw();
	}*/

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
	glDepthFunc(GL_LEQUAL);

	glDepthMask(GL_TRUE);

	glClearColor(0.2f, 0.2f, 0.2f, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	quad->Draw();
}



void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Shader* nodeShader = n->GetShader();
		BindShader(nodeShader);
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		UpdateShaderMatrices();

		//glUniformMatrix4fv(glGetUniformLocation(nodeShader->GetProgram(), "modelMatrix"), 1, false, model.values);

		Vector4 nodeCol = n->GetColour();
		glUniform4fv(glGetUniformLocation(nodeShader->GetProgram(), "nodeColour"), 1, (float*)&nodeCol);
		glUniform1i(glGetUniformLocation(nodeShader->GetProgram(), "useTexture"), 0);
		n->Draw(*this);
	}
	else if (n->GetGLTFScene()) {
		Shader* nodeShader = n->GetShader();
		BindShader(nodeShader);
		modelMatrix = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
			(float)width / (float)height, 45.0f);
		UpdateShaderMatrices();

		//glUniformMatrix4fv(glGetUniformLocation(nodeShader->GetProgram(), "modelMatrix"), 1, false, model.values);

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
