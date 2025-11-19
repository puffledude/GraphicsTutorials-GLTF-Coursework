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
	cone = Mesh::LoadFromMeshFile("Cone.msh");
	
	this->SetupDeferred();
	this->SetupShadow();
	this->LoadEnvironment();
	this->LoadSkyBox();
	this->LoadWater();
	this->SetUpPostProcessing();
	Shader* fireShader = new Shader("FireVertexShader.glsl", "FireFragmentShader.glsl");
	fireEmitter = new Emitter(Vector3(28.0235, 38.3, 35.7914), 100, Vector4(1, 0.5, 0, 1), nullptr, fireShader);
	fireTex = OGLTexture::TextureFromFile(TEXTUREDIR"fire.png");
	
	
	
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
	winterEnvironmentShader = new Shader("SnowyEnvironmentVertex.glsl", "SnowyEnvironmentFragment.glsl");
	skeletonShader = new Shader("DeferredSkinningVertex.glsl", "bufferFragment.glsl");
	if (!environmentShader->LoadSuccess() ||
		!pointLightShader->LoadSuccess() ||
		!combineShader->LoadSuccess()){
		return;
	}

	glGenFramebuffers(1, &gBufferFBO);
	glGenFramebuffers(1, &pointLightFBO);
	glGenFramebuffers(1, &combineFBO);

	GLenum buffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(bufferMaterialTex);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);
	GenerateScreenTexture(combineTex);

	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, bufferColourTex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, bufferNormalTex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
		GL_TEXTURE_2D, bufferMaterialTex, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, bufferDepthTex, 0);

	glDrawBuffers(3, buffers);

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

	glBindFramebuffer(GL_FRAMEBUFFER, combineFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, combineTex, 0);

	//Need to move these to where combineTex is generated
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, combineTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glDrawBuffers(1, buffers);

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

void Renderer::SetUpPostProcessing() {
	basicOutShader = new Shader("SceneOutVertex.glsl", "SceneOutFragment.glsl");
	transitionShader = new Shader("TransitionShaderVertex.glsl", "TransitionShaderFragment.glsl");
	FXAAShader = new Shader("FXAAVertex.glsl", "FXAAFragment.glsl");
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
	GLTFLoader::Load("../GLTF/Campfire/Campfire.gltf", Campfire);
	if (Campfire.meshes.size() == 0) {
		return;
	}
	GLTFLoader::Load("../GLTF/Tent/Tent.gltf", Tent);
	if (Tent.meshes.size() == 0) {
		return;
	}
	GLTFLoader::Load("../GLTF/Cabin/Cabin.gltf", Cabin);
	if (Cabin.meshes.size() == 0) {
		return;
	}
	GLTFLoader::Load("../GLTF/Snowman/Snowman.gltf", Snowman);
	if (Snowman.meshes.size() == 0) {
		return;
	}
	GLTFLoader::Load("../GLTF/Bird/Bird.gltf", Bird);
	if (Bird.meshes.size() == 0) {
		return;
	}
	/*Snowman = Mesh::LoadFromMeshFile("Snowman.msh");
	SnowmanMat = new MeshMaterial("Snowman.Mat");*/
	summerPointLights = vector<Light*>();
	winterPointLights = vector<Light*>();
	sun = new Light(Vector3(23.6744, 58.4126, 3.97436), Vector4(1, 1, 1, 1), 100.0f);
	pointLights = &summerPointLights;
	loadSummerScene();
	pointLights = &winterPointLights;
	loadWinterScene();

	
	
	
	centre = new Light(Vector3(36.8914, 30.1335, 34.4191), Vector4(0, 0, 0, 0), 0);
}

