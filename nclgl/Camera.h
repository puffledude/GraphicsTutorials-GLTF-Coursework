/******************************************************************************
Class:Camera
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description:FPS-Style camera. Uses the mouse and keyboard from the Window
class to get movement values!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Window.h"
#include "Matrix4.h"
#include "Vector3.h"
#include "CameraRail.h"

class Camera	{
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;
		this->roll = 0.0f;
		cameraSpeed = 10.0f;
	};

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->roll = 0.0f;
		this->position = position;
		cameraSpeed = 10.0f;
	}

	~Camera(void){};

	void UpdateCamera(float dt = 10.0f);

	//Builds a view matrix for the current camera variables, suitable for sending straight
	//to a vertex skeletonShader (i.e it's already an 'inverse camera matrix').
	Matrix4 BuildViewMatrix();

	//Gets position in world space
	Vector3 GetPosition() const { return position;}
	//Sets position in world space
	void	SetPosition(Vector3 val) { position = val;}

	//Gets yaw, in degrees
	float	GetYaw()   const { return yaw;}
	//Sets yaw, in degrees
	void	SetYaw(float y) {yaw = y;}

	//Gets pitch, in degrees
	float	GetPitch() const { return pitch;}
	//Sets pitch, in degrees
	void	SetPitch(float p) {pitch = p;}

	CameraRail* getRail(){ return rail; }

	void SetRail(CameraRail* track) {
		rail = track;
		freeCam = false;
	}

protected:
	float cameraSpeed;
	float yaw;
	float pitch;
	float roll;
	bool freeCam =true;
	CameraRail* rail;
	int railIndex = 0;
	Vector3 position;
	Vector3 targetPos = Vector3(0,0,0);
	Vector2 targetRot = Vector2(0,0);
};