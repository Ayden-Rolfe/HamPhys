#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/ext.hpp>
#include <gl_core_4_4.h>

#include "Helpers.h"

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 colour;

	float size;
	float lifetime;
	float lifespan;
};

struct ParticleVertex
{
	glm::vec4 position;
	glm::vec4 colour;
};

class ParticleEmitter;

// Dummy container for IMGUI editing
struct ParticleEmitterDummy
{
	// Parses variables to itself, then returns a pointer to a real particle emitter with the same values
	ParticleEmitter* InitPass(uint32_t a_maxParticles, uint32_t a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startCol, const glm::vec4& a_endCol);

	glm::vec3		m_position;

	float			m_emitTimer;
	float			m_emitRate;

	float			m_lifespanMin;
	float			m_lifespanMax;

	float			m_velocityMin;
	float			m_velocityMax;

	float			m_startSize;
	float			m_endSize;

	glm::vec4		m_startColour;
	glm::vec4		m_endColour;
};

class ParticleEmitter
{
public:
	ParticleEmitter();
	virtual ~ParticleEmitter();

	void Init(uint32_t a_maxParticles, uint32_t a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startCol, const glm::vec4& a_endCol);
	void VarInit(float a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startCol, const glm::vec4& a_endCol);
	void VarInit(const ParticleEmitterDummy& dummy);

	void Emit();

	void Update(float a_deltaTime, const glm::mat4& a_camTransform);
	void Draw();

	void SetPosition(glm::vec3 position, bool moveParticles = true);
	void AddPosition(glm::vec3 position, bool moveParticles = true);
	const glm::mat4& GetMatrix() { return m_matrix; }

	void SetEmitRate(float a_emitDelay) { m_emitRate = 1.0f / a_emitDelay; }
	void SetLifespanMin(float a_min) { m_lifespanMin = a_min; }
	void SetLifespanMax(float a_max) { m_lifespanMax = a_max; }
	void SetVelocityMin(float a_min) { m_velocityMin = a_min; }
	void SetVelocityMax(float a_max) { m_velocityMax = a_max; }
	void SetStartSize(float a_size) { m_startSize = a_size; }
	void SetEndSize(float a_size) { m_endSize = a_size; }
	void SetStartColour(glm::vec4 col) { m_startColour = col; }
	void SetEndColour(glm::vec4 col) { m_endColour = col; }

protected:

	// Particle details
	Particle*		m_particles;
	uint32_t		m_firstDead;
	uint32_t		m_maxParticles;

	uint32_t		m_vao, m_vbo, m_ibo;
	ParticleVertex* m_vertexData;

	glm::vec3		m_position;

	float			m_emitTimer;
	float			m_emitRate;

	float			m_lifespanMin;
	float			m_lifespanMax;
	
	float			m_velocityMin;
	float			m_velocityMax;

	float			m_startSize;
	float			m_endSize;

	glm::vec4		m_startColour;
	glm::vec4		m_endColour;

	void TranslateParticles(glm::vec3 translation);

	// Emitter details
	glm::mat4		m_matrix;
};

