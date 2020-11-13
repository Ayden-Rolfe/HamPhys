#include "HamEngineApp.h"
#include "Texture.h"
#include "Font.h"
#include "Input.h"

HamEngineApp::HamEngineApp() 
{

}

HamEngineApp::~HamEngineApp() 
{

}

bool HamEngineApp::startup() 
{
#ifdef DEBUG
	srand(0);
#else
	srand(time(NULL));
#endif // DEBUG

	m_2dRenderer = new aie::Renderer2D();

	// TODO: remember to change this when redistributing a build!
	// the following path would be used instead: "./font/consolas.ttf"
	m_font = new aie::Font("./font/consolas.ttf", 32);

	// Physics update time step
	float step = 0.01f;

	m_physScene = new PhysScene(step, vec2(0, -100));

	const int SPACING_W = 150;
	const int SPACING_H = 150;
	const int SPACING_RAND = 20;

	const float wallDepth = 100;
	const float wallHeight = WINDOW_HEIGHT;
	const float tunnelWallWidth = 50;
	const Material wallMat(0.f, 0.8f);
	const Material obstMat(0.f, 0.9f);
	const Material ballMat(1.2f, 0.7f);
	const Colour wallCol(1, 1, 0);

	// Outer walls
	m_wallRight = static_cast<Polygon*>(m_physScene->AddBody(new Polygon(wallDepth, wallHeight, vec2(WINDOW_WIDTH + wallDepth - 1, wallHeight / 2.f), wallMat, wallCol, vec2(), 0.0f)));
	m_wallLeft = static_cast<Polygon*>(m_physScene->AddBody(new Polygon(wallDepth, wallHeight, vec2(-wallDepth + 1, wallHeight / 2.f), wallMat, wallCol, vec2(), 0.0f)));

	// Bug where falling perfectly downwards causes some math messiness, todo: fix that
	m_ball = static_cast<Sphere*>(m_physScene->AddBody(new Sphere(20, vec2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2), Material(1.2f, 0.7f), Colour(1, 0, 0, 1)/*, vec2(0.01f, 0)*/)));
	
	// Starter platform, erased after player makes their own
	m_barrier = static_cast<Barrier*>(m_physScene->AddBody(new Barrier(m_ball->GetPosition() + vec2(-100, -25), m_ball->GetPosition() + vec2(100, -25), 0.0f, Colour(1, 0, 0))));
	return true;
}

void HamEngineApp::shutdown() 
{
	delete m_font;
	delete m_2dRenderer;
	delete m_physScene;
}

