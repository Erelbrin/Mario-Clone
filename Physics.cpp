#include "Physics.hpp"
#include "Components.hpp"
#include <cstdlib>

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	Vec2& posA = a->getComponent<CTransform>().pos;
	Vec2& posB = b->getComponent<CTransform>().pos;
	Vec2 delta(abs(posA.x - posB.x), abs(posA.y - posB.y));

	return Vec2(a->getComponent<CBoundingBox>().halfSize.x + b->getComponent<CBoundingBox>().halfSize.x - delta.x,
		a->getComponent<CBoundingBox>().halfSize.y + b->getComponent<CBoundingBox>().halfSize.y - delta.y);
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b)
{
	Vec2& posA = a->getComponent<CTransform>().pos;
	Vec2& posB = b->getComponent<CTransform>().prevPos;
	Vec2 delta(abs(posA.x - posB.x), abs(posA.y - posB.y));

	return Vec2(a->getComponent<CBoundingBox>().halfSize.x + b->getComponent<CBoundingBox>().halfSize.x - delta.x,
		a->getComponent<CBoundingBox>().halfSize.y + b->getComponent<CBoundingBox>().halfSize.y - delta.y);
}