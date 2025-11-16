#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "glad/glad.h"

struct Particle {
	Vector3 position;
	Vector3 direction;
	Vector4 colour;
	float velocity;
	float startLife;
	float size;
	float life;
};

struct InstanceData {
	Vector3 pos;
	Vector4 colour;
	float size;
};

struct InstanceAtributes {
	GLuint location;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	const void* offset;
};