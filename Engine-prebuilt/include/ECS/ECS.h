#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
#include "Components.h"
#include "Logger/Logger.h"
#include "Rendering/Core/SpriteBatchRenderer.h"
#include <utility>

namespace FrostEngine
{
    class ECS
    {
    public:
        ECS() = default;
        ECS(const ECS&) = delete;
        ECS& operator=(const ECS&) = delete;
        ~ECS() = default;

        //---------------------
        // Entity
        //---------------------
        Entity CreateEntity()
        {
            return m_entityManager.CreateEntity();
        }


        void DeleteEntity(Entity _entity)
        {
            m_entityManager.DeleteEntity(_entity);
            m_componentManager.EntityDestroyed(_entity);
            m_systemManager.EntityDestroyed(_entity);
        }

        //---------------------
        // Components
        //---------------------

        template<typename T>
        void RegisterComponent()
        {
            m_componentManager.RegisterComponent<T>();
        }
        
        template <typename T>
        void AddComponent(Entity _entity, T _component)
        {
            m_componentManager.AddComponent<T>(_entity, _component);

            ComponentID id = m_componentManager.GetComponentID<T>();
            Signature signature = m_entityManager.GetSignature(_entity);
            signature.set(id);
            m_entityManager.SetSignature(_entity, signature);

            m_systemManager.EntitySignatureChange(_entity, signature);
            // FROST_LOG("aM I GETTING ERE");
        }      

        template <typename T>
        void RemoveComponent(Entity _entity)
        {
            m_componentManager.RemoveComponent<T>(_entity);

            // ← these two lines are probably missing
            ComponentID id = m_componentManager.GetComponentID<T>();
            Signature signature = m_entityManager.GetSignature(_entity);
            signature.reset(id);  // clear the bit
            m_entityManager.SetSignature(_entity, signature);

            m_systemManager.EntitySignatureChange(_entity, signature);
        }

        template <typename T>
        T &GetComponent(Entity _entity)
        {
            return m_componentManager.GetComponent<T>(_entity);
        }

        template <typename T>
        ComponentID GetComponentID()
        {
            return m_componentManager.GetComponentID<T>();
        }

        template <typename T>
        bool HasComponent(Entity _entity)
        {
            return m_componentManager.HasComponent<T>(_entity);
        }


        //---------------------
        // Systems
        //---------------------

        template <typename T, typename... Args>
        void RegisterSystem(Args &&...args)
        {
            m_systemManager.RegisterSystem<T>(std::forward<Args>(args)...);
        }

        template <typename T>
        T &getSystem()
        {
            return m_systemManager.getSystem<T>();
        }

        

        template <typename T>
        void SetSystemSignature(Signature _signature)
        {
            m_systemManager.SetSignature<T>(_signature);
        }

        enum class EventType
        {
            COLLISON = 0
        };

        struct EventMembers
        {
            EventMembers(Entity _triggered, Entity _trigger) : triggered(_triggered), trigger(_trigger) {}
            Entity triggered;
            Entity trigger;
        };

        void RegisterEvent(Entity _triggered, Entity _trigger)
        {
            m_eventMembers.emplace_back(_triggered, _trigger);
        }
        void fortest()
        {
            for (auto &pair : m_eventMembers)
            {
                auto &e1 = pair.triggered;
                auto &e2 = pair.trigger;
                // std::cout<<"Entity " << e1 << " collided with Entity" << e2 <<std::endl;
            }
            m_eventMembers.clear();
        }

        std::vector<EventMembers> &GetEvents()
        {
            return m_eventMembers;
        }

        void Reset()
        {
            // Destroy all alive entities
            for (Entity e = 0; e < MAX_ENTITIES; e++)
            {
                if (m_entityManager.IsAlive(e))
                {
                    m_componentManager.EntityDestroyed(e);
                    m_systemManager.EntityDestroyed(e);
                    m_entityManager.DeleteEntity(e);
                }
            }
        }

    private:
        EntityManager m_entityManager{};
        ComponentManager m_componentManager{};
        SystemManager m_systemManager;
        std::vector<EventMembers> m_eventMembers;
    };
}
