#include "_3dSceneApp.h"
#include "Gizmos.h"
#include <imgui.h>
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <iostream>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

_3dSceneApp::_3dSceneApp() 
{

}

_3dSceneApp::~_3dSceneApp() 
{
	
}

bool _3dSceneApp::startup() 
{
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

#pragma region Camera Setup
	m_camera.SetLookAt(vec3(10, 10, 10), vec3(-30, 45, 0));
	// m_camera.SetPosition(glm::vec3(10, 5, 0));
	m_camera.SetPerspective(glm::pi<float>() * 0.25f, 16.0f / 9.0f, 0.1f, 1000.0f);
#pragma endregion

#pragma region Basic Model Setup
	m_texlitShader.loadShader(aie::eShaderStage::VERTEX, "./shaders/texlit.vert");
	m_texlitShader.loadShader(aie::eShaderStage::FRAGMENT, "./shaders/texlit.frag");

	if (m_texlitShader.link() == false)
	{
		printf("Lit Texture Shader error: %s\n", m_texlitShader.getLastError());
	}

	m_basiclitShader.loadShader(aie::eShaderStage::VERTEX, "./shaders/basiclit.vert");
	m_basiclitShader.loadShader(aie::eShaderStage::FRAGMENT, "./shaders/basiclit.frag");

	if (m_basiclitShader.link() == false)
	{
		printf("Basic Lit shader error: %s\n", m_basiclitShader.getLastError());
	}

	static const glm::mat4 bigMdl =
	{
		2.f, 0, 0, 0,
		0, 2.f, 0, 0,
		0, 0, 2.f, 0,
		0, 0, 0, 1
	};

	static const glm::mat4 smallMdl =
	{
		0.1f, 0, 0, 0,
		0, 0.1f, 0, 0,
		0, 0, 0.1f, 0,
		0, 0, 0, 1
	};

	m_stanford = new Model("./stanford/Dragon.obj", glm::mat4(1.0f), true, true);
	m_stanford->SetPosition(glm::vec3(-10, 0, 0));

	auto spear = new Model("./soulspear/soulspear.obj", glm::mat4(1.0f), true, true);
	spear->SetPosition(glm::vec3(0, 2, 0));
	m_mdlList.push_back(spear);

	 auto newMdl = new Model("./apricot/Apricot_02_hi_poly.obj", glm::mat4(1.0f), true, true);
	 newMdl->SetPosition(glm::vec3(10, 0, 0));
	 m_mdlList.push_back(newMdl);

#pragma endregion

#pragma region PostProcessing
	if (m_renderTarget.initialise(1, getWindowWidth(), getWindowHeight(), true) == false)
	{
		printf("Render target error!\n");
		return false;
	}

	m_postQuad.InitFullscreenQuad();

	m_postprocessShader.loadShader(aie::eShaderStage::VERTEX, "./shaders/post.vert");
	m_postprocessShader.loadShader(aie::eShaderStage::FRAGMENT, "./shaders/post.frag");

	if (m_postprocessShader.link() == false)
	{
		printf("PostProcess Shader error: %s\n", m_postprocessShader.getLastError());
	}

	// Assign map values
	for (int i = 0; i < postProcessEffectSize; ++i)
	{
		// Get index for effect & store with its name in the map
		m_effectManager.subroutineMap.insert(std::pair<const char*, GLuint>(postProcessEffects[i], glGetSubroutineIndex(m_postprocessShader.getHandle(), GL_FRAGMENT_SHADER, postProcessEffects[i])));
	}
#pragma endregion

#pragma region Lighting Setup
	m_sun = new Sun();
	m_sun->diffuse = { 1, 1, 1 };
	m_sun->specular = { 1, 1, 0 };
	m_sun->ambient = { 0.25f, 0.25f, 0.25f };
	m_lightList.push_back(m_sun);

	m_pointLight = new PointLight();
	m_pointLight->position = vec3(-17.5f, 5.0f, 0.0f);
	m_pointLight->constant = 1.0f;
	m_pointLight->linear = 0.09f;
	m_pointLight->quadratic = 0.032f;
	m_pointLight->diffuse = { 1, 1, 1 };
	m_pointLight->specular = { 1, 1, 1 };

#pragma endregion

#pragma region Particle System
	m_particleShader.loadShader(aie::eShaderStage::VERTEX, "./shaders/particle.vert");
	m_particleShader.loadShader(aie::eShaderStage::FRAGMENT, "./shaders/particle.frag");

	if (m_particleShader.link() == false)
	{
		printf("Shader error: %s\n", m_particleShader.getLastError());
	}

	m_imguiEmitter = ParticleEmitterDummy();
	m_emitter = m_imguiEmitter.InitPass(1000, 500,
										0.25f, 2.0f,
										1, 5,
										1, 0.f,
										glm::vec4(1, 0, 0, 1), glm::vec4(1, 1, 0, 1));
#pragma endregion

	return true;
}

