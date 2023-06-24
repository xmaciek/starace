#include <audio/audio.hpp>

#include <shared/indexer.hpp>

#include <Tracy.hpp>

#include <SDL.h>

#include <array>
#include <atomic>
#include <algorithm>
#include <cassert>
#include <memory_resource>
#include <string_view>
#include <string>
#include <vector>

constexpr unsigned long long operator""_Hz( unsigned long long hz ) noexcept
{
    return hz;
}

using Buffer = std::pmr::vector<Uint8>;

struct alignas( 8 ) PlaySpan {
    uint32_t position;
    Audio::Slot slot;
    bool isEnqueued;
    bool padding;
};
static_assert( sizeof( PlaySpan ) == 8 );

class SDLAudioEngine : public Audio {
    SDL_AudioSpec m_spec{};
    SDL_AudioDeviceID m_device{};

    static constexpr uint64_t c_maxSlots = 8;
    Indexer<c_maxSlots> m_slotMachine{};
    std::array<Buffer, c_maxSlots> m_audioSlots{};

    std::array<std::atomic<PlaySpan>, 64> m_nowPlaying{};

    static void callback( void*, Uint8*, int );

    std::pmr::memory_resource* allocator();

public:
    virtual ~SDLAudioEngine() override;
    SDLAudioEngine();

    virtual void play( Slot ) override;
    virtual Slot load( std::span<const uint8_t> ) override;
};

Audio* Audio::create()
{
    return new SDLAudioEngine();
}

SDLAudioEngine::~SDLAudioEngine()
{
    ZoneScoped;
    SDL_PauseAudioDevice( m_device, 1 );
    SDL_CloseAudioDevice( m_device );
    SDL_AudioQuit();
}

SDLAudioEngine::SDLAudioEngine()
{
    ZoneScoped;
    SDL_InitSubSystem( SDL_INIT_AUDIO );

    using size_type = std::pmr::vector<std::pmr::string>::size_type;
    std::pmr::vector<std::pmr::string> drivers( static_cast<size_type>( SDL_GetNumAudioDrivers() ) );
    std::generate( drivers.begin(), drivers.end(), [idx = 0]() mutable -> std::pmr::string
    {
        return SDL_GetAudioDriver( idx++ );
    } );

    [[maybe_unused]]
    auto driver = std::find_if( drivers.begin(), drivers.end(), []( const auto& name )
    {
        return SDL_AudioInit( name.c_str() ) == 0;
    } );
    assert( driver != drivers.end() );

    std::pmr::vector<std::pmr::string> devices( static_cast<size_type>( SDL_GetNumAudioDevices( false ) ) );
    std::generate( devices.begin(), devices.end(), [idx = 0]() mutable -> std::pmr::string
    {
        return SDL_GetAudioDeviceName( idx++, false );
    } );
    assert( !devices.empty() );
    std::reverse( devices.begin(), devices.end() );

    const SDL_AudioSpec want{
        .freq = 48000_Hz,
        .format = AUDIO_S16LSB,
        .channels = 2,
        .samples = 512,
        .callback = &SDLAudioEngine::callback,
        .userdata = this,
    };

    for ( const auto& name : devices ) {
        m_device = SDL_OpenAudioDevice( name.c_str(), 0, &want, &m_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE );
        if ( m_device != 0 ) { break; }
    }
    if( m_device == 0 ) {
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "Error", "Failed to initialize audio", nullptr );
        std::abort();
    }

    SDL_PauseAudioDevice( m_device, 0 );
}

std::pmr::memory_resource* SDLAudioEngine::allocator()
{
    return std::pmr::get_default_resource();
}

void SDLAudioEngine::callback( void* userData, Uint8* stream, int len )
{
    ZoneScoped;
    assert( userData );
    assert( stream );

    std::fill_n( stream, len, (Uint8)0 );
    SDLAudioEngine* instance = reinterpret_cast<SDLAudioEngine*>( userData );
    auto& toPlay = instance->m_nowPlaying;
    const auto format = instance->m_spec.format;

    for ( auto& it : toPlay ) {
        PlaySpan span = it.load();
        if ( !span.isEnqueued ) continue;
        if ( span.slot == c_invalidSlot ) continue;
        assert( span.slot < instance->m_audioSlots.size() );
        const Buffer& buffer = instance->m_audioSlots[ span.slot ];
        const uint32_t bufferSize = static_cast<uint32_t>( buffer.size() );
        assert( span.position < bufferSize );

        const Uint32 sizeRemaining = static_cast<Uint32>( bufferSize - span.position );
        const Uint32 playLength = std::min( static_cast<Uint32>( len ), sizeRemaining );
        SDL_MixAudioFormat( stream
            , buffer.data() + span.position
            , format
            , playLength
            , SDL_MIX_MAXVOLUME
        );
        span.position += playLength;
        it.store( span.position == bufferSize ? PlaySpan{} : span );
    }
}

Audio::Slot SDLAudioEngine::load( std::span<const uint8_t> data )
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
    assert( cvt.needed );

    buffer.resize( static_cast<Buffer::size_type>( tmpLen ) * static_cast<Buffer::size_type>( cvt.len_mult ) );
    cvt.buf = buffer.data();
    cvt.len = static_cast<int>( tmpLen );
    std::copy( tmpBuff, tmpBuff + tmpLen, buffer.begin() );

    [[maybe_unused]]
    const int convertErr = SDL_ConvertAudio( &cvt );
    assert( convertErr == 0 );
    buffer.resize( static_cast<Buffer::size_type>( cvt.len_cvt ) );

    SDL_FreeWAV( tmpBuff );

    uint16_t slot = static_cast<uint16_t>( m_slotMachine.next() );
    assert( slot < m_audioSlots.size() );
    m_audioSlots[ slot ] = std::move( buffer );
    SDL_RWclose( rwops );
    return slot + 1;
}

void SDLAudioEngine::play( Audio::Slot idx )
{
    ZoneScoped;
    assert( idx > 0 );
    idx--;
    assert( idx < m_audioSlots.size() );

    PlaySpan span{
        .position = 0,
        .slot = idx,
        .isEnqueued = true,
    };
    for ( auto& it : m_nowPlaying ) {
        PlaySpan empty{};
        if ( it.compare_exchange_weak( empty, span ) ) return;
    }
    assert( !"too many sounds playing" );
}
