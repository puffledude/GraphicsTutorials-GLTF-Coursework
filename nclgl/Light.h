#pragma once

#include "Vector3.h"
#include "Vector4.h"

class Light {
public:
	Light() {};

	/// <summary>
	/// Constructor for light
	/// </summary>
	/// <param name="pos">Positional coordinate</param>
	/// <param name="col"> Colour of the light</param>
	/// <param name="radius">How far you want the light to go</param>
	/// <param name="spec">(Optional) Specularity of the light</param>
	Light(const Vector3& pos, const Vector4& col, float radius, const Vector4& spec = Vector4(1,1,1,1))
	{
		this->position = pos;
		this->colour = col;
		this->radius = radius;
		this->specular = spec;
		this->defaultRadius = radius; // store the original intended radius
	}

	~Light(void) {};


	const Vector3& GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }

	const Vector4& GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

	const Vector4& GetSpec() const { return specular; }
	void SetSpec(const Vector4& val) { specular = val; }

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	// Returns the original configured radius for this light (unchanged by runtime effects)
	float GetDefaultRadius() const { return defaultRadius; }


protected:
	Vector3 position;
	Vector4 colour;
	Vector4 specular;
	float	radius;
	float	defaultRadius{0.0f};
};