void _3dSceneApp::shutdown() 
{
	for (size_t i = 0; i < m_mdlList.size(); i++)
		delete m_mdlList[i];
	for (size_t i = 0; i < m_lightList.size(); i++)
		delete m_lightList[i];
	Gizmos::destroy();
	delete m_emitter;
}

void _3dSceneApp::update(float deltaTime) 
{
	static glm::vec3 emitterPos = vec3(0.0f);

#pragma region UI
	ImGui::Begin("Options");
	ImGui::ColorEdit3("Sun Colour", glm::value_ptr(m_sun->diffuse));
	if (ImGui::TreeNode("Particle Emitter"))
	{
		ImGui::Checkbox("Static Particles", &m_staticParticles);
		ImGui::Checkbox("Attach to box", &m_emitterFollowBox);
		ImGui::DragFloat3("Position", glm::value_ptr(emitterPos), 0.25f);
		ImGui::DragFloatRange2("Lifetime Range", &m_imguiEmitter.m_lifespanMin, &m_imguiEmitter.m_lifespanMax, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloatRange2("Velocity Range", &m_imguiEmitter.m_velocityMin, &m_imguiEmitter.m_velocityMax, 0.1f, 0.1f, 10.0f);
		ImGui::DragFloat("Start size", &m_imguiEmitter.m_startSize, 0.1f, 0.0f, FLT_MAX);
		ImGui::DragFloat("End size", &m_imguiEmitter.m_endSize, 0.1f, 0.0f, FLT_MAX);
		ImGui::ColorEdit3("Start Colour", glm::value_ptr(m_imguiEmitter.m_startColour));
		ImGui::ColorEdit3("End Colour", glm::value_ptr(m_imguiEmitter.m_endColour));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Point Light"))
	{
		ImGui::DragFloat3("Position", glm::value_ptr(m_pointLight->position), 0.25f);
		ImGui::Checkbox("Rainbowify", &m_rainbowLight);
		ImGui::DragFloat("Rainbowify speed", &m_rainbowLightSpeed, 0.1f, 0.0f, FLT_MAX);
		ImGui::ColorEdit3("Colour", glm::value_ptr(m_pointLight->diffuse));
		ImGui::DragFloat("Constant", &m_pointLight->constant, 0.05f);
		ImGui::DragFloat("Linear", &m_pointLight->linear, 0.05f);
		ImGui::DragFloat("Quadratic", &m_pointLight->quadratic, 0.05f);
		ImGui::TreePop();
	}
	ImGui::Combo("Post Process Effects", &m_effectManager.selection, postProcessEffects, postProcessEffectSize);
	ImGui::End();
#pragma endregion

#pragma region UI data fill
	m_emitter->VarInit(m_imguiEmitter);
	if (!m_emitterFollowBox)
		m_emitter->SetPosition(emitterPos, m_staticParticles);

	if (m_rainbowLight)
		m_pointLight->diffuse = glm::rgbColor(glm::vec3((fmodf((getTime() * m_rainbowLightSpeed), 1.0f) * 360.f), 1, 1));
#pragma endregion

	// wipe the gizmos clean for this frame
	Gizmos::clear();

	m_camera.Update(deltaTime);

#pragma region Gizmos

	// draw a simple grid with gizmos
	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	for (int i = 0; i < 21; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, 10),
			vec3(-10 + i, 0, -10),
			i == 10 ? white : black);
		Gizmos::addLine(vec3(10, 0, -10 + i),
			vec3(-10, 0, -10 + i),
			i == 10 ? white : black);
	}

	// Draw light dir
	Gizmos::addLine(glm::vec3(0), m_sun->direction, glm::vec4(1, 1, 0, 1));

	const static int sunScalar = 10;
	const static int sunDistScalar = 10 * sunScalar;
	// Draw "sun"
	Gizmos::addSphere(-(m_sun->direction * sunDistScalar), sunScalar, 10, 10, glm::vec4(m_sun->diffuse, 1));

	// Draw pointlight
	Gizmos::addSphere(m_pointLight->position, 0.1f, 5, 5, glm::vec4(m_pointLight->diffuse, 1));

#pragma endregion

	// add a transform so that we can see the axis
	// Gizmos::addTransform(glm::scale(mat4(1), glm::vec3(10, 10, 10)));

	aie::Input* input = aie::Input::getInstance();

	DrawQuatBox();

	// Query time since application started
	float time = getTime();

	// Rotate sunlight
	m_sun->direction = glm::normalize(vec3(glm::cos(time * 0.5f), glm::sin(time * 0.5f), 0));
	glm::mat4 lightProj = glm::ortho<float>(-10, 10, -10, 10, -10, 10);
	glm::mat4 lightView = glm::lookAt(m_sun->direction, glm::vec3(0), glm::vec3(0, 1, 0));
	// m_sun->matrix = lightProj * lightView;

	// Rotate model
	glm::mat4 rotMatrix = glm::eulerAngleY(glm::sin(time * 0.5f));
	rotMatrix[3] = glm::vec4(m_stanford->GetPosition(), 1.0f);
	m_stanford->matrix = rotMatrix;

	// Update particle system
	m_emitter->Update(deltaTime, m_camera.GetView());

	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();

	if (input->isKeyDown(aie::INPUT_KEY_R))
		m_camera.SetLookAt(vec3(10, 10, 10), vec3(-30, 45, 0));
}