void HamEngineApp::update(float deltaTime) 
{
	// input example
	aie::Input* input = aie::Input::getInstance();

	if (!m_gameOver)
	{
		// Store mouse & camera values for later use
		int mouseX, mouseY;
		input->getMouseXY(&mouseX, &mouseY);

		float camX, camY;
		m_2dRenderer->getCameraPos(camX, camY);

		mouseY += camY;

		m_physScene->Update(deltaTime);

		vec2 ballPos = m_ball->GetPosition();
		// Convert world space to camera space
		vec2 conv = ballPos - vec2(WINDOW_WH, WINDOW_HH);

		float ballPosUpperDifference = conv.y - m_camUpperBound;

		// Alter camera, wall, and text positions as the ball rises
		if (ballPosUpperDifference > 0.0f)
		{
			camY += ballPosUpperDifference;
			m_camUpperBound += ballPosUpperDifference;
			m_camLowerBound += ballPosUpperDifference;
			m_camLowerDestroyBound += ballPosUpperDifference;
			m_camHeight += ballPosUpperDifference;
			m_wallLeft->AddPosition(vec2(0, ballPosUpperDifference));
			m_wallRight->AddPosition(vec2(0, ballPosUpperDifference));
			m_2dRenderer->setCameraPos(camX, camY);
			m_barrierStart.y += ballPosUpperDifference;
		}

		if (input->isMouseButtonDown(aie::INPUT_MOUSE_BUTTON_LEFT))
		{
			// Make mouse position end of barrier
			if (m_mouseDown)
			{
				m_barrierEnd.x = mouseX;
				m_barrierEnd.y = mouseY;
			}
			// Make initial click location start of barrier
			else
			{
				m_mouseDown = true;
				m_barrierStart = vec2(mouseX, mouseY);
				m_barrierEnd = m_barrierStart;
			}
		}
		// Mouse released, commit to barrier creation
		else if (m_mouseDown)
		{
			// Remove old barrier from simulation
			m_physScene->RemoveBody(m_barrier);
			// Add this as the new barrier
			m_barrier = static_cast<Barrier*>(m_physScene->AddBody(new Barrier(m_barrierStart, m_barrierEnd, 4.0f, Colour(0, 1, 0))));
			m_barrierStart = vec2();
			m_barrierEnd = vec2();
			m_mouseDown = false;
			m_gameStart = true;
		}

		if (m_barrier && m_barrier->hit && m_gameStart)
		{
			m_physScene->RemoveBody(m_barrier);
		}

		static float obstacleTimeAcc = 0.0f;


		static Material obsMat = Material(0.f, 0.95f);
		static Colour obsCol = Colour(1.f, 0.95f, 0.f);

		obstacleTimeAcc += deltaTime;
		while (obstacleTimeAcc >= OBS_SPAWNDELAY)
		{
			// Create obstacles
			float heightRatio = m_camHeight / OBS_MAXSPAWNHEIGHT;
			heightRatio = (heightRatio < 1.0f) ? heightRatio : 1.0f;
			float spawnChance = OBS_MAXSPAWNCHANCE * heightRatio;
			if (spawnChance > hamh::fRand())
			{
				vec2 randPos = vec2(hamh::RandRange(0, WINDOW_WIDTH), hamh::RandRange(OBS_MINSPAWNSPACING, OBS_MAXSPAWNSPACING) + m_camHeight + WINDOW_HEIGHT);
				// Choose between circle or box randomly, & generate random values for them
				Rigidbody* randBody = nullptr;
				if (hamh::fRand() < 0.5f)
				{
					randBody = new Sphere(hamh::RandRange(10, 50), randPos, obsMat, obsCol);
				}
				else
				{
					randBody = new Polygon(hamh::RandRange(10, 50), hamh::RandRange(10, 50), randPos, obsMat, obsCol, vec2(), hamh::Degrees2Radians(hamh::fRand() * 360.f));
				}

				m_physScene->AddBody(randBody);
			}
			obstacleTimeAcc -= OBS_SPAWNDELAY;
		}

		// Clean obstacles below visible area
		static std::vector<Rigidbody*> remBodyList = std::vector<Rigidbody*>(m_physScene->GetBodyCount());
		// If clear list size is smaller than body list, increase it to fit atleast all bodies in scene
		if (m_physScene->GetBodyCount() > remBodyList.size())
			remBodyList.resize(m_physScene->GetBodyCount());
		remBodyList.clear();
		// Iterate bodies, checking if they're below the deletion threshold
		for (size_t i = 0; i < m_physScene->GetBodyCount(); ++i)
		{
			Rigidbody* rb = m_physScene->GetBody(i);
			// If body is below camera by over OBS_SPAWNSPACING, mark for deletion
			// Also, don't delete either the ball or barrier
			if (rb->GetPosition().y <= m_camHeight - OBS_AVGSPAWNSPACING && !m_ball && !m_barrier)
				remBodyList.push_back(rb);
		}

		// Ball failure condition check
		if (m_ball->GetPosition().y < m_camHeight - OBS_AVGSPAWNSPACING)
		{
			m_gameOver = true;
		}

		// Clear bodies marked for deletion
		for (size_t i = 0; i < remBodyList.size(); i++)
			m_physScene->RemoveBody(remBodyList[i]);
	}
	

	// exit the application
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}

