#include "SceneNode.h"



SceneNode::SceneNode()
{
	mesh = nullptr;
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
SceneNode::SceneNode(Mesh* m ,Vector4 colour, Shader* shader)
{
	this->mesh = m;
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

void SceneNode::Draw(const OGLRenderer &r)
{
	
	if (mesh) {
		mesh->Draw();
	}
	else if(gltfScene && gltfScene->meshes.size()>0) {
		SharedMesh		mesh = gltfScene->meshes[0];
		GLTFMaterial	material = gltfScene->materials[0];

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
			//Material layer n refers to submesh n
			GLTFMaterialLayer& layer = material.allLayers[i];
			glBindTexture(GL_TEXTURE_2D, layer.albedo->GetObjectID());
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
	}
	else {
		worldTransform = transform;
	}
	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); ++i) {
		(*i)->Update(dt);
	}
}