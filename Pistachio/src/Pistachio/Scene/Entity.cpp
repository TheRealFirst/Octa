﻿#include "papch.h"
#include "Entity.h"

namespace Pistachio
{
    Entity::Entity(entt::entity handle, Scene* scene)
        : m_EntityHandle(handle), m_Scene(scene)
    {
    }
}