void _3dSceneApp::DrawQuatBox()
{
	// Draw cube that moves between 2 defined points in space
	static const glm::vec3 positions[2] =
	{
		glm::vec3(10, 5, 10),
		glm::vec3(-10, 0, 10)
	};
	static const glm::fquat rotations[2] =
	{
		glm::fquat(glm::vec3(0, -1, 0)),
		glm::fquat(glm::vec3(0, 1, 0))
	};

	// Time value in range [0..1]
	float s = glm::cos(getTime()) * 0.5f + 0.5f;

	// LERP
	glm::vec3 p = (1.0f - s) * positions[0] + s * positions[1];

	// SLERP
	glm::quat r = glm::slerp(rotations[0], rotations[1], s);

	// Matrix
	glm::mat4 m = glm::translate(p) * glm::toMat4(r);

	// Draw transform/box
	Gizmos::addTransform(m);
	Gizmos::addAABB(glm::vec3(), glm::vec3(.5f), glm::vec4(1, 0, 0, 1), &m);

	// Set particle emitter position to new pos
	if (m_emitterFollowBox)
		m_emitter->SetPosition(p, m_staticParticles);
}

void _3dSceneApp::draw() 
{
	// Bind render target
	m_renderTarget.bind();

	// wipe the screen to the background colour
	clearScreen();

#pragma region Particles
	// Particle shader
	m_particleShader.bind();

	auto particlePVM = m_camera.GetProjectionView() * m_emitter->GetMatrix();
	m_particleShader.bindUniform("projectionView", particlePVM);

	m_emitter->Draw();
#pragma endregion

	Gizmos::draw(m_camera.GetProjectionView());

#pragma region Generic/Lit
	// Bind shader
	m_texlitShader.bind();

	m_texlitShader.bindUniform("cameraPosition", m_camera.GetPosition());

	// Bind sun
	m_texlitShader.bindUniform("directLight.direction", m_sun->direction);
	m_texlitShader.bindUniform("directLight.ambient", m_sun->ambient);
	m_texlitShader.bindUniform("directLight.diffuse", m_sun->diffuse);
	m_texlitShader.bindUniform("directLight.specular", m_sun->diffuse);

	// bind pointlight
	m_texlitShader.bindUniform("pointLight.position", m_pointLight->position);
	m_texlitShader.bindUniform("pointLight.diffuse", m_pointLight->diffuse);
	m_texlitShader.bindUniform("pointLight.specular", m_pointLight->diffuse);
	m_texlitShader.bindUniform("pointLight.constant", m_pointLight->constant);
	m_texlitShader.bindUniform("pointLight.linear", m_pointLight->linear);
	m_texlitShader.bindUniform("pointLight.quadratic", m_pointLight->quadratic);

	// Draw textured lit models
	for (uint32_t i = 0; i < m_mdlList.size(); i++)
	{
		// Bind transform
		auto mdlPVM = m_camera.GetProjectionView() * m_mdlList[i]->matrix;
		m_texlitShader.bindUniform("ProjectionViewModel", mdlPVM);

		// Bind lighting transforms
		m_texlitShader.bindUniform("NormalMatrix", glm::inverseTranspose(glm::mat3(m_mdlList[i]->matrix)));

		// Bind model matrix
		m_texlitShader.bindUniform("ModelMatrix", m_mdlList[i]->matrix);

		// Draw
		m_mdlList[i]->mesh.draw();
	}

	// Draw basic models
	m_basiclitShader.bind();

	m_basiclitShader.bindUniform("cameraPosition", m_camera.GetPosition());

	// Bind sun
	m_basiclitShader.bindUniform("directLight.direction", m_sun->direction);
	m_basiclitShader.bindUniform("directLight.ambient", m_sun->ambient);
	m_basiclitShader.bindUniform("directLight.diffuse", m_sun->diffuse);
	m_basiclitShader.bindUniform("directLight.specular", m_sun->diffuse);

	// bind pointlight
	m_basiclitShader.bindUniform("pointLight.position", m_pointLight->position);
	m_basiclitShader.bindUniform("pointLight.diffuse", m_pointLight->diffuse);
	m_basiclitShader.bindUniform("pointLight.specular", m_pointLight->diffuse);
	m_basiclitShader.bindUniform("pointLight.constant", m_pointLight->constant);
	m_basiclitShader.bindUniform("pointLight.linear", m_pointLight->linear);
	m_basiclitShader.bindUniform("pointLight.quadratic", m_pointLight->quadratic);

	auto basicPVM = m_camera.GetProjectionView() * m_stanford->matrix;
	m_basiclitShader.bindUniform("ProjectionViewModel", basicPVM);
	m_basiclitShader.bindUniform("NormalMatrix", glm::inverseTranspose(glm::mat3(m_stanford->matrix)));
	m_basiclitShader.bindUniform("ModelMatrix", m_stanford->matrix);

	m_stanford->mesh.draw();

	
#pragma endregion

	// Unbind target to return to backbuffer
	m_renderTarget.unbind();

	clearScreen();

	m_postprocessShader.bind();

	// Bind post-processing subroutine to specified type
	// Get index from map
	auto it = m_effectManager.subroutineMap.find(postProcessEffects[m_effectManager.selection]);
	GLuint index;
	if (it != m_effectManager.subroutineMap.end())
		index = it->second;
	else
	{
		std::cout << "ERR: index not found; using default";
		index = glGetSubroutineIndex(m_postprocessShader.getHandle(), GL_FRAGMENT_SHADER, "Default");
	}

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

	// Render screen-quad
	m_postprocessShader.bindUniform("colourTarget", 0);
	m_renderTarget.bindDepthTarget(1);
	m_postprocessShader.bindUniform("depthTarget", 1);
	m_renderTarget.getTarget(0).bind(0);

	m_postQuad.Draw();
}