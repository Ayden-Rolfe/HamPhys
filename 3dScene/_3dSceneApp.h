#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Mesh.h"
#include "Application.h"
#include "FlyCamera.h"
#include <glm/mat4x4.hpp>
#include <glm/common.hpp>
#include "OBJMesh.h"
#include "tiny_obj_loader.h"
#include "Shader.h"
#include "RenderTarget.h"
#include "ParticleEmitter.h"
#include <unordered_map>

struct OBJVertex
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};

struct GLInfo
{
	uint32_t vao;
	uint32_t vbo1;
	uint32_t vbo2;
	uint32_t faceCount;
};

struct Model
{
	Model() {};
	Model(const char* aOBJPath, glm::mat4 aMatrix, bool loadTex = true, bool flipY = false) 
	{ 
		if (!Load(aOBJPath, loadTex, flipY)) 
		{
			printf("ERROR: Failed to load %s", aOBJPath);
		};
		matrix = aMatrix;
	}

	bool Load(const char* mdlPath, bool loadTex = true, bool flipY = false)
	{
		return mesh.load(mdlPath, loadTex, flipY);
	}
	void SetPosition(glm::vec3 position)
	{
		matrix[3][0] = position.x;
		matrix[3][1] = position.y;
		matrix[3][2] = position.z;
	}
	glm::vec3 GetPosition()
	{
		return glm::vec3(matrix[3]);
	}

	aie::Texture		tex;
	aie::OBJMesh		mesh;
	glm::mat4			matrix;
};

struct Light
{
	glm::vec3 diffuse;
	glm::vec3 specular;
};

struct Sun : public Light
{
	glm::vec3 direction;

	glm::vec3 ambient;
};

struct PointLight : public Light
{
	glm::vec3 position;
	
	float constant;
	float linear;
	float quadratic;
};

struct ShadowMap
{
	uint32_t m_fbo = 0;
	uint32_t m_fboDepth = 0;
};

// List of post-processing effects in post.frag
static const char* postProcessEffects[]
{
	"Default",
	"BoxBlur",
	"Distort",
	"GreyScale",
	"Sepia",
	"Scanline",
	"HorrorFog",
	"HeightTest",
	"DepthTest"
};
static constexpr int postProcessEffectSize = sizeof(postProcessEffects) / sizeof(char*);

// Post Processing Effect Manager
struct ppeManager
{
	// Map of each subroutine GLUint to each effect type
	std::unordered_map<const char*, GLuint> subroutineMap;

	// Current selection
	int selection = 0;
};

class _3dSceneApp : public aie::Application 
{
public:

	_3dSceneApp();
	virtual ~_3dSceneApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	void DrawQuatBox();

protected:
	FlyCamera			m_camera;
	
	// Shader programs & postprocess objects
	aie::ShaderProgram	m_texlitShader;
	aie::ShaderProgram	m_basiclitShader;
	aie::ShaderProgram	m_postprocessShader;
	aie::RenderTarget	m_renderTarget;
	Mesh				m_postQuad;
	ppeManager			m_effectManager;

	// Model & lighting objects
	std::vector<Model*>	m_mdlList;
	std::vector<Light*>	m_lightList;
	Sun*				m_sun;
	PointLight*			m_pointLight;
	bool				m_rainbowLight = false;
	float				m_rainbowLightSpeed = 0.1f;
	Model*				m_stanford;

	// Animated mesh objects
	std::vector<GLInfo>	m_meshes;
	aie::ShaderProgram	m_animVertexShader;

	// Particle shader & objects
	aie::ShaderProgram	m_particleShader;
	ParticleEmitter*	m_emitter;
	bool				m_emitterFollowBox = false;
	bool				m_staticParticles = false;

	// Debugging dummy, allows easy modification of actual particle emitter
	ParticleEmitterDummy		m_imguiEmitter;
};	