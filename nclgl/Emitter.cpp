#include "Emitter.h"
#include "particle.h"
#include <random>

/// <summary>
/// Make an emitter that emits paticles up to a set emount
/// </summary>
/// <param name="colour">Colour of the particles</param>
/// <param name="amount">Maximum amount of partices</param>
/// <param name="texture">Texture of the particles</param>
Emitter::Emitter(Vector3 position, unsigned int amount, Vector4 colour, Mesh shape)
{
	this->position = position;
	particleColour = colour;
	texture = texture;
	maxParticles = amount;
	emitTimer = 0.0f;
	emitRate = 0.01f; // Emit a particle every 0.01 seconds
	texture = texture;
	particleMesh = shape;
	for (int i = 0; i < particles.size(); ++i) {
		particles[i].colour = particleColour;
		particles[i].velocity = 0.0f; // Default velocity
		particles[i].direction = Vector3(0.0f, 1.0f, 0.0f); // Default upward direction
		particles[i].life = 0.0f; // Initialize all particles as dead
	}
	//Next steps here is to generate the instance attributes and the vbo and pass them through to the mesh object.
	std::vector<InstanceAtributes> attributeData = GetAttributeData();
	GLuint instanceVBO;
	glGenBuffers(1, &instanceVBO);
	particleMesh.AddInstanceBuffer(instanceVBO, attributeData);
}

void Emitter::Update(float dt) {
	for (size_t i = 0; i < aliveCount; i++) {  // For all alive particles
		Particle& p = particles[i];  //Get a particle
		p.life -= dt;  //Remove life

		if (p.life <= 0.0f) {  //if dead
			//Swap dead particle with last alive particle
			particles[i] = particles[aliveCount - 1];
			i--;  //Go back one index
			continue;
		}
		p.position += p.direction * p.velocity * dt;
		p.velocity *= 0.98f * dt; //Slow down over time
	}
	updateAtributeData();

}

void Emitter::Emit(unsigned int count) {
	std::mt19937 mt(420);
	std::uniform_real_distribution<float> velDist(0.0, 1.0);
	std::uniform_real_distribution<float> lifeDist(4, 8);
	for (int i = 0; i < count; i++) {
		if (aliveCount >= maxParticles) {
			return; // No more room for new particles
		}

		Particle& p = particles[aliveCount++];
		p.position = position; // Start at emitter position
		p.direction = Vector3(0, 1, 0);
		p.velocity = velDist(mt);
		p.life = lifeDist(mt);

	}

}


std::vector<InstanceAtributes> Emitter::GetAttributeData()const {
	std::vector<InstanceAtributes> attribs = {
	{3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, pos)},
	{4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, colour)},
	{5, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, size)}
	};
	return attribs;
}

void Emitter::updateAtributeData() {
	//Need to either make this return or inject straight into the mesh object.
	//Then all I need is to do drawing maybe and then shader writing.
}