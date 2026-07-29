#include <audio/audio.hpp>

#include <shared/indexer.hpp>
#include <platform/utils.hpp>

#include <profiler.hpp>

#include <SDL.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <map>
#include <memory_resource>
#include <mutex>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <utility>

static constexpr inline bool operator < ( const SDL_AudioSpec& l, const SDL_AudioSpec& r ) noexcept
{
    if ( l.format != r.format ) return l.format < r.format;
    if ( l.channels != r.channels ) return l.channels < r.channels;
    return l.freq < r.freq;
}

namespace {
constexpr unsigned long long operator""_Hz( unsigned long long hz ) noexcept
{
    return hz;
}

static constexpr SDL_AudioSpec DEFAULT_SPEC{
    .format = SDL_AUDIO_S16LE,
    .channels = 2,
    .freq = 48000_Hz,
};

class DeviceVector {
    SDL_AudioDeviceID* m_ptr{};
    int m_size{};

    inline DeviceVector( SDL_AudioDeviceID* ptr, int size ) : m_ptr{ ptr }, m_size{ size } {}

public:
    ~DeviceVector() { SDL_free( m_ptr ); }
    DeviceVector() = default;
    DeviceVector( const DeviceVector& ) = delete;
    DeviceVector& operator = ( const DeviceVector& ) = delete;
    DeviceVector( DeviceVector&& );
    DeviceVector& operator = ( DeviceVector&& );

    static DeviceVector getPlayback();

    inline auto begin() { return m_ptr; };
    inline auto end() { return m_ptr + m_size; };

    std::pmr::vector<std::pmr::string> getNames();

    SDL_AudioDeviceID find( std::string_view );
};


SDL_AudioDeviceID DeviceVector::find( std::string_view sv )
{
    auto it = std::ranges::find_if( *this, [sv]( auto ptr ) { return sv == SDL_GetAudioDeviceName( ptr ); } );
    return it != end() ? *it : 0;
}

DeviceVector::DeviceVector( DeviceVector&& r )
: m_ptr{ std::exchange( r.m_ptr, {} ) }
, m_size{ std::exchange( r.m_size, {} ) }
{}

DeviceVector& DeviceVector::operator = ( DeviceVector&& r )
{
    std::swap( m_ptr, r.m_ptr );
    std::swap( m_size, r.m_size );
    return *this;
}

DeviceVector DeviceVector::getPlayback()
{
    DeviceVector ret{};
    ret.m_ptr = SDL_GetAudioPlaybackDevices( &ret.m_size );
    assert( ret.m_ptr );
    assert( ret.m_size );
    return ret;
}

std::pmr::vector<std::pmr::string> DeviceVector::getNames()
{
    std::pmr::vector<std::pmr::string> ret( (size_t)m_size );
    std::ranges::transform( *this, ret.begin(), &SDL_GetAudioDeviceName );
    return ret;
}

class Device {
    SDL_AudioDeviceID m_id{};

public:
    inline ~Device() { if ( m_id ) SDL_CloseAudioDevice( m_id ); }
    Device() = default;
    explicit inline Device( SDL_AudioDeviceID id ) { assert( id ); m_id = SDL_OpenAudioDevice( id, &DEFAULT_SPEC ); }
    inline Device( Device&& r ) : m_id{ std::exchange( r.m_id, {} ) } {}
    inline Device& operator = ( Device&& r ) { std::swap( m_id, r.m_id ); return * this; }
    Device( const Device& ) = delete;
    Device& operator = ( const Device& ) = delete;

    inline operator SDL_AudioDeviceID () const { return m_id; }
    inline operator bool () const { return !!m_id; }
};

struct WavSound {
    Uint8* m_ptr{};
    Uint32 m_size{};
    SDL_IOStream* m_stream{};
    SDL_AudioSpec m_spec{};

    ~WavSound();
    WavSound() = default;
    WavSound( std::span<uint8_t> );
    WavSound( WavSound&& );
    WavSound( const WavSound& ) = delete;
    WavSound& operator = ( WavSound&& );
    WavSound& operator = ( const WavSound& ) = delete;
};

WavSound::~WavSound()
{
    SDL_CloseIO( m_stream );
    SDL_free( m_ptr );
}

WavSound::WavSound( std::span<uint8_t> data )
{
    m_stream = SDL_IOFromMem( data.data(), data.size() );
    assert( m_stream );
    [[maybe_unused]]
    bool loadOK = SDL_LoadWAV_IO( m_stream, false, &m_spec, &m_ptr, &m_size );
    assert( loadOK );
}

WavSound::WavSound( WavSound&& r )
: m_ptr{ std::exchange( r.m_ptr, {} ) }
, m_size{ std::exchange( r.m_size, {} ) }
, m_stream{ std::exchange( r.m_stream, {} ) }
, m_spec{ std::exchange( r.m_spec, {} ) }
{}

WavSound& WavSound::operator = ( WavSound&& r )
{
    std::swap( m_ptr, r.m_ptr );
    std::swap( m_size, r.m_size );
    std::swap( m_stream, r.m_stream );
    std::swap( m_spec, r.m_spec );
    return *this;
}

}