void Renderer::loadSummerScene() {
	this->summerRoot = SceneNode();
	SceneNode* ground = new SceneNode(&Environment, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for environment
	ground->SetModelScale(Vector3(75.0f, 75.0f, 75.0f));
	ground->SetBoundingRadius(1000.0f);
	summerRoot.AddChild(ground);
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
		treeNode->SetBoundingRadius(7.0f);
		ground->AddChild(treeNode);
	}
	SceneNode* campfireNode = new SceneNode(&Campfire, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for campfire
	campfireNode->SetTransform(Matrix4::Translation(Vector3(28.9073, 37.9502, 35.55)) * Matrix4::Rotation(90, Vector3(-90, 0, 0)));
	campfireNode->SetModelScale(Vector3(0.0003f, 0.0003f, 0.0003f));
	campfireNode->SetBoundingRadius(3.0f);
	ground->AddChild(campfireNode);

	SceneNode* tentNode = new SceneNode(&Tent, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for tent
	tentNode->SetTransform(Matrix4::Translation(Vector3(30.5615, 38.3325, 35.5342)) * Matrix4::Rotation(-90, Vector3(0, 1, 0)));
	tentNode->SetModelScale(Vector3(0.01f, 0.01f, 0.01f));
	tentNode->SetBoundingRadius(5.0f);
	ground->AddChild(tentNode);
	
	SceneNode* birdNode = new SceneNode(&Bird, Vector4(1, 1, 1, 1), skeletonShader); //Scenenode for bird
	birdNode->SetTransform(Matrix4::Translation(Vector3(40.0f, 40.0f, 34.0f)));
	birdNode->SetModelScale(Vector3(0.3f, 0.3f, 0.3f));
	birdNode->SetBoundingRadius(2.0f);
	ground->AddChild(birdNode);

	pointLights->push_back(sun);

	Light* campFireLight = new Light(Vector3(27.4, 38.9918, 34.3), Vector4(0.5, 0.5, 0, 1), 5.0f);
	pointLights->push_back(campFireLight);
	//this->fireEmitter->SetLight(campFireLight);
}


void Renderer::loadWinterScene() {

	SceneNode* ground = new SceneNode(&Environment, Vector4(1, 1, 1, 1), winterEnvironmentShader); //Scenenode for environment
	ground->SetModelScale(Vector3(75.0f, 75.0f, 75.0f));
	ground->SetBoundingRadius(1000.0f);
	winterRoot.AddChild(ground);

	Vector3 treePositions[] = {
		Vector3(44.5433, 36.0f, 26.3742),
		Vector3(50.0999,37.3,33.0351),
		Vector3(45.7918,37.0f,40.43),
		Vector3(37.3799,37.4,51.1133),
		Vector3(33.1454,36.44,43.647),
		Vector3(44.4436,36.4546,48.1839) };
	for (Vector3 pos : treePositions) {
		SceneNode* treeNode = new SceneNode(&Tree, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for trees
		treeNode->SetTransform(Matrix4::Translation(pos));
		treeNode->SetModelScale(Vector3(0.1, 0.1, 0.1));
		treeNode->SetBoundingRadius(7.0f);
		ground->AddChild(treeNode);
	}

	SceneNode* campfireNode = new SceneNode(&Campfire, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for campfire
	campfireNode->SetTransform(Matrix4::Translation(Vector3(28.9073, 37.9502, 35.55)) * Matrix4::Rotation(90, Vector3(-90, 0, 0)));
	campfireNode->SetModelScale(Vector3(0.0003f, 0.0003f, 0.0003f));
	campfireNode->SetBoundingRadius(3.0f);
	ground->AddChild(campfireNode);
	Light* campFireLight = new Light(Vector3(27.4, 38.9918, 34.3), Vector4(0.5, 0.5, 0, 1), 5.0f);

	pointLights->push_back(sun);
	pointLights->push_back(campFireLight);
	
	SceneNode* houseNode = new SceneNode(&Cabin, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for house
	houseNode->SetTransform(Matrix4::Translation(Vector3(37.8515, 38.0f, 37.5047)) * Matrix4::Rotation(-90, Vector3(1, 0, 0)));
	//*Matrix4::Rotation(30, Vector3(0,1,0)));
	houseNode->SetModelScale(Vector3(0.5f, 0.5f, 0.5f));
	ground->AddChild(houseNode);

	SceneNode* secondHouseNode = new SceneNode(&Cabin, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for house
	secondHouseNode->SetTransform(Matrix4::Translation(Vector3(26.5793, 37.8, 41.7133)) * Matrix4::Rotation(-90, Vector3(1, 0, 0)));
	secondHouseNode->SetModelScale(Vector3(0.5f, 0.5f, 0.5f));
	ground->AddChild(secondHouseNode);

	SceneNode* snowmanNode = new SceneNode(&Snowman,Vector4(1, 1, 1, 1), environmentShader); //Scenenode for snowman
	//SceneNode* snowmanNode = new SceneNode(&Snowman, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for snowman
	snowmanNode->SetTransform(Matrix4::Translation(Vector3(32.9472, 37.0f, 19.8575)) *Matrix4::Rotation((-90), Vector3(1,0,0)) * Matrix4::Rotation(-115, Vector3(0, 0, 1)));
	ground->AddChild(snowmanNode);

	SceneNode* secondSnowmanNode = new SceneNode(&Snowman, Vector4(1, 1, 1, 1), environmentShader); //Scenenode for snowman
	secondSnowmanNode->SetTransform(Matrix4::Translation(Vector3(29.801, 37.2549, 52.9451)) * Matrix4::Rotation(30, Vector3(0, 1, 0)) * Matrix4::Rotation((-90), Vector3(1, 0, 0)));
	ground->AddChild(secondSnowmanNode);

	SceneNode* birdNode = new SceneNode(&Bird, Vector4(1, 1, 1, 1), skeletonShader); //Scenenode for bird
	birdNode->SetTransform(Matrix4::Translation(Vector3(40.0f, 35.0f, 20.0f)));
	birdNode->SetModelScale(Vector3(0.1f, 0.1f, 0.1f));
	birdNode->SetBoundingRadius(2.0f);
	ground->AddChild(birdNode);
	/*sun = new Light(Vector3(23.6744, 58.4126, 3.97436), Vector4(1, 1, 1, 1), 100.0f);
	pointLights->push_back(sun);*/
}


void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	gameFrameTime = dt;
	summerRoot.Update(dt);
	winterRoot.Update(dt);
	fireEmitter->Update(dt);
	transitionTimer -= dt;
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
}


Renderer::~Renderer(void)	{
	delete environmentShader;
	delete camera;

	delete pointLightShader;
	delete combineShader;
	delete shadowShader;
	delete skyboxShader;
	delete waterShader;

	delete transitionShader;
	delete FXAAShader;
	delete basicOutShader;

	delete sphere;
	delete quad;
	glDeleteTextures(1, &bufferColourTex);
	glDeleteTextures(1, &bufferNormalTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteTextures(1, &lightDiffuseTex);
	glDeleteTextures(1, &lightSpecularTex);
	glDeleteTextures(1, &combineTex);
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteFramebuffers(1, &gBufferFBO);
	glDeleteFramebuffers(1, &pointLightFBO);
	glDeleteFramebuffers(1, &combineFBO);
}

void Renderer::switchSeason() {

	isSummer = !isSummer;
	if (pointLights == &summerPointLights) {
		pointLights = &winterPointLights;
		
	}
	else {
		pointLights = &summerPointLights;
	}
	fireEmitter->SetLight((*pointLights)[1]);
}

/// <summary>
/// Renders Scene. Skybox -> Shadow Scene -> Environment fill -> Lighting calc -> Combine Buffers.
/// </summary>
void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//DrawSkybox();  
	DrawShadowScene();

	DrawEnvironment();

	DrawLights();

	CombineBuffers();

	//std::cout << "Camera location is : " << camera->GetPosition()<< std::endl;
	DrawPostProcessing();
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
	if (isSummer) { DrawNode(&summerRoot, shadow); }
	else { DrawNode(&winterRoot, shadow); }
	DrawWater(shadow);
	modelMatrix.ToIdentity();

	if (!shadow && fireEmitter) {
		// Particles store world positions in the instance data, so use identity model matrix
		modelMatrix.ToIdentity();

		// Make sure view/proj are the camera's (shadow pass may have changed them)
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
			(float)width / (float)height, 45.0f);

		// Bind particle shader and upload matrices/uniforms
		glDisable(GL_CULL_FACE);
		Shader* fireShader = fireEmitter->GetShader();
		BindShader(fireShader);
		glUniform1i(glGetUniformLocation(fireShader->GetProgram(),
			"diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fireTex->GetObjectID());
		UpdateShaderMatrices();
	}

	fireEmitter->Emit();
	if (!shadow && fireEmitter) { glEnable(GL_CULL_FACE); }
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
		centre->GetPosition());
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
		"materialTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bufferMaterialTex);

	glUniform1i(glGetUniformLocation(pointLightShader->GetProgram(),
		"shadowTex"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	const Vector3 camPos = camera->GetPosition();
	glUniform3fv(glGetUniformLocation(pointLightShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

	glUniform2f(glad_glGetUniformLocation(pointLightShader->GetProgram(),
		"pixelSize"), 1.0f / width, 1.0f / (float)height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();
	glUniformMatrix4fv(glGetUniformLocation(pointLightShader->GetProgram(),
		"invProjViewMatrix"), 1, false, invViewProj.values);

	UpdateShaderMatrices();
	for(Light* l : *pointLights) {
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
	//glEnable(GL_DEPTH_TEST);
	UpdateShaderMatrices();
	quad->Draw();

}


void Renderer::CombineBuffers() {
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glBindFramebuffer(GL_FRAMEBUFFER, combineFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawSkybox();
	BindShader(combineShader);

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
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawPostProcessing() {

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, combineTex);
	glGenerateMipmap(GL_TEXTURE_2D); 

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);


	if (transitionTimer > 0.0f){
		//Idea. Generate mipmaps out of the combine texture.
		//Then sample the mipmaps as time goes on. Lower mipmaps blown up are blurrier.
		BindShader(transitionShader);
		for (int i = 0; i < 5; i++) {

			glUniform1i(glGetUniformLocation(transitionShader->GetProgram(),
				"sceneTex"), 0);
			//glActiveTexture(GL_TEXTURE0);
			glUniform1f(glGetUniformLocation(transitionShader->GetProgram(),
				"time"), transitionTimer);
			glUniform1i(glGetUniformLocation(transitionShader->GetProgram(),
				"isVertical"), 0);
			quad->Draw();

			glUniform1i(glGetUniformLocation(transitionShader->GetProgram(),
				"isVertical"), 1);
			quad->Draw();

		}
		if (!transitioned) {
			switchSeason();
			transitioned = true;
		}
		
	}

	else if (useFXAA) {
		BindShader(FXAAShader);
		glUniform1i(glGetUniformLocation(FXAAShader->GetProgram(),
			"sceneTex"), 0);
		
		glUniform1i (glGetUniformLocation(FXAAShader->GetProgram(),
			"depthTex"), 1);
		
		glUniform1i(glGetUniformLocation(FXAAShader->GetProgram(),
			"width"), width);
		glUniform1i(glGetUniformLocation(FXAAShader->GetProgram(),
			"height"), height);
		quad->Draw();
	}
	else {
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		BindShader(basicOutShader);
		glUniform1i(glGetUniformLocation(basicOutShader->GetProgram(),
			"sceneTex"), 0);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, combineTex);
		glUniform1i(glGetUniformLocation(basicOutShader->GetProgram(),
			"depthTex"), 1);
		quad->Draw();
	}

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
	glDisable(GL_BLEND);
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
	glEnable(GL_BLEND);
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
