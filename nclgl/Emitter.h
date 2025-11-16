#pragma once
#include "particle.h"
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"
#include "../nclgl/extra/OGLTexture.h"
#include <vector>

class Emitter
{
public:
	Emitter(Vector3 position, unsigned int amount, Vector4 colour, Mesh shape);
	~Emitter();

	void Update(float dt);


	void setPosition(const Vector3& pos) { position = pos; }
	Vector3 getPosition() { return position; }

	const std::vector<Particle>& GetParticles() const { return particles; }


	void setParticleNumber(unsigned int num) { particles.resize(num); }
	unsigned int getParticleNumber() { return particles.size(); }


protected:
	void Emit(unsigned int count);
	void updateAtributeData();
	std::vector<InstanceAtributes> GetAttributeData() const;
	Mesh particleMesh;
	Vector4 particleColour;
	UniqueOGLTexture* texture;
	Vector3 position;
	std::vector<Particle> particles;
	unsigned int maxParticles;
	int aliveCount;
	float emitTimer;
	float emitRate;

};