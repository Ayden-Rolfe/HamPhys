#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter() :
	m_particles(nullptr),
	m_firstDead(0),
	m_maxParticles(0),
	m_position(0, 0, 0),
	m_vao(0), m_vbo(0), m_ibo(0),
	m_vertexData(nullptr),
	m_matrix(1.0f)
{

}

ParticleEmitter::~ParticleEmitter()
{
	delete[] m_particles;
	delete[] m_vertexData;

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_ibo);
}

void ParticleEmitter::Init(uint32_t a_maxParticles, uint32_t a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startCol, const glm::vec4& a_endCol)
{
	// Emitter setup
	m_emitTimer = 0;
	VarInit(1.0f / a_emitRate, a_lifetimeMin, a_lifetimeMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startCol, a_endCol);
	m_maxParticles = a_maxParticles;

	// Particle array
	m_particles = new Particle[m_maxParticles];
	m_firstDead = 0;

	// Create array of vertices for particles
	// 4 verts per particle for quad
	// filled during update
	m_vertexData = new ParticleVertex[m_maxParticles * 4];
	// Create index buffer data for particles
	// 6 indices per quad of 2 tris
	// fill as it never changes
	uint32_t* indexData = new uint32_t[m_maxParticles * 6];
	for (uint32_t i = 0; i < m_maxParticles; ++i)
	{
		indexData[i * 6 + 0] = i * 4 + 0;
		indexData[i * 6 + 1] = i * 4 + 1;
		indexData[i * 6 + 2] = i * 4 + 2;

		indexData[i * 6 + 3] = i * 4 + 0;
		indexData[i * 6 + 4] = i * 4 + 2;
		indexData[i * 6 + 5] = i * 4 + 3;
	}

	// create opengl buffers
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glGenBuffers(1, &m_ibo);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 4 * sizeof(ParticleVertex), m_vertexData, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_maxParticles * 6 * sizeof(uint32_t), indexData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // col
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), ((char*)0) + 16);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] indexData;
}

void ParticleEmitter::VarInit(float a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startCol, const glm::vec4& a_endCol)
{
	m_emitRate = a_emitRate;
	m_startColour = a_startCol;
	m_endColour = a_endCol;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_lifespanMin = a_lifetimeMin;
	m_lifespanMax = a_lifetimeMax;
}

void ParticleEmitter::VarInit(const ParticleEmitterDummy& dummy)
{
	VarInit(dummy.m_emitRate, dummy.m_lifespanMin, dummy.m_lifespanMax, dummy.m_velocityMin, dummy.m_velocityMax, dummy.m_startSize, dummy.m_endSize, dummy.m_startColour, dummy.m_endColour);
}

void ParticleEmitter::Emit()
{
	// only emit if there is a dead particle to use
	if (m_firstDead >= m_maxParticles)
		return;

	// resurrect the first dead particle
	Particle& particle = m_particles[m_firstDead++];

	// assign starting pos
	particle.position = m_position;

	// randomise lifespan
	particle.lifetime = 0;
	particle.lifespan = hamh::fRandRange(m_lifespanMin, m_lifespanMax);

	// set start size & col
	particle.colour = m_startColour;
	particle.size = m_startSize;

	float vel = hamh::fRandRange(m_velocityMin, m_velocityMax);
	particle.velocity.x = hamh::fRand() * 2.0f - 1.0f;
	particle.velocity.y = hamh::fRand() * 2.0f - 1.0f;
	particle.velocity.z = hamh::fRand() * 2.0f - 1.0f;
	particle.velocity = glm::normalize(particle.velocity) * vel;
}

