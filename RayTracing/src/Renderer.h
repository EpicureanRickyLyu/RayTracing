#pragma once

#include "Walnut/Image.h"

#include <memory>

#include <glm/glm.hpp>
class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	
	void Render();

	inline std::shared_ptr<Walnut::Image> GetFinalImage() { return m_Image; };

private:
	glm::vec4 PerPixel(glm::vec2 coord);

private:
	std::shared_ptr<Walnut::Image> m_Image;
	
	uint32_t* m_ImageData = nullptr;
};