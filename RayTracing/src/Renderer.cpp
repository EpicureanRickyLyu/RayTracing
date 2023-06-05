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
	Ray ray;

 	ray.origin = camera.GetPosition();


	//fill data buffer 
	for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
		{
			//glm::vec2 coord = { (float)x / (float)m_Image->GetWidth(), (float)y / (float)m_Image->GetHeight() };//calculate uv map to (0,1)
			//coord = coord * 2.0f - 1.0f; //map from (0,1) to (-1,1)
			//coord.x *= (float)m_Image->GetWidth() / (float)m_Image->GetHeight();//fix x radio between x and y, to make ellipse into sphere
			//coord -= 0.5f;//map to (-0.5, 0.5)

			ray.dir = camera.GetRayDirections()[x + y * m_Image->GetWidth()];

			glm::vec4 color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_Image->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	//send to gpu to render
	m_Image->SetData(m_ImageData);
}


//implicit surface -> parametric linear equation
//fragment shader -> pixel
glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	using namespace glm;
	vec3 bkcolor = mix(vec3(0.8f, 1.0f, 1.0f), vec3(0.5f, 0.7f, 1.0f), 0.5f);//interpolate between two color
	if (scene.Spheres.empty())
	{
		return glm::vec4(0, 0, 0, 1);
	}
	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	for (const Sphere& sphere : scene.Spheres)
	{
		const Sphere& sphere = scene.Spheres[0];

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

		// background,Output to screen// blue sky dome
		if (discrimant < 0.0f)
			continue;
	
		float closePoint = ( - b - sqrt(discrimant) ) / (2.0f * a);
		//float t0 = ( - b + sqrt(discrimant) ) / (2.0f * a);
		//t1 is closest point, hitpoint
		if (closePoint < hitDistance)
		{
			hitDistance = closePoint;
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
	{
		return glm::vec4(bkcolor, 1.0f);
	}


	glm::vec3 origin = ray.origin - closestSphere->Position;
	glm::vec3 rayOri(ray.origin);
	glm::vec3 rayDir(ray.dir);
	normalize(rayDir);

	//calculate line sphere intersections
	float radius = closestSphere->Radius;
	float a = glm::dot(rayDir, rayDir);//quadratic equation : rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z  * rayDir.z;
	float b = 2.0f * glm::dot(origin, rayDir);
	float c = glm::dot(origin, origin) - radius * radius;
	float discrimant = b * b - 4.0f * a * c;// -b += sqrt(b^2 - 4ac) / 2a
	float contourtorlerance = 0.1f;


	//second hit point ? further hitpoint
	float t0 = ( - b + sqrt(discrimant) ) / (2.0f * a);
	// background,Output to screen// blue sky dome
	if (discrimant < 0.0f)
	{
		return glm::vec4(bkcolor, 1.0f);
	}
	
	//record hitpoint 
	vec3 h1 = origin + rayDir * t0;
	vec3 hitpoint = origin + rayDir * hitDistance;
	vec3 normal = normalize(hitpoint);
	vec3 lightDir = normalize(vec3( - 1, -1, -1 ));

	float angle = glm::max(dot(normal, -lightDir), 0.0f);//equal to the degree between light and face normal
	
	//outline, when quadratic dicscrimant == 0, mean only one intersction
	if (discrimant == 0.0f || discrimant < contourtorlerance)
	{
		return glm::vec4(0.3f, 0.3f, 0.3f, 0.3f);
	}
	else
	{
		//if angle > 0, means facing the light, < 0 means they are back face, the asign dark light
		return glm::vec4(closestSphere->Albedo * angle, 1.0f);//why normal brighter?
	}
	

}