void ParticleEmitter::Update(float a_deltaTime, const glm::mat4& a_camTransform)
{
	// spawn particles
	m_emitTimer += a_deltaTime;
	while (m_emitTimer > m_emitRate)
	{
		Emit();
		m_emitTimer -= m_emitRate;
	}

	uint32_t quad = 0;

	// update particles and turn live particles into billboard quads
	for (uint32_t i = 0; i < m_firstDead; i++)
	{
		Particle* particle = &m_particles[i];

		particle->lifetime += a_deltaTime;
		if (particle->lifetime >= particle->lifespan)
		{
			// swap last alive with this one
			*particle = m_particles[m_firstDead - 1];
			m_firstDead--;
		}
		else
		{
			// Update particle vars
			particle->position += particle->velocity * a_deltaTime;
			particle->size = glm::mix(m_startSize, m_endSize, particle->lifetime / particle->lifespan);
			particle->colour = glm::mix(m_startColour, m_endColour, particle->lifetime / particle->lifespan);

			// Make quad correct size/col
			float halfSize = particle->size * 0.5f;

			m_vertexData[quad * 4 + 0].position = glm::vec4(halfSize, halfSize, 0, 1);
			m_vertexData[quad * 4 + 0].colour = particle->colour;
			m_vertexData[quad * 4 + 1].position = glm::vec4(-halfSize, halfSize, 0, 1);
			m_vertexData[quad * 4 + 1].colour = particle->colour;
			m_vertexData[quad * 4 + 2].position = glm::vec4(-halfSize, -halfSize, 0, 1);
			m_vertexData[quad * 4 + 2].colour = particle->colour;
			m_vertexData[quad * 4 + 3].position = glm::vec4(halfSize, -halfSize, 0, 1);
			m_vertexData[quad * 4 + 3].colour = particle->colour;
			
			// Create billboard transform
			glm::vec3 zAxis = glm::normalize(glm::vec3(a_camTransform[3]) - particle->position - glm::vec3(m_matrix[3]));
			glm::vec3 xAxis = glm::cross(glm::vec3(a_camTransform[1]), zAxis);
			glm::vec3 yAxis = glm::cross(zAxis, xAxis);
			glm::mat4 billboard(glm::vec4(xAxis, 0),
								glm::vec4(yAxis, 0),
								glm::vec4(zAxis, 0),
								glm::vec4(0, 0, 0, 1));

			m_vertexData[quad * 4 + 0].position = billboard * m_vertexData[quad * 4 + 0].position + glm::vec4(particle->position, 0);
			m_vertexData[quad * 4 + 1].position = billboard * m_vertexData[quad * 4 + 1].position + glm::vec4(particle->position, 0);
			m_vertexData[quad * 4 + 2].position = billboard * m_vertexData[quad * 4 + 2].position + glm::vec4(particle->position, 0);
			m_vertexData[quad * 4 + 3].position = billboard * m_vertexData[quad * 4 + 3].position + glm::vec4(particle->position, 0);

			++quad;
		}
	}
}

void ParticleEmitter::Draw()
{
	// sync the particle vertex buffer
	// based on how many alive particles there are
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_firstDead * 4 * sizeof(ParticleVertex), m_vertexData);

	// Draw particles
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_firstDead * 6, GL_UNSIGNED_INT, 0);
}

void ParticleEmitter::SetPosition(glm::vec3 position, bool moveParticles)
{
	// By default, particles move with the origin of the emitter
	// Otherwise, particles will retain their position in world space
	if (!moveParticles)
	{
		// Calculate difference in positions
		glm::vec3 deltaMove = position - glm::vec3(m_matrix[3]);
		TranslateParticles(deltaMove);
	}
	m_matrix[3][0] = position.x;
	m_matrix[3][1] = position.y;
	m_matrix[3][2] = position.z;
}

void ParticleEmitter::AddPosition(glm::vec3 position, bool moveParticles)
{
	// By default, particles move with the origin of the emitter
	// Otherwise, particles will retain their position in world space
	if (!moveParticles)
		TranslateParticles(position);
	m_matrix[3][0] += position.x;
	m_matrix[3][1] += position.y;
	m_matrix[3][2] += position.z;
}

// subtracts the position all living particles by the given translation
// specifically made for add/set position
void ParticleEmitter::TranslateParticles(glm::vec3 translation)
{
	for (uint32_t i = 0; i < m_firstDead; ++i)
	{
		// Subtract change in movement to retain old particle position
		Particle* particle = &m_particles[i];
		particle->position -= translation;
	}
}

ParticleEmitter* ParticleEmitterDummy::InitPass(uint32_t a_maxParticles, uint32_t a_emitRate, float a_lifetimeMin, float a_lifetimeMax, float a_velocityMin, float a_velocityMax, float a_startSize, float a_endSize, const glm::vec4& a_startCol, const glm::vec4& a_endCol)
{
	m_emitRate = 1.0f / a_emitRate;
	m_startColour = a_startCol;
	m_endColour = a_endCol;
	m_startSize = a_startSize;
	m_endSize = a_endSize;
	m_velocityMin = a_velocityMin;
	m_velocityMax = a_velocityMax;
	m_lifespanMin = a_lifetimeMin;
	m_lifespanMax = a_lifetimeMax;

	ParticleEmitter* emitter = new ParticleEmitter;
	emitter->Init(a_maxParticles, a_emitRate, a_lifetimeMin, a_lifetimeMax, a_velocityMin, a_velocityMax, a_startSize, a_endSize, a_startCol, a_endCol);
	return emitter;
}
