#include "Emitter.h"
#include "particle.h"
#include <algorithm>
#include <random>

/// <summary>
/// Make an emitter that emits paticles up to a set emount
/// </summary>
/// <param name="colour">Colour of the particles</param>
/// <param name="amount">Maximum amount of partices</param>
/// <param name="texture">Texture of the particles</param>
Emitter::Emitter(Vector3 position, unsigned int amount, Vector4 colour, Mesh* shape, Shader* shaderProgram)
{
	this->position = position;
	particleColour = colour;
	//texture = texture;
	shader = shaderProgram;
	maxParticles = amount;
	emitTimer = 0.0f;
	emitRate = 100.0f; // Emit 100 particles per second
	//texture = texture;
	if (!shape) {
		shape = Mesh::GenerateQuad();
	}
	particleMesh = shape;
	particles.reserve(amount);
	for (int i = 0; i < amount; ++i) {
		Particle p;
		p.colour = particleColour;
		p.velocity = 0.0f; // Default velocity
		p.direction = Vector3(0.0f, 1.0f, 0.0f); // Default upward direction
		p.life = 0.0f; // Initialize all particles as dead
		particles.push_back(p);
	}
	aliveCount = 0;
	//Next steps here is to generate the instance attributes and the vbo and pass them through to the mesh object.
	std::vector<InstanceAtributes> attributeData = GetAttributeData();
	glGenBuffers(1, &instanceVBO);
	particleMesh->AddInstanceBuffer(instanceVBO, attributeData);
}

void Emitter::Update(float dt) {

	std::mt19937 mt((std::random_device())());
	std::uniform_real_distribution<float> velDist(0.0f, 3.0f);
	std::uniform_real_distribution<float> lifeDist(0.1f, 0.5f);
	emitTimer += dt;
	int toEmit = (int)floor(emitRate * emitTimer);
	if (toEmit > 0) {
		// subtract the emitted fraction of time
		emitTimer -= (float)toEmit / emitRate;
	}
	for (int i = 0; i < toEmit; i++) { //Refresh dead particles here
		if (aliveCount >= maxParticles) {
			break; // No more room for new particles
		}

		Particle& p = particles[aliveCount++];
		p.position = position; // Start at emitter position
		p.direction = Vector3(0, 1, 0);
		p.velocity = velDist(mt);
		p.life = lifeDist(mt);
		p.startLife = p.life;
		p.size = 0.4f;

	}
	for (size_t i = 0; i < aliveCount; i++) {  // For all alive particles
		Particle& p = particles[i];  //Get a particle
		p.life -= dt;  //Remove life

		if (p.life <= 0.0f) {  //if dead
			//Swap dead particle with last alive particle
			particles[i] = particles[aliveCount - 1];

			aliveCount--;        // <-- IMPORTANT: decrement aliveCount
			i--;  //Go back one index
			continue;
		}
		p.position += p.direction * p.velocity * dt;
		p.velocity *= 0.98f; //Slow down over time
	}

	
	UpdateInstanceData();
}

void Emitter::Emit() {
	//glUseProgram(shader->GetProgram());
	particleMesh->DrawInstances(aliveCount);

}


std::vector<InstanceAtributes> Emitter::GetAttributeData()const {
	std::vector<InstanceAtributes> attribs = {
	{1, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, pos)},
	{2, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, size)},
	{3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, colour)}
	};
	return attribs;
}

void Emitter::UpdateInstanceData() {
	//Need to either make this return or inject straight into the mesh object.
	//Then all I need is to do drawing maybe and then shader writing.

	std::vector<InstanceData> instanceData;
	for (int i = 0; i < aliveCount; ++i) {
		InstanceData data;
		data.pos = particles[i].position;
		float ratio = 0.0f;
		if (particles[i].startLife > 0.0f) {
			ratio = std::clamp(particles[i].life / particles[i].startLife, 0.0f, 1.0f);
		}
		// pick grey level (you can tweak this)
		const Vector4 grey(0.5f, 0.5f, 0.5f, particles[i].colour.w);

		// lerp: result = original * ratio + grey * (1 - ratio)
		data.colour = Vector4(
			particles[i].colour.x * ratio + grey.x * (1.0f - ratio),
			particles[i].colour.y * ratio + grey.y * (1.0f - ratio),
			particles[i].colour.z * ratio + grey.z * (1.0f - ratio),
			particles[i].colour.w // keep alpha from the original colour (or lerp if desired)
		);
		particles[i].size = 0.4f * ratio; // Particles shrink over their lifetime
		data.size = particles[i].size; // Set a default size, could be modified based on particle properties
		instanceData.push_back(data);
	}
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
