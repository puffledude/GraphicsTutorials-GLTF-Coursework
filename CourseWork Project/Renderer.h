#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../NCLGL/Camera.h"
#include "../NCLGL/Extra/GLTFLoader.h"
#include "../NCLGL/SceneNode.h"
#include "../nclgl/Light.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene()				override;
	void UpdateScene(float dt)	override;
protected:
	void DrawNode(SceneNode* n);
	GLTFScene Environment;

	//For Lights
	Mesh* sphere;
	Light* sun;

	vector<Light*> pointLights;


	void DrawEnvironment();

	//For deferred rendering.
	Shader* environmentShader;
	Shader* pointLightShader;
	Shader* combineShader;
	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void SetupDeferred();



	//For shadow Mapping.
	Shader* shadowShader;
	GLuint shadowFBO;
	GLuint shadowTex;
	void SetupShadow();
	void DrawShadowScene();



	//gBuffer
	GLuint gBufferFBO;
	GLuint bufferColourTex;
	GLuint bufferNormalTex;
	GLuint bufferDepthTex;

	//Point light stuff
	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;

	unsigned int SHADOWSIZE = 2048;
	Mesh* quad;

	SceneNode root;
	Camera* camera;

	int currentFrame = 0.0f;
	float frameTime = 0.0f;

	float gameFrameTime = 0.0f;
};
