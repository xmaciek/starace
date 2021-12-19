#pragma once

#include <engine/action.hpp>

#include <shared/fixed_map.hpp>

#include <tuple>

class ActionMapping {
    FixedMap<Actuator, Action::Enum, 32> m_actionMap{};

public:
    using Range = std::tuple<const Action::Enum*, const Action::Enum*>;
    ActionMapping() noexcept = default;

    void registerAction( Action::Enum, Actuator );

    Range resolve( Actuator ) const;
};
