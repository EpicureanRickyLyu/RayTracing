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

void Renderer::Render()
{
	//fill data buffer 
	for (uint32_t y = 0; y < m_Image->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_Image->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_Image->GetWidth(), (float)y / (float)m_Image->GetHeight() };//calculate uv map to (0,1)
			coord = coord * 2.0f - 1.0f; //map from (0,1) to (-1,1)
			coord.x *= (float)m_Image->GetWidth() / (float)m_Image->GetHeight();//fix x radio between x and y, to make ellipse into sphere
			//coord -= 0.5f;//map to (-0.5, 0.5)

			glm::vec4 color = PerPixel(coord);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_Image->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	//send to gpu to render
	m_Image->SetData(m_ImageData);
}


//implicit surface -> parametric linear equation
//fragment shader -> pixel
glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	using namespace glm;

	glm::vec3 rayOri(0.0f, 0.0f, 1.0f);
	glm::vec3 rayDir(coord.x, coord.y, -1.0f);
	normalize(rayDir);

	//calculate line sphere intersections
	float radius = 0.5f;
	float a = glm::dot(rayDir, rayDir);//quadratic equation : rayDir.x * rayDir.x + rayDir.y * rayDir.y + rayDir.z  * rayDir.z;
	float b = 2.0f * glm::dot(rayOri, rayDir);
	float c = glm::dot(rayOri, rayOri) - radius * radius;
	float discrimant = b * b - 4.0f * a * c;// -b += sqrt(b^2 - 4ac) / 2a
	float contourtorlerance = 0.1f;

	// background,Output to screen
	if (discrimant < 0.0f)
	{
		float t = coord.y; // map y to [0, 1]
		vec3 color = mix(vec3(0.8f, 1.0f, 1.0f), vec3(0.5f, 0.7f, 1.0f), t);//interpolate between two color
		return glm::vec4(color, 1.0f);
	}
		
	//t1 is closest point, hitpoint
	float t0 = ( - b + sqrt(discrimant) ) / (2.0f * a);
	float closePoint = ( - b - sqrt(discrimant) ) / (2.0f * a);
	
	//record hitpoint 
	vec3 h1 = rayOri + rayDir * t0;
	vec3 hitpoint = rayOri + rayDir * closePoint;
	vec3 normal = normalize(hitpoint);

	vec3 lightDir = normalize(vec3( - 1, -1, -1 ));
	float angle = glm::max(dot(normal, -lightDir), 0.0f);//equal to the degree between light and face normal
	
	//if angle > 0, means facing the light, < 0 means they are back face, the asign dark light


	//outline, when quadratic dicscrimant == 0, mean only one intersction
	if (discrimant == 0.0f || discrimant < contourtorlerance)
	{
		return glm::vec4(0.3f, 0.3f, 0.3f, 0.3f);
	}
	else
	{
		vec3 color = normal * 0.5f + 0.5f;// (-1.0f * 0.5f + 0.5f ) > 0 -> map all value to (0,1)
		return glm::vec4(color * angle, 1.0f);//why normal brighter?
	}
	

}
