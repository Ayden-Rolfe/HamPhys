#pragma once

#define GLM_GTX_EXPERIMENTAL

#include "Camera.h"
#include <Input.h>
#include <glm/gtx/quaternion.hpp>

class FlyCamera : public Camera
{
public:
	virtual void Update(float deltaTime);
	void SetSpeed(float speed) { m_speed = speed; }
	void SetRotSpeedMouse(float speed) { m_rotSpeedMouse = speed; }
	void SetRotSpeedRoll(float speed) { m_rotSpeedRoll = speed; }

private:
	float m_speed = 3.0f;
	float m_rotSpeedMouse = 0.5f;
	float m_rotSpeedRoll = 1.0f;
	glm::vec3 m_up;

	bool m_camLock = false;
	bool m_stateChange = false;
};