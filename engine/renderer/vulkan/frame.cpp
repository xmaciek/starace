#include "frame.hpp"

#include <algorithm>
#include <cassert>

void Frame::reset()
{
    m_state = Frame::State::eNone;
    m_uniformBuffer.reset();
    std::ranges::for_each( m_descriptorSets, []( auto& s ) { s.reset(); } );
    m_pendingScissor.reset();
    m_scissorStack.clear();
    m_commandPool.reset();
}

void Frame::pushScissor( const VkRect2D& rect )
{
    m_scissorStack.emplace_back( rect );
    m_pendingScissor = m_scissorStack.back();
}

void Frame::popScissor()
{
    assert( !m_scissorStack.empty() );
    m_scissorStack.pop_back();
    assert( !m_scissorStack.empty() );
    m_pendingScissor = m_scissorStack.back();
}

void Frame::processPendingOperations()
{
    if ( m_pendingScissor ) [[unlikely]] {
        vkCmdSetScissor( m_cmdDepthPrepass, 0, 1, &*m_pendingScissor );
        vkCmdSetScissor( m_cmdColorPass, 0, 1, &*m_pendingScissor );
        m_pendingScissor.reset();
    }
}

static constexpr uint64_t operator | ( Frame::State from, Frame::State to )
{
    uint64_t ret = static_cast<uint64_t>( from );
    ret <<= 32;
    ret |= static_cast<uint64_t>( to );
    return ret;
}

void Frame::switchState( State newState )
{
    using enum State;
    if ( m_state == newState ) [[likely]] return;
    switch ( m_state | newState ) {
    case eCompute | eGraphics:
        m_renderDepthTarget.transfer( m_cmdDepthPrepass, constants::depthWrite );
        m_renderTarget.transfer( m_cmdColorPass, constants::fragmentWrite );
        m_depthPrepass.resume( m_cmdDepthPrepass, m_renderDepthTarget, m_renderTarget );
        m_mainPass.resume( m_cmdColorPass, m_renderDepthTarget, m_renderTarget );
        break;

    case eNone | eGraphics:
        pushScissor( VkRect2D{ {}, m_renderDepthTarget.extent() } );
        m_renderDepthTarget.transfer( m_cmdDepthPrepass, constants::depthWrite );
        m_renderTarget.transfer( m_cmdColorPass, constants::fragmentWrite );
        m_depthPrepass.begin( m_cmdDepthPrepass, m_renderDepthTarget, m_renderTarget );
        m_mainPass.begin( m_cmdColorPass, m_renderDepthTarget, m_renderTarget );
        break;

    case eGraphics | eNone:
        m_depthPrepass.end( m_cmdDepthPrepass );
        m_mainPass.end( m_cmdColorPass );
        [[fallthrough]];

    case eCompute | eNone:
        m_renderDepthTarget.transfer( m_cmdDepthPrepass, constants::depthRead );
        m_renderTarget.transfer( m_cmdColorPass, constants::copyFrom );
        break;

    case eGraphics | eCompute:
        m_depthPrepass.end( m_cmdDepthPrepass );
        m_mainPass.end( m_cmdColorPass );
        [[fallthrough]];

    case eNone | eCompute:
        m_renderTarget.transfer( m_cmdColorPass, constants::computeReadWrite );
        m_renderTargetTmp.transfer( m_cmdColorPass, constants::computeReadWrite );
        break;

    default:
        assert( !"unexpected frame state" );
        break;
    }
    m_state = newState;
}
