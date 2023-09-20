#pragma once

#include <ui/widget.hpp>
#include <ui/input.hpp>
#include <shared/hash.hpp>

#include <span>
#include <cstdint>
#include <array>

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

    virtual void render( RenderContext ) const override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;

private:
    struct ActionInfo {
        Action::Enum action{};
        Hash::value_type textId = 0;
        Hash::value_type triggerId = 0;
    };
    std::array<ActionInfo, 4> m_actions{};
};



}
