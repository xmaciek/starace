#pragma once

#include <ui/widget.hpp>
#include <ui/data_model.hpp>

class Progressbar : public ui::Widget {
    ui::DataModel* m_dataModel = nullptr;
    ui::DataModel::size_type m_current = 0;
    float m_value = 0.0f;

public:
    struct CreateInfo {
        ui::DataModel* model = nullptr;
        math::vec2 position{};
    };

    virtual ~Progressbar() noexcept override = default;
    Progressbar() noexcept = default;
    Progressbar( const CreateInfo& ) noexcept;

    virtual void render( ui::RenderContext ) const override;
    virtual void update( const UpdateContext& ) override;

};
