#pragma once

#include <engine/action.hpp>

#include <shared/fixed_map.hpp>

#include <tuple>

class ActionMapping {
    FixedMap<Actuator, UserEnumUType, 32> m_actionMap{};

public:
    using Range = std::tuple<const UserEnumUType*, const UserEnumUType*>;
    ActionMapping() noexcept = default;

    void registerAction( uint16_t uid, Actuator );

    Range resolve( Actuator ) const;
};
