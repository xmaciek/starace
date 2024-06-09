#pragma once

#include <ui/label.hpp>
#include <ui/nineslice.hpp>

#include <functional>
#include <string_view>

namespace ui {

class Button : public NineSlice {
private:
    Label m_label{};
    std::function<void()> m_onTrigger{};

public:
    struct CreateInfo {
        Hash::value_type text{};
        Hash::value_type trigger{};
        math::vec2 position{};
        math::vec2 size{};
        uint16_t tabOrder{};
    };
    ~Button() noexcept = default;
    Button() noexcept = default;
    Button( const CreateInfo& ) noexcept;

    virtual void render( RenderContext ) const override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;

    void setTrigger( std::function<void()> );
    void trigger() const;
    void setText( std::u32string_view );

};

template <> struct TabOrdering<Button> : public std::true_type {};

}

using Button = ui::Button;
