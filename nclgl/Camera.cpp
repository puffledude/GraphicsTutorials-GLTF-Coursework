#include "Camera.h"
#include "Mouse.h"

void Camera::UpdateCamera(float msec)	{
	//Update the mouse by how much

	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	//Bounds check the pitch, to be between straight up and straight down ;)
	pitch = std::min(pitch,90.0f);
	pitch = std::max(pitch,-90.0f);

	if(yaw <0) {
		yaw+= 360.0f;
	}
	if(yaw > 360.0f) {
		yaw -= 360.0f;
	}

	msec *= 5.0f;

	if (Window::GetMouse()->ButtonDown(MOUSE_FOUR)) {
		msec *= 5.0f;
	}

	if(Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += Matrix4::Rotation(yaw, Vector3(0,1,0)) * Vector3(0,0,-1) * msec;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= Matrix4::Rotation(yaw, Vector3(0,1,0)) * Vector3(0,0,-1) * msec;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position += Matrix4::Rotation(yaw, Vector3(0,1,0)) * Vector3(-1,0,0) * msec;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position -= Matrix4::Rotation(yaw, Vector3(0,1,0)) * Vector3(-1,0,0) * msec;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += msec;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= msec;
	}
}

/*
Generates a view matrix for the camera's viewpoint. This matrix can be sent
straight to the skeletonShader...it's already an 'inverse camera' matrix.
*/
Matrix4 Camera::BuildViewMatrix()	{
	//Why do a complicated matrix inversion, when we can just generate the matrix
	//using the negative values ;). The matrix multiplication order is important!
	return	Matrix4::Rotation(-pitch, Vector3(1,0,0)) * 
			Matrix4::Rotation(-yaw, Vector3(0,1,0)) * 
			Matrix4::Translation(-position);
};
