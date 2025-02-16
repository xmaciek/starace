#pragma once

#include <cstdint>
#include <span>
#include <input/actuator.hpp>
#include <input/action.hpp>

#include <memory_resource>
#include <vector>
#include <span>


namespace input {

class Remapper {
    struct Pair {
        Actuator m_min{};
        Actuator m_max{};
        Action::Enum m_userEnum{};

        int16_t m_maxF = 0;
        int16_t m_minF = 0;

        Action makeAction() const;
    };

    std::pmr::vector<Pair> m_data{};


public:
    ~Remapper() = default;
    Remapper() = default;
    uint32_t apply( Actuator::Source, char32_t, std::span<char32_t> ) const;

    void add( Action::Enum, Actuator );
    void add( Action::Enum, Actuator, Actuator );
    std::pmr::vector<Action> updateAndResolve( Actuator );
};

}
