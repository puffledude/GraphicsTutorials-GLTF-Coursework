#include "Camera.h"
#include "../nclgl/Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt) {
	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);


	if (yaw < 0) {
		yaw += 360.0f;
	}
	else if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	if (rail && !freeCam) {
		if (targetPos == Vector3(0, 0, 0))
		{
			this->position = rail->points[0].first;
			this->yaw = rail->points[0].second.x;
			this->pitch = rail->points[0].second.y;

			targetPos = rail->points[1].first;
			targetRot = rail->points[1].second;
			railIndex = 2; //The next target
		}
		Vector3 posDir = targetPos - this->GetPosition();
		float posDist = posDir.Length();
		posDir.Normalise();
		Vector2 rotDir = targetRot - Vector2(this->yaw, this->pitch);
		float rotDist = rotDir.Length();
		rotDir.Normalize();
		if (posDist < 0.5 && rotDist < 0.5) 
		{
			railIndex++;
			if (railIndex >= rail->points.size()) {
				railIndex = 0;

			}
			targetPos = rail->points[railIndex].first;
			targetRot = rail->points[railIndex].second;
		}

		position += posDir * std::min(rail->velocity * dt, posDist);
		yaw += rotDir.x * std::min(rail->velocity * dt *2, rotDist);
		pitch += rotDir.y * std::min(rail->velocity * dt *2, rotDist);

	}

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float speed = (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) ? cameraSpeed * 2 * dt : cameraSpeed * dt;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * speed;
		freeCam = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * speed;
		freeCam = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * speed;
		freeCam = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * speed;
		freeCam = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q)) {
		roll -= 0.2;
		freeCam = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_E)) {
		roll += 0.2;
		freeCam = true;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y += speed;
		freeCam = true;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_CONTROL)) {
		position.y -= speed;
		freeCam = true;
	}
}

Matrix4 Camera::BuildViewMatrix() {
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Rotation(-roll, Vector3(0, 0, 1)) *
		Matrix4::Translation(-position)
		;

}
