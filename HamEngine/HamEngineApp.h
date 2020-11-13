#pragma once

#include "Application.h"
#include "Renderer2D.h"

#include "PhysScene.h"

#include "Barrier.h"

// Handy constants relating to window size
constexpr static int WINDOW_WIDTH = 1280;
constexpr static int WINDOW_HEIGHT = 720;
constexpr static int WINDOW_WH = WINDOW_WIDTH / 2;
constexpr static int WINDOW_HH = WINDOW_HEIGHT / 2;

// The distance between the top of the screen & the ball.
constexpr static int CAM_SPACING = 150;
// Height at which the score text is fully opaque.
constexpr static float CAM_SCOREOPACITYMAXHEIGHT = 1000;
// Maximum size of the score text at the top of the screen
constexpr static uint16_t CAM_MAXSCORETEXTSIZE = 30;

// Fixed-time delay between object spawn attempts
constexpr static float OBS_SPAWNDELAY = 0.2f;
// Spacing that determines the spawn region for obstacles
constexpr static float OBS_MINSPAWNSPACING = 100.0f;
constexpr static float OBS_MAXSPAWNSPACING = 300.0f;
constexpr static float OBS_AVGSPAWNSPACING = (OBS_MINSPAWNSPACING + OBS_MAXSPAWNSPACING) / 2.0f;
// Height at which the spawn chance maxes out
constexpr static float OBS_MAXSPAWNHEIGHT = 20000.0f;
// Spawn chance once the max spawn height has been reached
constexpr static float OBS_MAXSPAWNCHANCE = 0.65f;

class HamEngineApp : public aie::Application 
{
public:
	HamEngineApp();
	virtual ~HamEngineApp();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

protected:
	aie::Renderer2D*	m_2dRenderer = nullptr;
	aie::Font*			m_font = nullptr;

	PhysScene*			m_physScene = nullptr;

	Sphere*				m_ball = nullptr;
	Polygon*			m_wallLeft = nullptr;
	Polygon*			m_wallRight = nullptr;
	Barrier*			m_barrier = nullptr;

	bool				m_gameStart = false;
	bool				m_gameOver = false;

	// Mouse drawing functionality
	bool				m_mouseDown = false;

	vec2				m_barrierStart = vec2();
	vec2				m_barrierEnd = vec2();

	// Camera shifting functionality
	float				m_camUpperBound = WINDOW_HH - CAM_SPACING;
	float				m_camLowerBound = -WINDOW_HH + CAM_SPACING;
	float				m_camHeight = 0;

	float				m_camLowerDestroyBound = -WINDOW_HH - CAM_SPACING;
};