#include "PowerUp.h"

PowerUp::PowerUp(std::string type, glm::vec3 color, float duration, glm::vec2 position, Texture texture)
	: GameObject(position, glm::vec2(60.0f, 20.0f), texture, color, glm::vec2(0.0f, 150.0f))
	, Type(type), Duration(duration), Activated(false)
{

}

PowerUp::~PowerUp()
{
}
