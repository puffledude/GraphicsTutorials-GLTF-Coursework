#pragma once
#include <vector>
#include "Vector3.h"
#include "Vector2.h"

struct CameraRail {
	std::vector<std::pair<Vector3, Vector2>> points;
	float velocity;
};