#pragma once

#include <engine/math.hpp>

#include <SDL_gamecontroller.h>

#include <cstdint>

struct ControllerState {
    SDL_GameController* controller = nullptr;
    SDL_JoystickID id = -1;
    uint32_t btns = 0;
    int16_t lx = 0;
    int16_t ly = 0;
    int16_t lt = 0;
    int16_t rx = 0;
    int16_t ry = 0;
    int16_t rt = 0;

    // operator ~= would be much appreciated
    inline ControllerState& operator ^= ( SDL_GameControllerButton btn ) noexcept
    {
        btns &= ~( 1u << btn );
        return *this;
    }

    inline ControllerState& operator |= ( SDL_GameControllerButton btn ) noexcept
    {
        btns |= 1u << btn;
        return *this;
    }

    inline bool operator & ( SDL_GameControllerButton btn ) const noexcept
    {
        return !!( btns & ( 1u << btn ) );
    }

    inline bool operator == ( std::nullptr_t ) const noexcept
    {
        return !controller;
    }

    inline bool operator == ( SDL_GameController* rhs ) const noexcept
    {
        return controller == rhs;
    }

    inline bool operator == ( SDL_JoystickID rhs ) const noexcept
    {
        return id == rhs;
    }

    inline static int16_t ret_magnitude( int16_t x, int16_t y, int16_t ret, int16_t dzone ) noexcept
    {
        return math::sqrt( (float)x * (float)x + (float)y * (float)y ) > dzone ? ret : 0;
    }
    inline int16_t lt_dzone( int16_t dzone = 1000 ) const noexcept { return lt > dzone ? lt : 0; }
    inline int16_t lx_dzone( int16_t dzone = 8000 ) const noexcept { return ret_magnitude( lx, ly, lx, dzone ); }
    inline int16_t ly_dzone( int16_t dzone = 8000 ) const noexcept { return ret_magnitude( lx, ly, ly, dzone ); }
    inline int16_t rt_dzone( int16_t dzone = 1000 ) const noexcept { return rt > dzone ? rt : 0; }
    inline int16_t rx_dzone( int16_t dzone = 8000 ) const noexcept { return ret_magnitude( rx, ry, rx, dzone ); }
    inline int16_t ry_dzone( int16_t dzone = 8000 ) const noexcept { return ret_magnitude( rx, ry, ry, dzone ); }
};
