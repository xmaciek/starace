#include <audio/audio.hpp>

#include <shared/indexer.hpp>
#include <platform/utils.hpp>

#include <profiler.hpp>

#include <SDL.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <memory_resource>
#include <mutex>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

constexpr unsigned long long operator""_Hz( unsigned long long hz ) noexcept
{
    return hz;
}

class SDLAudio;

using Buffer = std::pmr::vector<Uint8>;

struct PlayerWav {
    SDLAudio* m_instance{};
    uint32_t m_position{};
    Audio::Slot m_slot{};
    Audio::Channel m_channel{};

    void operator () ( std::span<Uint8> );
    inline operator bool () const { return !!m_instance; }
};

using Player = std::variant<PlayerWav>;

class SDLAudio : public Audio {
public:
    SDL_AudioSpec m_spec{};
    std::array<float, (size_t)Audio::Channel::count> m_volumeChannels{};

    std::mutex m_bottleneck;
    std::pmr::vector<Player> m_players;

    static constexpr uint64_t c_maxSlots = 8;
    Indexer<c_maxSlots> m_slotMachine{};
    std::array<Buffer, c_maxSlots> m_audioSlots{};


    SDL_AudioDeviceID m_device{};
    std::pmr::string m_deviceName{};
    std::pmr::string m_driverName{};

    static void callback( void*, Uint8*, int );

    std::pmr::memory_resource* allocator();

    float volume( Channel c ) const
    {
        return m_volumeChannels[ 0 ] * m_volumeChannels[ (size_t)c ];
    }

    virtual ~SDLAudio() override;
    SDLAudio();

    virtual void play( Slot, Channel ) override;
    virtual Slot load( std::span<const uint8_t> ) override;
    virtual void setVolume( Channel, float ) override;
    virtual std::pmr::vector<std::pmr::string> listDrivers() override;
    virtual bool selectDriver( std::string_view ) override;
    virtual std::pmr::vector<std::pmr::string> listDevices() override;
    virtual bool selectDevice( std::string_view ) override;
};

Audio* Audio::create()
{
    return new SDLAudio();
}


void PlayerWav::operator () ( std::span<Uint8> stream )
{
    assert( m_instance );
    assert( m_slot < m_instance->m_audioSlots.size() );
    const Buffer& buffer = m_instance->m_audioSlots[ m_slot ];
    const uint32_t bufferSize = static_cast<uint32_t>( buffer.size() );
    assert( m_position < bufferSize );

    const Uint32 sizeRemaining = static_cast<Uint32>( bufferSize - m_position );
    const Uint32 playLength = std::min( static_cast<Uint32>( stream.size() ), sizeRemaining );
    SDL_MixAudioFormat( stream.data()
        , buffer.data() + m_position
        , m_instance->m_spec.format
        , playLength
        , static_cast<decltype(SDL_MIX_MAXVOLUME)>( SDL_MIX_MAXVOLUME * m_instance->volume( m_channel ) )
    );
    m_position += playLength;
    if ( m_position == bufferSize ) m_instance = nullptr;
}

SDLAudio::~SDLAudio()
{
    ZoneScoped;
    SDL_PauseAudioDevice( m_device, 1 );
    SDL_CloseAudioDevice( m_device );
    SDL_AudioQuit();
}

SDLAudio::SDLAudio()
{
    ZoneScoped;
    std::ranges::fill( m_volumeChannels, 1.0f );
    SDL_InitSubSystem( SDL_INIT_AUDIO );

    auto drivers = listDrivers();
    if ( drivers.empty() ) return;
    selectDriver( drivers.front() );

    auto devices = listDevices();
    if ( devices.empty() ) return;
    selectDevice( devices.front() );
}

std::pmr::memory_resource* SDLAudio::allocator()
{
    return std::pmr::get_default_resource();
}

void SDLAudio::callback( void* userData, Uint8* data, int len )
{
    ZoneScoped;
    assert( userData );
    assert( data );

    std::fill_n( data, len, (Uint8)0 );
    SDLAudio* instance = reinterpret_cast<SDLAudio*>( userData );
    auto& toPlay = instance->m_players;
    std::span<Uint8> stream( data, data + len );
    std::scoped_lock bottleneck( instance->m_bottleneck );
    std::ranges::for_each( toPlay, [stream]( auto& p ) { std::visit( [stream]( auto& pp ) { pp( stream ); }, p ); } );
    std::erase_if( toPlay, []( const auto& p ) { return std::visit( []( const auto& pp ) -> bool { return !pp; }, p ); } );
}

