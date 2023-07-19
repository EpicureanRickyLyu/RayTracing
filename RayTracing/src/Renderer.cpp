#include "Renderer.h"
#include "Walnut/Random.h"

namespace Utils
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_Image)
	{
		if (m_Image->GetWidth() == width || m_Image->GetHeight() == height)
			return;
		
		m_Image->Resize(width, height);
	}
	else
	{
		m_Image = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_pActiveScene = &scene;
	m_pActiveCamera = &camera;

	//fill data buffer 
	for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_Image->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	//send to gpu to render
	m_Image->SetData(m_ImageData);
}



Renderer::HitPayLoad Renderer::TraceRay( const Ray& ray)
{
	//GetClosest HitDistance
	int closestSphereID = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (int i = 0; i < m_pActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_pActiveScene->Spheres[i];
		glm::vec3 origin = ray.origin - sphere.Position;
		glm::vec3 rayOri(ray.origin);
		glm::vec3 rayDir(ray.dir);
		normalize(rayDir);

		//calculate line sphere intersections
		float radius = sphere.Radius;
		float a = glm::dot(rayDir, rayDir);//quadratic equation : rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z  * rayDir.z;
		float b = 2.0f * glm::dot(origin, rayDir);
		float c = glm::dot(origin, origin) - radius * radius;
		float discrimant = b * b - 4.0f * a * c;// -b += sqrt(b^2 - 4ac) / 2a
		float contourtorlerance = 0.1f;

		// background,Output to screen< 0 no intersections, = 0 1 intersection,> 0 2 intersection
		if (discrimant < 0.0f)
			continue;
	
		float closePoint = ( - b + sqrt(discrimant) ) / (2.0f * a);
		//float t0 = ( - b + sqrt(discrimant) ) / (2.0f * a);
		//t1 is closest point, hitpoint
		if (closePoint > 0.0f && closePoint < hitDistance)
		{
			hitDistance = closePoint;
			closestSphereID = i;
		}
	}

	if (closestSphereID < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closestSphereID);
	
}
glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	using namespace glm;

	vec3 skyColor = mix(vec3(0.8f, 1.0f, 1.0f), vec3(0.5f, 0.7f, 1.0f), 0.5f);//interpolate between two color
	Ray ray;
	ray.origin = m_pActiveCamera->GetPosition();
	ray.dir = m_pActiveCamera->GetRayDirections()[x + y * m_Image->GetWidth()];

	glm::vec3 color(0.0f);
	int bounces = 2;
	float attenuation = 1.0f;
	//for (int i = 0; i < bounces; i++)
	//{
	HitPayLoad payload  = TraceRay(ray);	
	//If ray missed, No light then add sky color
	if (payload.HitDistance < 0.0f)
	{
		color += vec3(0.0f,0.0f,0.0f) * attenuation;//bk color
		//break;
		return vec4(color, 1.0f);
	}
			
	const Sphere& closestSphere = m_pActiveScene->Spheres[payload.ObjectIndex];
	vec3 lightDir = normalize(m_pActiveScene->m_LightDirection);

	float angle = glm::max(dot(payload.WorldNormal, -lightDir), 0.0f);//equal to the degree between light and face normal
	glm::vec3 sphereColor = closestSphere.Albedo;

	sphereColor *= angle;
	color += attenuation * sphereColor;

	attenuation *= 0.7f;

		//when bounce change ray origin and move forward
		//ray.origin = payload.WorldPosition + 0.0001f * payload.WorldNormal;
		//ray.dir = glm::reflect(ray.dir, payload.WorldNormal);
	//}

	return vec4(color, 1.0f);
}

Renderer::HitPayLoad Renderer::ClosestHit(const Ray& ray, float hitDistance, uint32_t objectIndex)
{
	Renderer::HitPayLoad payLoad;

	payLoad.HitDistance = hitDistance;
	payLoad.ObjectIndex = objectIndex;

	const Sphere& closestSphere = m_pActiveScene->Spheres[objectIndex];
	glm::vec3 origin = ray.origin - closestSphere.Position;

	payLoad.WorldPosition = origin + ray.dir * hitDistance;
	payLoad.WorldNormal = normalize(payLoad.WorldPosition);

	payLoad.WorldPosition += closestSphere.Position;

	return payLoad;
}

Renderer::HitPayLoad Renderer::Miss(const Ray& ray)
{
	HitPayLoad payload;
	payload.HitDistance = -1;
	return payload;
}
