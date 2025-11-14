#pragma once
#include "particle.h"
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"
#include "../nclgl/extra/OGLTexture.h"
#include <vector>

class Emitter
{
public:
	Emitter(Vector4 colour, unsigned int amount, UniqueOGLTexture* texture);
	~Emitter();

	void Update(float dt);
	void setColour(const Vector4& col) { particleColour = col; }
	Vector4 getColour() { return particleColour; }

	void setPosition(const Vector3& pos) { position = pos; }
	Vector3 getPosition() { return position; }

	void setParticleNumber(unsigned int num) { particles.resize(num); }
	unsigned int getParticleNumber() { return particles.size(); }


protected:
	Vector3 position;
	Vector4 particleColour;
	UniqueOGLTexture* texture;
	std::vector<Particle> particles;
	float emitTimer;
	float emitRate;

};