Audio::Slot SDLAudio::load( std::span<const uint8_t> data )
{
    ZoneScoped;
    Buffer buffer{ allocator() };
    Uint8* tmpBuff = nullptr;
    Uint32 tmpLen = 0;

    SDL_RWops* rwops = SDL_RWFromConstMem( data.data(), static_cast<int>( data.size() ) );
    SDL_AudioSpec spec{};
    [[maybe_unused]]
    SDL_AudioSpec* specPtr = SDL_LoadWAV_RW( rwops, 0, &spec, &tmpBuff, &tmpLen );
    assert( &spec == specPtr );

    SDL_AudioCVT cvt{};
    SDL_BuildAudioCVT( &cvt, spec.format, spec.channels, spec.freq, m_spec.format, m_spec.channels, m_spec.freq );
    if ( cvt.needed ) {

        buffer.resize( static_cast<Buffer::size_type>( tmpLen ) * static_cast<Buffer::size_type>( cvt.len_mult ) );
        cvt.buf = buffer.data();
        cvt.len = static_cast<int>( tmpLen );
        std::copy( tmpBuff, tmpBuff + tmpLen, buffer.begin() );

        [[maybe_unused]]
        const int convertErr = SDL_ConvertAudio( &cvt );
        assert( convertErr == 0 );
        buffer.resize( static_cast<Buffer::size_type>( cvt.len_cvt ) );

    }
    SDL_FreeWAV( tmpBuff );

    uint16_t slot = static_cast<uint16_t>( m_slotMachine.next() );
    assert( slot < m_audioSlots.size() );
    m_audioSlots[ slot ] = std::move( buffer );
    SDL_RWclose( rwops );
    return slot + 1;
}

void SDLAudio::play( Audio::Slot idx, Audio::Channel c )
{
    ZoneScoped;
    assert( idx > 0 );
    idx--;
    assert( idx < m_audioSlots.size() );

    PlayerWav play{
        .m_instance = this,
        .m_position = 0,
        .m_slot = idx,
        .m_channel = c,
    };
    std::scoped_lock bottleneck( m_bottleneck );
    m_players.emplace_back( play );
}

void SDLAudio::setVolume( Audio::Channel c, float v )
{
    assert( c < Channel::count );
    m_volumeChannels[ (size_t)c ] = std::clamp( v, 0.0f, 1.0f );
}

std::pmr::vector<std::pmr::string> SDLAudio::listDrivers()
{
    std::pmr::vector<std::pmr::string> drivers( (size_t)SDL_GetNumAudioDrivers() );
    std::ranges::generate( drivers, [i=0]() mutable { return SDL_GetAudioDriver( i++ ); } );
    return drivers;
}

bool SDLAudio::selectDriver( std::string_view name )
{
    auto drivers = listDrivers();
    auto it = std::find( drivers.begin(), drivers.end(), name );
    if ( it == drivers.end() ) return false;
    if ( !m_deviceName.empty() ) {
        SDL_PauseAudioDevice( m_device, 0 );
        SDL_CloseAudioDevice( m_device );
    }
    if ( !m_driverName.empty() ) {
        SDL_AudioQuit();
    }
    m_driverName = *it;
    SDL_AudioInit( it->c_str() );
    selectDevice( m_deviceName );
    return true;
}


std::pmr::vector<std::pmr::string> SDLAudio::listDevices()
{
    int i = SDL_GetNumAudioDevices( false );
    std::pmr::vector<std::pmr::string> devices( (size_t)i );
    std::ranges::generate( devices, [i]() mutable { return SDL_GetAudioDeviceName( --i, false ); } );
    return devices;
}

bool SDLAudio::selectDevice( std::string_view name )
{
    auto devices = listDevices();
    auto it = std::ranges::find( devices, name );
    if ( it == devices.end() ) return false;

    const SDL_AudioSpec want{
        .freq = 48000_Hz,
        .format = AUDIO_S16LSB,
        .channels = 2,
        .samples = 512,
        .callback = &SDLAudio::callback,
        .userdata = this,
    };
    if ( m_device ) SDL_CloseAudioDevice( m_device );
    m_device = SDL_OpenAudioDevice( it->c_str(), 0, &want, &m_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE );
    if ( m_device == 0 ) platform::showFatalError( "Failed to initialize audio device", (std::string)name );
    SDL_PauseAudioDevice( m_device, 0 );
    m_deviceName = *it;
    return true;

}

