#include "EntityManager.hpp"
#include <iostream>

EntityManager::EntityManager()
{

}

void EntityManager::update()
{
	// add all the entities that are pending
	for (auto& e : m_entitiesToAdd)
	{
		// add it to the vector of all entities
		m_entities.push_back(e);

		// add it to the entity map in the correct place
		// map[key] will create an element at "key" if it does not already exist
		//			therefore we are not in danger of adding to a vector that doesn't exist
		m_entityMap[e->tag()].push_back(e);
	}

	// clear the temporary vector since we have added everything
	m_entitiesToAdd.clear();

	// remove dead entities from the vector of all entities
	removeDeadEntities(m_entities);

	// remove dead entities from each vector in the entity map
	// C++17 way of iterating through [key, value] pairs in a map
	for (auto& [tag, entityVec] : m_entityMap)
	{
		removeDeadEntities(entityVec);
	}
}

void EntityManager::removeDeadEntities(EntityVec& vec)
{
	// remove all dead entities from the input vector
	// this is called by the update() function

	const auto newEnd = std::remove_if(vec.begin(), vec.end(),
		[](const std::shared_ptr<Entity>& i)
		{
			return i->isActive() == false;
		}
	);

	vec.erase(newEnd, vec.end());
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

	m_entitiesToAdd.push_back(entity);

	return entity;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}