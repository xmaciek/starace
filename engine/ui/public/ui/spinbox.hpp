#pragma once

#include <ui/data_model.hpp>
#include <ui/label.hpp>
#include <ui/nineslice.hpp>
#include <ui/tab_order.hpp>

namespace ui {

class SpinBox : public NineSlice {
protected:
    Label* m_label{};
    Label* m_value{};
    DataModel* m_model = nullptr;
    float m_animL = 1.0f;
    float m_animR = 1.0f;
    bool m_focusL : 1 = false;
    bool m_focusR : 1 = false;

    Sprite m_arrowLeft{};
    Sprite m_arrowRight{};

public:
    struct CreateInfo {
        Hash::value_type data;
        Hash::value_type text;
        math::vec2 position{};
        math::vec2 size{};
        uint16_t tabOrder{};
    };

    ~SpinBox() noexcept = default;
    SpinBox() noexcept = default;
    SpinBox( const CreateInfo& ) noexcept;

    virtual void render( const RenderContext& ) const override;
    virtual void update( const UpdateContext& ) override;
    virtual EventProcessing onMouseEvent( const MouseEvent& ) override;
    virtual EventProcessing onAction( ui::Action ) override;
};

template <> struct TabOrdering<SpinBox> : public std::true_type {};

} // namespace ui
