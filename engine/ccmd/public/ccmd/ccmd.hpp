#pragma once

#include <cstdint>
#include <memory_resource>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ccmd {

struct Vm;
using Fn = uint32_t( Vm*, void* );

enum class ErrorCode : uint32_t
{
    eSuccess,
    eFunctionFail,
    eBytecodeVersionMismatch,
    eBytecodeNotEnoughData,
    eBytecodeCorrupted,
    eUnresolvedCommand,
};
using enum ErrorCode;

struct Command {
    std::string_view name{};
    Fn* callback{};
    void* ctx{};

    constexpr inline Command( std::string_view n, Fn* fn, void* context = nullptr ) noexcept
    : name{ n }
    , callback{ fn }
    , ctx{ context }
    {
    }

    constexpr inline Command( Fn* fn, void* context = nullptr ) noexcept
    : callback{ fn }
    , ctx{ context }
    {
    }
};

struct CompileContext {
    std::pmr::memory_resource* alloc = std::pmr::get_default_resource();
    inline CompileContext() = default;
};

struct RunContext {
    std::pmr::memory_resource* alloc = std::pmr::get_default_resource();
    std::pmr::string commandName{};
    std::pmr::string commandErrorMessage{};
    uint32_t commandExitCode = 0;
    inline RunContext() = default;
};

std::pmr::vector<std::byte> compile( std::string_view, const CompileContext& ctx = {} );
ErrorCode run( std::span<const Command>, std::span<const std::byte>, RunContext* rctx = nullptr );

uint32_t argc( Vm* );
bool argv( Vm*, uint32_t, bool& );
bool argv( Vm*, uint32_t, float& );
bool argv( Vm*, uint32_t, int32_t& );
bool argv( Vm*, uint32_t, uint8_t& );
bool argv( Vm*, uint32_t, uint32_t& );
bool argv( Vm*, uint32_t, std::pmr::string& );

void setError( Vm*, std::string_view );

}
