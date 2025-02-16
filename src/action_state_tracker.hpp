#pragma once

#include <input/actuator.hpp>
#include <input/action.hpp>

#include <cstdint>
#include <memory_resource>
#include <vector>

class ActionStateTracker {
    struct Pair {
        input::Actuator m_min{};
        input::Actuator m_max{};
        input::Action::Enum m_userEnum{};

        int16_t m_maxF = 0;
        int16_t m_minF = 0;

        input::Action makeAction() const;
    };

    std::pmr::vector<Pair> m_data{};

public:
    void add( input::Action::Enum, input::Actuator );
    void add( input::Action::Enum, input::Actuator, input::Actuator );
    std::pmr::vector<input::Action> updateAndResolve( input::Actuator );

};
