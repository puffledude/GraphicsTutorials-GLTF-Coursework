#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../NCLGL/Camera.h"
#include "../nclgl/Light.h"
#include "../NCLGL/Extra/GLTFLoader.h"

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);
	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;
protected:

	void DrawStaticScene();
	void DrawAnimatedScene();

	GLTFScene animatedScene;
	GLTFScene staticScene;


	//std::vector<Matrix4> skeleton;

	Shader* skeletonShader;
	Shader* staticShader;

	Camera* camera;

	int currentFrame = 0.0f;
	float frameTime = 0.0f;

	float gameFrameTime = 0.0f;
};
