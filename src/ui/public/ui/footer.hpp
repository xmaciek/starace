#pragma once

#include <ui/widget.hpp>
#include <ui/input.hpp>
#include <ui/label.hpp>
#include <shared/hash.hpp>

#include <array>
#include <cstdint>
#include <memory_resource>
#include <span>
#include <string>

namespace ui {

class Footer : public Widget
{
public:
    struct Entry {
        enum class Type : uint32_t {
            unknown,
            eButton,
        };
        Type type = Type::unknown;
        Hash::value_type textId = 0;
        Hash::value_type triggerId = 0;
        Action::Enum action{};
    };

    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        std::span<Entry> entries{};
    };

    virtual ~Footer() noexcept override = default;
    Footer( const CreateInfo& ) noexcept;

    virtual EventProcessing onAction( ui::Action ) override;

private:
    Label* m_label{};
    struct ActionInfo {
        Action::Enum action{};
        Hash::value_type textId = 0;
        Hash::value_type triggerId = 0;
    };
    static constexpr uint32_t MAX_ENTRIES = 4;
    std::array<ActionInfo, MAX_ENTRIES> m_actions{};
    std::pmr::u32string m_text{};

    void refreshText();
};



}
