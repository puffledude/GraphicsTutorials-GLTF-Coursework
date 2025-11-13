#include "SceneNode.h"



SceneNode::SceneNode()
{
	mesh = nullptr;
	this->material = nullptr;
	this->animation = nullptr;
	gltfScene = nullptr;
	heightMap = nullptr;
	shader = nullptr;
	parent = NULL;
	modelScale = Vector3(1.0f, 1.0f, 1.0f);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

/// <summary>
/// Scene Node Constructor
/// </summary>
/// <param name="m"> Pointer to a mesh object</param>
/// <param name="colour">Colour of the underlying mesh</param>
/// <param name="shader">Shader tied to the node. Cannot bind the shader in the node though, has to be bound in the renderer.</param>
SceneNode::SceneNode(Mesh* m, MeshMaterial* mat, MeshAnimation* anim ,Vector4 colour, Shader* shader)
{
	this->mesh = m;
	this->material = mat;
	this->animation = anim;
	if (material) {
		for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
			const MeshMaterialEntry* matEntry =
				material->GetMaterialForLayer(i);
			const string* filename = nullptr;
			matEntry->GetEntry("Diffuse", &filename);
			string path = TEXTUREDIR + *filename;
			//UniqueOGLTexture texture = 
			// move the unique_ptr into the vector
			materialTextures.emplace_back(OGLTexture::TextureFromFile(path.c_str()));
		}
	}


	this->gltfScene = nullptr;
	this->heightMap = nullptr;
	this->colour = colour;
	this->shader = shader;
	parent = NULL;
	modelScale = Vector3(1.0f, 1.0f, 1.0f);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

/// <summary>
/// Scene Node Constructor
/// </summary>
/// <param name="gltfScene"> Pointer to a gltfScene object</param>
/// <param name="colour">Colour of the underlying mesh</param>
/// <param name="shader">Shader tied to the node. Cannot bind the shader in the node though, has to be bound in the renderer.</param>
SceneNode::SceneNode(GLTFScene* gltfScene, Vector4 colour, Shader* shader) {
	this->mesh = nullptr;
	this->material = nullptr;
	this->animation = nullptr;
	this->gltfScene = gltfScene;
	this->colour = colour;
	this->shader = shader;
	parent = NULL;
	modelScale = Vector3(1.0f, 1.0f, 1.0f);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}

SceneNode::SceneNode(HeightMap* heightMap, Vector4 colour, Shader* shader) {
	this->mesh = nullptr;
	this->gltfScene = nullptr;
	this->heightMap = heightMap;
	this->colour = colour;
	this->shader = shader;
	parent = NULL;
	modelScale = Vector3(1.0f, 1.0f, 1.0f);
	boundingRadius = 1.0f;
	distanceFromCamera = 0.0f;
	texture = 0;
}


SceneNode::~SceneNode(void)
{
	for (unsigned int i=0; i<children.size(); ++i) {
		delete children[i];
	}
	children.clear();
}

void SceneNode::AddChild(SceneNode* s)
{
	s->parent = this;
	children.push_back(s);
}

void SceneNode::DeleteChild(SceneNode* s)
{
	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i) {
		if ((*i) == s) {
			// clear parent to avoid dangling parent references inside the object
			(*i)->parent = nullptr;
			delete* i;
			children.erase(i);
			return;
			
			return;
		}
	}
}

void SceneNode::Draw(const OGLRenderer &r, bool shadow)
{
	
	if (mesh) {

		if (material) {
			if (shadow) {
				for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
					
					mesh->DrawSubMesh(i);
				}
				return;
			}
			glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);

			if (animation) {
				vector<Matrix4> frameMatrices;

				const Matrix4* invBindPose = mesh->GetInverseBindPose();
				const Matrix4* frameData = animation->GetJointData(currentFrame);

				for (unsigned int i = 0; i < mesh->GetJointCount(); ++i) {
					frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
				}
				int j = glGetUniformLocation(shader->GetProgram(), "joints");
				glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());
			}
			for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, materialTextures[i]->GetObjectID());
				mesh->DrawSubMesh(i);
			}
		
		}
		else{ mesh->Draw(); }
		
	}
	else if (gltfScene && gltfScene->meshes.size() > 0) {
		SharedMesh		mesh = gltfScene->meshes[0];
		GLTFMaterial	material = gltfScene->materials[0];

		for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
			//Material layer n refers to submesh n
			GLTFMaterialLayer& layer = material.allLayers[i];
			if (shadow) {
				// In shadow pass, only care about depth, so skip texture binding
				mesh->DrawSubMesh(i);
				continue;
			}
			// Bind albedo (diffuse) to texture unit 0 if present
			if (layer.albedo) {
				GLint loc = -1;
				if (shader) loc = glGetUniformLocation(shader->GetProgram(), "diffuseTex");
				if (loc >= 0) glUniform1i(loc, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, layer.albedo->GetObjectID());
			}

			// Bind normal map (bump) to texture unit 1 if present
			if (layer.bump != nullptr) {
				GLint loc = -1;
				if (shader) loc = glGetUniformLocation(shader->GetProgram(), "normalTex");
				if (loc >= 0) glUniform1i(loc, 1);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, layer.bump->GetObjectID());
				// optionally restore active unit back to 0
				//glActiveTexture(GL_TEXTURE0);
			}

			mesh->DrawSubMesh(i);
		}
	}

	else if (heightMap) {
		heightMap->Draw();
	}
}

void SceneNode::Update(float dt)
{
	if (parent) {
		worldTransform = parent->worldTransform * transform;
		frameTime -= dt;
		if (this->animation) {
			while (frameTime < 0.0f) {
				currentFrame = (currentFrame + 1) % animation->GetFrameCount();
				frameTime += 1.0f / animation->GetFrameRate();
			}
		}
	}
	else {
		worldTransform = transform;
		frameTime -= dt;
		if (this->animation != nullptr) {
		while (frameTime < 0.0f) {
			currentFrame = (currentFrame + 1) % animation->GetFrameCount();
			frameTime += 1.0f / animation->GetFrameRate();
			}
		}
	}
	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update(dt);
	}
}