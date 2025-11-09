#pragma once
#include "Extra/GLTFLoader.h"
#include "HeightMap.h"
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>


class SceneNode{

public:
	SceneNode();
	SceneNode(Mesh* m,Vector4 colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f), Shader* shader=nullptr);
	SceneNode(GLTFScene* gltfScene, Vector4 colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f), Shader* shader=nullptr);
	SceneNode(HeightMap* heightMap, Vector4 colour = Vector4(1.0f, 1.0f, 1.0f, 1.0f), Shader* shader = nullptr);
	~SceneNode(void);

	void SetTransform(const Matrix4& matrix) {
		transform = matrix;
	}

	const Matrix4& GetTransform() const {
		return transform;
	}
	Matrix4 GetWorldTransform() const {
		return worldTransform;
	}

	Vector4 GetColour() const {
		return colour;
	}

	void SetColour(Vector4 updated) {
		colour = updated;
	}


	Vector3 GetModelScale() const {
		return modelScale;
	}

	void SetModelScale(Vector3 scale) {
		modelScale = scale;
	}

	Mesh* GetMesh() const {
		return mesh;
	}

	void SetMesh(Mesh* m) {
		mesh = m;
	}

	GLTFScene* GetGLTFScene() const {
		return gltfScene;
	}
	void SetGLTFScene(GLTFScene* scene) {
		gltfScene = scene;
	}


	Shader* GetShader() {
		return shader;
	}
	void SetShader(Shader* s) {
		shader = s;
	}
	HeightMap* GetHeightMap() const {
		return heightMap;
	}
	void SetHeightMap(HeightMap* hm) {
		heightMap = hm;
	}

	void AddChild(SceneNode* s);

	void DeleteChild(SceneNode* s);

	virtual void Update(float dt);
	virtual void Draw(const OGLRenderer &r);

	std::vector<SceneNode*>::const_iterator GetChildIteratorStart(){
		return children.begin();
	};

	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd(){
		return children.end();
	};

	float GetBoundingRadius() const {
		return boundingRadius;
	}

	void SetBoundingRadius(float r) {
		boundingRadius = r;
	}

	float GetCameraDistance() const {
		return distanceFromCamera;
	}

	void SetCameraDistance(float d) {
		distanceFromCamera = d;
	}

	void SetTexture(GLuint tex) {
		texture = tex;
	}

	GLuint GetTexture() const {
		return texture;
	}

	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) {
		return (a->distanceFromCamera < b->distanceFromCamera) ? true: false;
	}


protected:

	SceneNode* parent;
	Mesh* mesh;
	
	GLTFScene* gltfScene;
	HeightMap* heightMap;
	Shader* shader;
	Matrix4 worldTransform;
	Matrix4 transform;
	Vector3 modelScale;
	Vector4 colour;
	std::vector<SceneNode*> children;
	float distanceFromCamera;
	float boundingRadius;
	GLuint texture;


};