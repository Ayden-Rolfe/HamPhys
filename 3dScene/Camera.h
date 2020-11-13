#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

class Camera
{
public:
	virtual void Update(float deltaTime) = 0;
	void SetPerspective(float fov, float aspectRatio, float near, float far);
	void SetLookAt(glm::vec3 pos, glm::vec3 euler);
	void SetPosition(glm::vec3 position);
	void AddPosition(glm::vec3 position);
	glm::vec3 GetPosition();
	glm::mat4 GetWorldTransform() { return m_worldTransform; }
	glm::mat4 GetView() { return m_viewTransform; }
	glm::mat4 GetProjection() { return m_projectionTransform; }
	glm::mat4 GetProjectionView() { return m_projectionViewTransform; }

protected:
	void UpdateProjectionViewTransform();

	glm::fquat m_rotQuat;

	glm::mat4 m_worldTransform;
	glm::mat4 m_viewTransform = glm::mat4(1.0f);
	glm::mat4 m_projectionTransform;
	glm::mat4 m_projectionViewTransform;
};