class SDLAudio : public Audio {
public:
    std::pmr::map<SDL_AudioSpec, std::pmr::vector<SDL_AudioStream*>> m_streams{};

    std::array<float, (size_t)Audio::Channel::count> m_volumeChannels;
    static constexpr uint64_t MAX_SLOTS = 8;
    Indexer<MAX_SLOTS> m_slotMachine{};
    std::array<WavSound, MAX_SLOTS> m_audioSlots{};

    Device m_device{};
    DeviceVector m_devices{};
    std::pmr::string m_deviceName{};
    std::pmr::string m_driverName{};

    float volume( Channel c ) const
    {
        return m_volumeChannels[ 0 ] * m_volumeChannels[ (size_t)c ];
    }

    SDL_AudioStream* findStream( const SDL_AudioSpec& );

    virtual ~SDLAudio() override;
    SDLAudio();

    virtual void play( Slot, Channel ) override;
    virtual Slot load( std::span<uint8_t> ) override;
    virtual void setVolume( Channel, float ) override;
    virtual std::pmr::vector<std::pmr::string> listDevices() override;
    virtual bool selectDevice( std::string_view ) override;
};

Audio* Audio::create()
{
    return new SDLAudio();
}

SDLAudio::~SDLAudio()
{
    ZoneScoped;
    if ( m_device ) SDL_PauseAudioDevice( m_device );
    std::ranges::for_each( m_streams, []( auto& v ) {
        std::ranges::for_each( v.second, &SDL_DestroyAudioStream );
    });
}

SDLAudio::SDLAudio()
{
    ZoneScoped;
    m_volumeChannels.fill( 1.0f );
    SDL_InitSubSystem( SDL_INIT_AUDIO );

    m_devices = DeviceVector::getPlayback();

    auto devices = listDevices();
    if ( devices.empty() ) return;
    selectDevice( devices.front() );
}

SDL_AudioStream* SDLAudio::findStream( const SDL_AudioSpec& spec )
{
    auto& vec = m_streams[ spec ];
    auto it = std::ranges::find_if( vec, []( auto* s ) { return 0 == SDL_GetAudioStreamQueued( s ); } );
    if ( it != vec.end() ) return *it;

    auto stream = SDL_CreateAudioStream( &spec, &DEFAULT_SPEC );
    assert( stream );
    SDL_BindAudioStream( m_device, stream );
    vec.emplace_back( stream );
    return stream;
}

Audio::Slot SDLAudio::load( std::span<uint8_t> data )
{
    ZoneScoped;
    uint16_t slot = static_cast<uint16_t>( m_slotMachine.next() );
    assert( slot < m_audioSlots.size() );
    m_audioSlots[ slot ] = WavSound{ data };
    return slot + 1;
}

void SDLAudio::play( Audio::Slot idx, Audio::Channel c )
{
    ZoneScoped;
    assert( idx > 0 );
    idx--;
    assert( idx < m_audioSlots.size() );
    auto& audio = m_audioSlots[ idx ];
    auto stream = findStream( audio.m_spec );
    assert( stream );
    SDL_SetAudioStreamGain( stream, volume( c ) );
    SDL_PutAudioStreamDataNoCopy( stream, audio.m_ptr, (int)audio.m_size, nullptr, nullptr );
    SDL_FlushAudioStream( stream );
}

void SDLAudio::setVolume( Audio::Channel c, float v )
{
    assert( c < Channel::count );
    m_volumeChannels[ (size_t)c ] = std::clamp( v, 0.0f, 1.0f );
}

std::pmr::vector<std::pmr::string> SDLAudio::listDevices()
{
    return m_devices.getNames();
}

bool SDLAudio::selectDevice( std::string_view name )
{
    auto deviceID = m_devices.find( name );
    m_device = Device{ deviceID ? deviceID : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK };
    if ( !m_device ) platform::showFatalError( "Failed to initialize audio device", (std::string)name );
    [[maybe_unused]]
    bool resumeOK = SDL_ResumeAudioDevice( m_device );
    assert( resumeOK );
    m_deviceName = SDL_GetAudioDeviceName( m_device );
    return true;

}

