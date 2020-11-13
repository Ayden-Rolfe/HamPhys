#include "Camera.h"

void Camera::UpdateProjectionViewTransform()
{
	m_worldTransform = glm::inverse(m_viewTransform);
	m_projectionViewTransform = m_projectionTransform * m_worldTransform;
}

void Camera::SetPerspective(float fov, float aspectRatio, float near, float far)
{
	m_projectionTransform = glm::perspective(fov, aspectRatio, near, far);
}

void Camera::SetLookAt(glm::vec3 pos, glm::vec3 euler)
{
	euler = glm::radians(euler);
	m_viewTransform = glm::mat4(1.0f);
	SetPosition(pos);
	glm::fquat angles(euler);
	m_viewTransform *= glm::toMat4(angles);
	//m_viewTransform = glm::lookAt(from, to, up);
	//m_viewTransform[3].x = from.x;
	//m_viewTransform[3].y = from.y;
	//m_viewTransform[3].z = from.z;
}

void Camera::SetPosition(glm::vec3 position)
{
	m_viewTransform[3][0] = position.x;
	m_viewTransform[3][1] = position.y;
	m_viewTransform[3][2] = position.z;
}

void Camera::AddPosition(glm::vec3 position)
{
	m_viewTransform[3][0] += position.x;
	m_viewTransform[3][1] += position.y;
	m_viewTransform[3][2] += position.z;
}

glm::vec3 Camera::GetPosition()
{
	return glm::vec3(m_viewTransform[3][0], m_viewTransform[3][1], m_viewTransform[3][2]);
}
