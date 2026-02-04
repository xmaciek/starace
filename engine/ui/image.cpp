#include "image.hpp"

#include <ui/pipeline.hpp>
#include <ui/property.hpp>

#include <renderer/renderer.hpp>

#include <cassert>
#include <iostream>

namespace ui {

Image::Image( const CreateInfo& ci ) noexcept
: Decorator{ Decorator::CreateInfo{ .position = ci.position, .size = ci.size, .style = "image"_hash, .anchor = ci.anchor } }
{
    m_color = g_uiProperty.color( ci.color );
    if ( ci.data ) {
        m_dataModel = g_uiProperty.dataModel( ci.data );
        m_revision = m_dataModel->revision();
        setSprite( m_dataModel->texture( m_dataModel->current() ) );
    }
    else if ( ci.path ) setSprite( g_uiProperty.sprite( ci.path ) );
    else {
        assert( !"expected data model or sprite id when creating image" );
    }
};

void Image::update( const UpdateContext& )
{
    if ( !m_dataModel ) {
        return;
    }
    DataModel::size_type rev = m_dataModel->revision();
    if ( rev == m_revision ) {
        return;
    }
    m_revision = rev;
    setSprite( m_dataModel->texture( m_dataModel->current() ) );
}

}
