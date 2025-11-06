#pragma once

#include <ui/label.hpp>
#include <ui/nineslice.hpp>
#include <shared/hash.hpp>

#include <functional>
#include <string_view>
#include <variant>

namespace ui {

class Button : public NineSlice {
private:
    Label* m_label{};
    Hash::value_type m_screenChange{};
    std::variant<std::monostate, Hash::value_type, std::function<void()>> m_trigger{};

public:
    struct CreateInfo {
        Hash::value_type text{};
        Hash::value_type trigger{};
        Hash::value_type screenChange{};
        math::vec2 position{};
        math::vec2 size{};
        uint16_t tabOrder{};
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };
    ~Button() noexcept = default;
    Button() noexcept = default;
    Button( const CreateInfo& ) noexcept;

    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;

    void trigger() const;
    void setTrigger( std::function<void()>&& );
    void setText( std::u32string_view );

};

template <> struct TabOrdering<Button> : public std::true_type {};

}

using Button = ui::Button;
