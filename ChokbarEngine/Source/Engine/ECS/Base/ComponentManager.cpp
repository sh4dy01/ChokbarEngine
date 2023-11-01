#include "Chokbar.h"
#include "ComponentManager.h"

namespace Chokbar {
	ComponentManager::ComponentManager()
		: m_NextComponentType(0)
	{
		m_ComponentArrays = {};
		m_ComponentTypes = {};
	}

	ComponentManager::~ComponentManager()
	{
		for (auto const& pair : m_ComponentArrays)
		{
			delete pair.second;
		}
	}

	void ComponentManager::EntityDestroyed(InstanceID entity)
	{
		for (auto const& pair : m_ComponentArrays)
		{
			auto const& component = pair.second;

			component->EntityDestroyed(entity);
		}
	}
}

