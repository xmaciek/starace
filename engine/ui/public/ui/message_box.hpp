#pragma once

#include <ui/widget.hpp>
#include <renderer/pipeline.hpp>
#include <shared/hash.hpp>

#include <array>
#include <cstdint>
#include <functional>

namespace ui {

class Button;

class MessageBox : public Widget {
    using Super = Widget;
    struct ButtonInfo {
        Button* btn{};
        ui::Action act{};
    };

    std::array<ButtonInfo, 2> m_buttons{};
    uint32_t m_buttonCount = 0;
    PipelineSlot m_blur{};

public:
    struct CreateInfo {
        math::vec2 position{};
        math::vec2 size{};
        Hash::value_type text;
    };
    ~MessageBox() override = default;
    MessageBox( const CreateInfo& );

    virtual EventProcessing onAction( ui::Action ) override;
    virtual void render( const RenderContext& ) const override;
    void addButton( Hash::value_type, ui::Action::Enum, std::function<void()>&& );
};

}
