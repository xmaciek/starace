#pragma once

#include <cstdint>
#include <functional>
#include <memory_resource>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class SaveSystem {
    std::pmr::string m_gameName{};

public:
    static constexpr inline const uint32_t MAX_SAVES = 1u;
    static constexpr inline const uint32_t INVALID_SLOT = ~0u;

    enum class ErrorCode : uint32_t {
        eSuccess,
        eSaveDirectoryNotFound,
        eSlotIndexOutOfRange,
        eFileCannotBeOpen,
        eReadWrite,
    };
    using enum ErrorCode;
    using Slot = uint32_t;

    ~SaveSystem();
    SaveSystem( std::string_view gameName );

    void list( std::function<void(Slot)>&& );
    ErrorCode load( Slot, std::pmr::vector<uint8_t>& );
    ErrorCode save( Slot, std::span<const uint8_t> );
};
