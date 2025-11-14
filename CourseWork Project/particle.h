#pragma once
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"

struct Particle {
	Vector3 position;
	Vector3 direction;
	Vector4 colour;

	float life;

};