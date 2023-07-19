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
	struct HitPayLoad  
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		uint32_t ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y); // RayGen

	HitPayLoad TraceRay(const Ray& ray);

	HitPayLoad ClosestHit(const Ray& ray,float hitDistance, uint32_t objectIndex);
	
	HitPayLoad Miss(const Ray& ray);
private:
	std::shared_ptr<Walnut::Image> m_Image;
	
	uint32_t* m_ImageData{ nullptr };

	const Scene* m_pActiveScene = nullptr;
	const Camera* m_pActiveCamera= nullptr;
};