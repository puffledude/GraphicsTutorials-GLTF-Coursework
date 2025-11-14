#include "Emitter.h"

Emitter::Emitter(Vector4 colour, unsigned int amount, UniqueOGLTexture* texture)
{
	particleColour = colour;
	particles.resize(amount);
	emitTimer = 0.0f;
	emitRate = 0.01f; // Emit a particle every 0.01 seconds
	texture = texture;
}
Emitter::~Emitter()
{
	delete texture;
}

