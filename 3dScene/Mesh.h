#pragma once
#include <gl_core_4_4.h>
#include <glm/glm.hpp>
#include <Texture.h>

class Mesh
{
public:
	Mesh() : triCount(0), vao(0), vbo(0), ibo(0) {}
	virtual ~Mesh();

	struct Vertex
	{
		glm::vec4 position;
		glm::vec4 normal;
		glm::vec2 texCoord;
	};

	void InitQuad(glm::mat4 matrix);
	void InitFullscreenQuad();
	glm::mat4 GetMatrix() { return mat; }
	aie::Texture& GetTexture() { return tex; }
	
	virtual void Draw();

protected:
	unsigned int triCount;
	unsigned int vao, vbo, ibo;
	glm::mat4 mat;
	aie::Texture tex;
};