#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../NCLGL/Camera.h"
#include "../NCLGL/Extra/GLTFLoader.h"
#include "../NCLGL/SceneNode.h"
#include "../nclgl/Light.h"
#include "../nclgl/Emitter.h"
#include "../nclgl/CameraRail.h"
#include "waterStruct.h"
class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene()				override;
	void UpdateScene(float dt)	override;

	

	void startTransition() {
		transitionTimer = transitionDuration;
		transitioned = false;
	}
	void setFXAA() { useFXAA = !useFXAA; }
	void setDOF() { useDOF = !useDOF; }
	void OutputCameraPos();

protected:
	void DrawNode(SceneNode* n, bool shadow=false);
	
	//GLTF Models
	GLTFScene Environment;
	GLTFScene Tree;
	GLTFScene Campfire;
	GLTFScene Tent;
	GLTFScene Cabin;
	GLTFScene Snowman;
	GLTFScene Bird;

	Shader* skeletonShader;

	void LoadEnvironment();

	//Meshes
	Mesh* sphere;
	Mesh* cone;
	Mesh* quad;
	//Mesh* tree;

	//Lights
	Light* sun;
	vector<Light*> summerPointLights;
	vector<Light*> winterPointLights;
	vector<Light*>* pointLights;

	//Centre point
	Light* centre;

	void DrawEnvironment(bool shadow=false);
	
	void DrawLights();
	void CombineBuffers();

	//For deferred rendering.
	Shader* environmentShader;
	Shader* winterEnvironmentShader;
	Shader* pointLightShader;
	Shader* combineShader;
	void GenerateScreenTexture(GLuint& into, bool depth = false);
	void SetupDeferred();

	//For SSAO
	void DrawSSAO();
	Shader* ssaoShader;
	Shader* ssaoBlurShader;
	GLuint gpositionTex;
	GLuint noiseTexture;
	std::vector<Vector3> ssaoKernel;

	GLuint ssaoFBO;
	GLuint ssaoColourBuffer;
	GLuint ssaoBlurFBO;
	GLuint ssaoColorBufferBlur;


	//For particle emitting
	Emitter* fireEmitter;
	UniqueOGLTexture fireTex;

	//For shadow Mapping.
	Shader* shadowShader;
	GLuint shadowFBO;
	GLuint shadowTex;
	void SetupShadow();
	void DrawShadowScene();

	//For Skybox
	Shader* skyboxShader;
	UniqueOGLTexture cubeMap;
	void LoadSkyBox();
	void DrawSkybox(bool shadow = false);

	//gBuffer
	GLuint gBufferFBO;
	GLuint bufferColourTex;
	GLuint bufferNormalTex;
	GLuint bufferDepthTex;
	GLuint bufferMaterialTex;

	//Point light stuff
	GLuint pointLightFBO;
	GLuint lightDiffuseTex;
	GLuint lightSpecularTex;

	//So I can post process
	GLuint combineFBO;
	GLuint combineTex;

	Shader* transitionShader;
	Shader* basicOutShader;
	Shader* FXAAShader;
	Shader* DOFShader;
	bool useDOF = false;
	bool useFXAA = false;
	void SetUpPostProcessing();
	void DrawPostProcessing();
	void switchSeason();

	//Water stuff
	UniqueOGLTexture waterTex;
	Shader* waterShader;
	Shader* iceShader;
	UniqueOGLTexture iceTex;
	void LoadWater();
	void DrawWater(bool shadow = false);


	unsigned int SHADOWSIZE = 2048;
	

	SceneNode summerRoot;
	void loadSummerScene();
	SceneNode winterRoot;
	void loadWinterScene();
	bool isSummer = true;
	


	Camera* camera;
	CameraRail cameraRail;
	
	void SetupCameraRail();

	int currentFrame = 0.0f;
	float frameTime = 0.0f;
	float gameFrameTime = 0.0f;

	float transitionDuration = 4.0f;
	float transitionTimer = 0.0f;
	bool transitioned = true;
};
