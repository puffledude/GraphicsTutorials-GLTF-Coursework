#include "Renderer.h"
#include "../nclgl/Extra/GLTFLoader.h"

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f,
		(float)width / (float)height, 45.0f);
	camera = new Camera(0.0f, 180.0f, Vector3(-0.5f, 1.5, -3.0f));

	GLTFLoader::Load("../GLTF/CesiumMan/CesiumMan.gltf", animatedScene);
	GLTFLoader::Load("../GLTF/Tree/Tree.gltf", staticScene);

	if (staticScene.meshes.size() == 0 ||
		animatedScene.meshes.size() == 0) {
		return;
	}
	skeletonShader = new Shader("SkinningVertex.glsl","TexturedFragment.glsl");
	if(!skeletonShader->LoadSuccess()) {
		return;
	}

	staticShader = new Shader("staticVertex.glsl", "TexturedFragment.glsl");

	if (!staticShader->LoadSuccess()) {
		return;
	}

	init = true;
}

Renderer::~Renderer(void)	{
	delete skeletonShader;
	delete staticShader;
	delete camera;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	gameFrameTime = dt;
}

void Renderer::RenderScene()	{
	glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);	

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	DrawStaticScene();
	DrawAnimatedScene();
}

void Renderer::DrawStaticScene() {
	SharedMesh		mesh		= staticScene.meshes[0];
	GLTFMaterial	material	= staticScene.materials[0];

	BindShader(staticShader);
	glUniform1i(glGetUniformLocation(staticShader->GetProgram(), "diffuseTex"), 0);

	modelMatrix = Matrix4::Translation({ -1,1.5,0 })
				/** Matrix4::Rotation(180.0f, Vector3(0, 1, 0))
				* Matrix4::Rotation(90.0f, Vector3(1, 0, 0))*/
				* Matrix4::Scale({ 75,75,75 });

	UpdateShaderMatrices();

	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {	
		//Material layer n refers to submesh n
		GLTFMaterialLayer& layer = material.allLayers[i];
		glBindTexture(GL_TEXTURE_2D, layer.albedo->GetObjectID());
		mesh->DrawSubMesh(i);
	}
}


void Renderer::DrawAnimatedScene() {
	SharedMesh		mesh		= animatedScene.meshes[0];
	//The loader has a limitation in that animations are stored to 
	//work just for the first animated mesh in the file. Not a problem if it's just 1!
	SharedMeshAnim	anim		= animatedScene.animations[0];
	GLTFMaterial	material	= animatedScene.materials[0];

	frameTime -= gameFrameTime;

	if (frameTime <= 0) {
		currentFrame++;

		frameTime += 1.0f / anim->GetFrameRate();
		currentFrame = currentFrame % anim->GetFrameCount();

		const Matrix4* inverseBindPose = mesh->GetInverseBindPose();

		const Matrix4* joints = anim->GetJointData(currentFrame);

		for (int i = 0; i < skeleton.size(); ++i) {
			skeleton[i] = joints[i] * inverseBindPose[i];
		}
	}

	BindShader(skeletonShader);
	glUniform1i(glGetUniformLocation(skeletonShader->GetProgram(), "diffuseTex"), 0);

	modelMatrix = Matrix4::Translation({ 0,0,0 })
		* Matrix4::Rotation(90.0f, Vector3(0, 1, 0))
		* Matrix4::Rotation(270.0f, Vector3(1, 0, 0))	
		* Matrix4::Scale({ 1,1,1 });

	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;
	frameMatrices.resize(mesh->GetJointCount());

	const Matrix4* invBindPose	= mesh->GetInverseBindPose();
	const Matrix4* frameData	= anim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
		frameMatrices[i] = (frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(skeletonShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false,
		(float*)frameMatrices.data());

	glActiveTexture(GL_TEXTURE0);
	for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
		
		//Material layer n refers to submesh n
		GLTFMaterialLayer& layer = material.allLayers[i];

		glBindTexture(GL_TEXTURE_2D, layer.albedo->GetObjectID());
		mesh->DrawSubMesh(i);
	}
}
