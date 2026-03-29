#pragma once

#include <ui/widget.hpp>

namespace ui {

class List : public Widget {
public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Anchor anchor = Anchor::fTop | Anchor::fLeft;
    };

    virtual ~List() noexcept = default;
    List() noexcept = default;
    List( const List& ) = delete;
    List( List&& ) = default;
    List& operator = ( const List& ) = delete;
    List& operator = ( List&& ) = default;

    List( const CreateInfo& ) noexcept;

    void preRender( RenderContext& ) const override;
    void postRender( RenderContext& ) const override;

};

}
