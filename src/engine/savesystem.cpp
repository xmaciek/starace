#include <engine/savesystem.hpp>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <type_traits>

static constexpr bool IS_WIDE_CHAR = std::is_same_v<wchar_t, std::filesystem::path::value_type>;

static void appendFileName( std::filesystem::path& path, SaveSystem::Slot s )
{
    if constexpr ( IS_WIDE_CHAR ) {
        path /= std::to_wstring( s ) + L".save";
    }
    else {
        path /= std::to_string( s ) + ".save";
    }
}

static SaveSystem::Slot fileNameToSlot( const std::filesystem::path& path )
{
    SaveSystem::Slot slot = SaveSystem::INVALID_SLOT;
#if defined( __linux__ )
    const int readOK = std::sscanf( path.filename().c_str(), "%u.save", &slot );
    return readOK == 1 && slot < SaveSystem::MAX_SAVES ? slot : SaveSystem::INVALID_SLOT;
#elif defined( __WIN64 )
    return SaveSystem::INVALID_SLOT;
#else
#error Unsupported platform
#endif
}

static std::filesystem::path getSaveDirectory( [[maybe_unused]] std::string_view gameName )
{
#if defined( __linux__ )
    std::filesystem::path ret{};
    const char* config = std::getenv( "XDG_CONFIG_HOME" );
    if ( config ) {
        ret = config;
        ret /= gameName;
        return ret;
    }
    const char* home = std::getenv( "HOME" );
    if ( home ) {
        ret = home;
        ret /= ".config";
        ret /= gameName;
        return ret;
    }
    assert( !"config dir not found" );
    return ret;
#elif defined( __WIN64 )
    return {};
#else
#error Unsupported platform
#endif
}

SaveSystem::~SaveSystem() = default;

SaveSystem::SaveSystem( std::string_view gameName )
: m_gameName{ gameName }
{
    const auto saveDir = getSaveDirectory( gameName );
    if ( saveDir.empty() ) return;
    if ( std::filesystem::exists( saveDir ) ) return;

    [[maybe_unused]]
    const bool createOK = std::filesystem::create_directories( saveDir );
    assert( createOK );
}

void SaveSystem::list( std::function<void(SaveSystem::Slot)>&& cb )
{
    assert( cb );
    for ( auto&& it : std::filesystem::directory_iterator{ getSaveDirectory( m_gameName ) } ) {
        if ( !it.is_regular_file() ) continue;
        const Slot slot = fileNameToSlot( it.path().filename() );
        if ( slot >= MAX_SAVES ) continue;
        cb( slot );
    }
}

SaveSystem::ErrorCode SaveSystem::load( Slot s, std::pmr::vector<uint8_t>& data )
{
    if ( s >= MAX_SAVES ) return ErrorCode::eSlotIndexOutOfRange;
    auto path = getSaveDirectory( m_gameName );
    if ( path.empty() ) return ErrorCode::eSaveDirectoryNotFound;
    appendFileName( path, s );
    std::ifstream ifs( path, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) return ErrorCode::eFileCannotBeOpen;
    const auto size = ifs.tellg();
    ifs.seekg( 0 );
    data.resize( static_cast<size_t>( size ) );
    ifs.read( reinterpret_cast<char*>( data.data() ), size );
    return ifs.good() ? ErrorCode::eSuccess : ErrorCode::eReadWrite;
}

SaveSystem::ErrorCode SaveSystem::save( Slot s, std::span<const uint8_t> data )
{
    if ( s >= MAX_SAVES ) return ErrorCode::eSlotIndexOutOfRange;
    auto path = getSaveDirectory( m_gameName );
    if ( path.empty() ) return ErrorCode::eSaveDirectoryNotFound;
    appendFileName( path, s );
    std::ofstream ofs( path, std::ios::binary );
    if ( !ofs.is_open() ) return ErrorCode::eFileCannotBeOpen;
    ofs.write( reinterpret_cast<const char*>( data.data() ), static_cast<std::streamsize>( data.size() ) );
    return ofs.good() ? ErrorCode::eSuccess : ErrorCode::eReadWrite;
}
