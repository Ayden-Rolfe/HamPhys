#include "FlyCamera.h"

void FlyCamera::Update(float deltaTime)
{
	aie::Input* input = aie::Input::getInstance();
	if (input->wasKeyPressed(aie::EInputCodes::INPUT_KEY_Z))
	{
		m_stateChange = true;
		m_camLock = !m_camLock;
	}

	if (m_stateChange)
	{
		m_stateChange = false;
		//if (!m_camLock)
		//	input->SetCursorState(aie::ECursorState::normal);
		//else
		//	input->SetCursorState(aie::ECursorState::locked);
	}

	glm::mat4 viewMat = GetView();
	glm::vec3 forward(viewMat[0][2], viewMat[1][2], viewMat[2][2]);
	

	if (m_camLock)
	{
		glm::vec3 desired(0, 0, 0);
		// Translations
		if (input->isKeyDown(aie::INPUT_KEY_SPACE))
			desired.y += 5.0f;
		if (input->isKeyDown(aie::INPUT_KEY_LEFT_CONTROL))
			desired.y -= 5.0f;
		if (input->isKeyDown(aie::INPUT_KEY_W))
			desired.z -= 5.0f;
		if (input->isKeyDown(aie::INPUT_KEY_S))
			desired.z += 5.0f;
		if (input->isKeyDown(aie::INPUT_KEY_A))
			desired.x -= 5.0f;
		if (input->isKeyDown(aie::INPUT_KEY_D))
			desired.x += 5.0f;

		float desiredRot = 0.0f;
		if (input->isKeyDown(aie::INPUT_KEY_Q))
			desiredRot += m_rotSpeedRoll;
		if (input->isKeyDown(aie::INPUT_KEY_E))
			desiredRot -= m_rotSpeedRoll;

		const glm::fquat forward = glm::toQuat(m_viewTransform);
		desired = forward * desired;

		AddPosition(desired * deltaTime);

		// Rotation
		int mDeltaX;
		int mDeltaY;
		input->getMouseDelta(&mDeltaX, &mDeltaY);
		glm::vec3 angles(mDeltaY * m_rotSpeedMouse * deltaTime, -mDeltaX * m_rotSpeedMouse * deltaTime, desiredRot * deltaTime);
		glm::fquat rot(angles);
		m_viewTransform *= glm::toMat4(rot);
	}

	UpdateProjectionViewTransform();
}