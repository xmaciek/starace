#include <engine/savesystem.hpp>

#include <extra/unicode.hpp>
#include <platform/windows.hpp>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <type_traits>


template <bool B>
static constexpr auto conditional( auto&& t1, auto&& t2 ) noexcept
{
    if constexpr ( B ) return std::forward<decltype(t1)>( t1 );
    else return std::forward<decltype(t2)>( t2 );
}

static constexpr bool IS_WIDE_CHAR = std::is_same_v<wchar_t, std::filesystem::path::value_type>;
static constexpr auto EXTENSION = conditional<IS_WIDE_CHAR>( L".save", ".save" );

static void appendFileName( std::filesystem::path& path, SaveSystem::Slot s )
{
    if constexpr ( IS_WIDE_CHAR ) {
        path /= std::to_wstring( s );
    }
    else {
        path /= std::to_string( s );
    }
    path += EXTENSION;
}

static SaveSystem::Slot fileNameToSlot( const std::filesystem::path& path )
{
    if ( path.extension() != EXTENSION ) return SaveSystem::INVALID_SLOT;

    auto stem = path.stem();
    std::size_t count = 0;
    unsigned long long v = std::stoull( stem.native(), &count);
    return v < SaveSystem::MAX_SAVES && count == stem.native().size()
        ? static_cast<SaveSystem::Slot>( v )
        : SaveSystem::INVALID_SLOT
        ;
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
#elif defined( _WIN64 )
    wchar_t* path = nullptr;
    const HRESULT pathOK = SHGetKnownFolderPath( FOLDERID_SavedGames, 0, 0, &path );
    assert( pathOK == S_OK );
    assert( path );
    if ( pathOK != S_OK || !path ) return {};

    unicode::Transcoder transcoder( gameName );
    std::wstring name{};
    name.resize( transcoder.length() );
    std::ranges::for_each( name, transcoder );
    std::filesystem::path ret = path;
    ret /= name;
    return ret;
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
