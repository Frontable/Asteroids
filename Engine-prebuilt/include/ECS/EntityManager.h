#pragma once
#include <queue>
#include <array>
#include "types.h"
#include "Logger/Logger.h"

namespace FrostEngine
{
    class EntityManager
    {
    public:
        
        EntityManager() = default;        

        Entity CreateEntity()
        {
            Entity entity;
            if (m_currentEntity >= MAX_ENTITIES && m_availableEntities.empty())
            {
                FROST_ERROR("Maximum entities reached!");
            }

            if (!m_availableEntities.empty())
            {
                entity = m_availableEntities.front();
                m_availableEntities.pop();
            }
            else
            {
                entity = m_currentEntity++;
            }

            m_aliveEntities[entity] = true;
            m_entities[entity] = Signature();
            return entity;
        }

        void DeleteEntity(Entity _entity)
        {
            if (_entity >= MAX_ENTITIES)
            {
                FROST_ERROR("Tried to delete invalid entity");
            }

            if (m_aliveEntities[_entity])
            {
                m_aliveEntities[_entity] = false;
                m_entities[_entity] = Signature();
                m_availableEntities.push(_entity);
                FROST_LOG("Deleted Entity with ID: %d", _entity);
            }
        }

        void SetSignature(Entity _entity, const Signature &signature)
        {
            if (_entity < MAX_ENTITIES && m_aliveEntities[_entity])
            {
                m_entities[_entity] = signature;
                return;
            }

            FROST_ERROR("Tried to set Signature to an invalid entity");
        }

        Signature GetSignature(Entity _entity) const
        {
            if (_entity < MAX_ENTITIES && m_aliveEntities[_entity])
            {
                return m_entities[_entity];
            }

            FROST_ERROR("Tried to get a Signature from an invalid entity");
        }

        bool IsAlive(Entity _entity) const
        {
            return _entity < MAX_ENTITIES && m_aliveEntities[_entity];
        }        

    private:
        Entity m_currentEntity{};
        std::array<Signature, MAX_ENTITIES> m_entities{};
        std::array<bool, MAX_ENTITIES> m_aliveEntities{};
        std::queue<Entity> m_availableEntities{};
    };
}