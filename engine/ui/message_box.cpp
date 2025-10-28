#include <ui/message_box.hpp>

#include <ui/button.hpp>
#include <ui/label.hpp>
#include <ui/property.hpp>

namespace ui {

MessageBox::MessageBox( const CreateInfo& ci )
: NineSlice{ NineSlice::CreateInfo{
    .position = ci.position,
    .size = ci.size,
    .anchor = Anchor::fMiddle | Anchor::fCenter,
} }
{
    m_text = emplace_child<Label>( Label::CreateInfo{ .text = ci.text, .font = "medium"_hash, .position = ci.size * 0.5f, .anchor = Anchor::fMiddle | Anchor::fCenter, } );
    m_blur = g_uiProperty.findMaterial( "blur"_hash );
}

EventProcessing MessageBox::onAction( ui::Action a )
{
    for ( auto&& it : m_buttons ) {
        if ( !it.btn ) break;
        if ( it.act.a != a.a ) continue;
        it.btn->trigger();
        return EventProcessing::eStop;
    }
    return Super::onAction( a );
}

void MessageBox::render( const RenderContext& rctx ) const
{
    using PushConstant = PushConstant<Pipeline::eBlur>;
    const PushConstant horizontal{ .m_direction = 0 };
    const PushConstant vertical{ .m_direction = 1 };
    const DispatchInfo dih{ .m_pipeline = m_blur, .m_uniform = horizontal };
    const DispatchInfo div{ .m_pipeline = m_blur, .m_uniform = vertical };

    rctx.renderer->dispatch( dih );
    rctx.renderer->dispatch( div );
    Super::render( rctx );
}

void MessageBox::addButton( Hash::value_type text, ui::Action::Enum action, std::function<void()>&& trigger )
{
    assert( m_buttonCount < m_buttons.size() );
    uint32_t index = m_buttonCount++;
    const float offset[ 2 ] = { 0.45f, 0.55f };
    const Anchor anchor[ 2 ] = { Anchor::fRight, Anchor::fLeft };
    m_buttons[ index ].btn = emplace_child<Button>( Button::CreateInfo{
        .text = text,
        .position = size() * math::vec2{ offset[ index ], 0.95f },
        .size = size() * math::vec2{ 0.40f, 0.16f },
        .anchor = Anchor::fBottom | anchor[ index ],
    } );
    m_buttons[ index ].btn->setTrigger( std::move( trigger ) );
    m_buttons[ index ].act.a = action;
}

}
