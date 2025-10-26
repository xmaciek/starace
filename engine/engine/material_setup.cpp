#include "material_setup.hpp"
#include <ccmd/ccmd.hpp>
#include <renderer/pipeline.hpp>

#include <array>
#include <algorithm>
#include <cassert>

namespace {

uint32_t readU8( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    auto instance = reinterpret_cast<uint8_t*>( ctx );
    ccmd::argv( vm, 0, *instance );
    return 0;
}

uint32_t readString( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    auto instance = reinterpret_cast<std::pmr::string*>( ctx );
    ccmd::argv( vm, 0, *instance );
    return 0;
}

uint32_t blendMode( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::BlendMode;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    std::pmr::string val;
    ccmd::argv( vm, 0, val );
    switch ( Hash{}( val ) ) {
    case "alpha"_hash: instance->m_blendMode = eAlpha; break;
    case "additive"_hash: instance->m_blendMode = eAdditive; break;
    default: assert( !"unknown blendMode" ); return 1;
    }
    return 0;
}

uint32_t cullMode( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::CullMode;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    std::pmr::string val;
    ccmd::argv( vm, 0, val );
    switch ( Hash{}( val ) ) {
    case "front"_hash: instance->m_cullMode = eFront; break;
    case "back"_hash: instance->m_cullMode = eBack; break;
    default: assert( !"unknown cullMode" ); return 1;
    }
    return 0;
}

uint32_t frontFace( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::FrontFace;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    std::pmr::string val;
    ccmd::argv( vm, 0, val );
    switch ( Hash{}( val ) ) {
    case "cw"_hash: instance->m_frontFace = eCW; break;
    case "ccw"_hash: instance->m_frontFace = eCCW; break;
    default: assert( !"unknown frontFace" ); return 1;
    }
    return 0;
}

uint32_t name( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::FrontFace;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    std::pmr::string val;
    ccmd::argv( vm, 0, val );
    instance->m_userHint = Hash{}( val );
    return 0;
}

uint32_t depthTest( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::CullMode;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    bool val = false;
    ccmd::argv( vm, 0, val );
    instance->m_enableDepthTest = val;
    return 0;
}

uint32_t depthWrite( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::CullMode;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    bool val = false;
    ccmd::argv( vm, 0, val );
    instance->m_enableDepthWrite = val;
    return 0;
}

uint32_t topology( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 1 );
    using enum PipelineCreateInfo::Topology;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );
    std::pmr::string val;
    ccmd::argv( vm, 0, val );
    switch ( Hash{}( val ) ) {
    case "lineStrip"_hash: instance->m_topology = eLineStrip; break;
    case "lineList"_hash: instance->m_topology = eLineList; break;
    case "triangleFan"_hash: instance->m_topology = eTriangleFan; break;
    case "triangleList"_hash: instance->m_topology = eTriangleList; break;
    default: assert( !"unknown topology" ); return 1;
    }
    return 0;
}

uint32_t vertexAssembly( ccmd::Vm* vm, void* ctx )
{
    assert( ccmd::argc( vm ) == 3 );
    using enum PipelineCreateInfo::InputType;
    auto instance = reinterpret_cast<PipelineCreateInfo*>( ctx );

    auto it = std::ranges::find_if( instance->m_vertexAssembly, []( const auto& a ) { return a.m_input == eNone; } );
    if ( it == std::end( instance->m_vertexAssembly ) ) {
        assert( !"input assembly full" );
        return 1;
    }
    auto& assembly = *it;

    std::pmr::string val;
    ccmd::argv( vm, 0, val );
    switch ( Hash{}( val ) ) {
    case "f2"_hash: assembly.m_input = eF2; break;
    case "f3"_hash: assembly.m_input = eF3; break;
    default: assert( !"unknown inputType" ); return 1;
    }
    ccmd::argv( vm, 1, assembly.m_location );
    ccmd::argv( vm, 2, assembly.m_offset );
    return 0;
}

}

void MaterialSetup::operator () ( Asset&& a )
{
    using namespace ccmd;
    PipelineCreateInfo pci{};
    std::pmr::string vertexShader{};
    std::pmr::string fragmentShader{};
    std::pmr::string computeShader{};

    const std::array cmds {
        Command{ "blendMode", &blendMode, &pci },
        Command{ "computeUniform", &readU8, &pci.m_computeUniformCount },
        Command{ "computeImage", &readU8, &pci.m_computeImageCount },
        Command{ "computeShader", &readString, &computeShader },
        Command{ "cullMode", &cullMode, &pci },
        Command{ "depthTest", &depthTest, &pci },
        Command{ "depthWrite", &depthWrite, &pci },
        Command{ "fragmentImage", &readU8, &pci.m_fragmentImageCount },
        Command{ "fragmentShader", &readString, &fragmentShader },
        Command{ "frontFace", &frontFace, &pci },
        Command{ "name", &name, &pci },
        Command{ "topology", &topology, &pci },
        Command{ "vertexShader", &readString, &vertexShader },
        Command{ "vertexUniform", &readU8, &pci.m_vertexUniformCount },
        Command{ "vertexStride", &readU8, &pci.m_vertexStride },
        Command{ "vertexAssembly", &vertexAssembly, &pci },
    };
    auto bytecode = compile( std::string_view{ (const char*)a.data.data(), a.data.size() } );
    assert( !bytecode.empty() );
    auto ec = run( cmds, bytecode );
    assert( ec == ErrorCode::eSuccess );
    if ( vertexShader.size() ) pci.m_vertexShaderData = m_filesystem->viewWait( vertexShader );
    if ( fragmentShader.size() ) pci.m_fragmentShaderData = m_filesystem->viewWait( fragmentShader );
    if ( computeShader.size() ) pci.m_computeShaderData = m_filesystem->viewWait( computeShader );
    auto pip = m_renderer->createPipeline( pci );
    assert( pip );
    assert( pci.m_userHint );
    [[maybe_unused]]
    auto [ it, inserted ] = m_map->insert( std::make_pair( pci.m_userHint, pip ) );
    assert( inserted );
}
