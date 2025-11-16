#pragma once
#include "particle.h"
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"
#include "../nclgl/extra/OGLTexture.h"
#include <random>
#include <vector>

class Emitter
{
public:
	Emitter(Vector3 position, unsigned int amount, Vector4 colour, Mesh* shape, Shader* shaderProgram);
	~Emitter();

	void Update(float dt);
	void Emit();

	Shader* GetShader() const { return shader; }
	void SetShader(Shader* newShader) { shader = newShader; }

	void setPosition(const Vector3& pos) { position = pos; }
	Vector3 getPosition() { return position; }

	const std::vector<Particle>& GetParticles() const { return particles; }


	void setParticleNumber(unsigned int num) {
		maxParticles = num;
		particles.resize(num);
	}
	unsigned int getParticleNumber() { return particles.size(); }


protected:
	void UpdateInstanceData();
	Vector3 GetRandomDirection(std::mt19937& mt);
	std::vector<InstanceAtributes> GetAttributeData() const;
	Mesh* particleMesh;
	Vector4 particleColour;
	UniqueOGLTexture* texture;
	Vector3 position;

	GLuint instanceVBO;
	std::vector<Particle> particles;
	unsigned int maxParticles;
	int aliveCount;
	Shader* shader;
	float emitTimer;
	float emitRate;

};