void HamEngineApp::draw() 
{

	// wipe the screen to the background colour
	clearScreen();

	// begin drawing sprites
	m_2dRenderer->begin();

	// draw your stuff here!
	m_physScene->Draw(m_2dRenderer);
	
	// Output intro text while game hasn't started
	// Or score when >100 & game has started
	static constexpr char* t_goal = "Keep the ball going up towards the sky!";
	static constexpr char* t_init = "Draw a platform under the ball with the mouse to begin";
	// Score text displayed at top of screen with score beneath
	static constexpr char* t_scrt = "SCORE:";
	// Game over text
	static constexpr char* t_fail = "Game over...";

	// Textbox spacing around letters
	static constexpr float goTextSpacing = 10.0f;
	static constexpr vec2 goSpacingVector = vec2(goTextSpacing, goTextSpacing);

	static const float t_goalWidth = m_font->getStringWidth(t_goal);
	static const float t_initWidth = m_font->getStringWidth(t_init);
	static const float t_scrtWidth = m_font->getStringWidth(t_scrt);
	static const float t_failWidth = m_font->getStringWidth(t_fail);

	if (!m_gameStart)
	{
		m_2dRenderer->setRenderColour(0xFFFFFFFF);
		// Draw initial tutorial text
		m_2dRenderer->drawText(m_font, t_goal, WINDOW_WH - (t_goalWidth / 2), WINDOW_HEIGHT - 32);
		m_2dRenderer->drawText(m_font, t_init, WINDOW_WH - (t_initWidth / 2), WINDOW_HEIGHT - 86);
	}
	else
	{
		// If gameovered, draw that
		if (m_gameOver)
		{
			static vec2 goBoxBL;
			static vec2 goBoxTR;

			static vec2 goBoxExtents;
			static vec2 goBoxPosition = vec2(WINDOW_WH, WINDOW_HH + m_camHeight);
			// First draw in gameover state, get info
			if (m_ball)
			{
				// Get tightwrapped box around gameover text
				m_font->getStringRectangle(t_fail, goBoxBL.x, goBoxBL.y, goBoxTR.x, goBoxTR.y);
				// Add spacing
				goBoxBL -= goSpacingVector;
				goBoxTR += goSpacingVector;
				// Convert to extents for draw function
				goBoxExtents.x = (goBoxTR.x - goBoxBL.x);
				goBoxExtents.y = (goBoxTR.y - goBoxBL.y);

				goBoxPosition.y += 10;
				// remove and nullptr the ball
				m_physScene->RemoveBody(m_ball);
				m_ball = nullptr;
			}
			m_2dRenderer->setRenderColour(1, 1, 1, 0.5f);
			m_2dRenderer->drawBox(goBoxPosition.x, goBoxPosition.y, goBoxExtents.x, goBoxExtents.y);
			m_2dRenderer->setRenderColour(0xFFFFFFFF);
			m_2dRenderer->drawText(m_font, t_fail, WINDOW_WH - (t_failWidth / 2), WINDOW_HH + m_camHeight);
		}
		// Draws text with transparency being increased as height does to a maximum
		float textVis = (m_camHeight / CAM_SCOREOPACITYMAXHEIGHT);
		textVis = textVis < 1.0f ? textVis : 1.0f;
		char t_buffer[20];
		sprintf_s(t_buffer, "%.f", m_camHeight);
		float t_scoreWidth = m_font->getStringWidth(t_buffer);
		m_2dRenderer->setRenderColour(1.0f, 1.0f, 1.0f, textVis);
		m_2dRenderer->drawText(m_font, t_scrt, WINDOW_WH - (t_scrtWidth / 2), (WINDOW_HEIGHT - 32) + m_camHeight);
		m_2dRenderer->drawText(m_font, t_buffer, WINDOW_WH - (t_scoreWidth / 2), (WINDOW_HEIGHT - 86) + m_camHeight);
	}

	m_2dRenderer->setRenderColour(0xFF0000FF);
	m_2dRenderer->drawLine(m_barrierStart.x, m_barrierStart.y, m_barrierEnd.x, m_barrierEnd.y);

	// Debug draws below

	// done drawing sprites
	m_2dRenderer->end();
}