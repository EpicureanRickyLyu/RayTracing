#pragma once

#include "Walnut/Image.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"
#include <memory>

#include <glm/glm.hpp>
class Renderer
{
public:
	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);
	
	void Render(const Scene& scene, const Camera& camera);

	inline std::shared_ptr<Walnut::Image> GetFinalImage() { return m_Image; };

private:
	glm::vec4 TraceRay(const Scene& scene, const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> m_Image;
	
	uint32_t* m_ImageData = nullptr;
};