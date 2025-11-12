#pragma once
#include "..\nclgl\Matrix4.h"
#include "..\nclgl\Shader.h"
struct WaterStruct
{
	Shader* waterShader;

	Matrix4 modelMatrix;
};