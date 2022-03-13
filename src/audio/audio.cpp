#include <audio/audio.hpp>

#include <shared/indexer.hpp>

#include <Tracy.hpp>

#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <mutex>
#include <string_view>
#include <vector>
#include <memory_resource>

#include <iostream>

constexpr unsigned long long operator""_Hz( unsigned long long hz ) noexcept
{
    return hz;
}

struct Buffer {
    using size_type = std::vector<Uint8>::size_type;
    std::vector<Uint8> data{};
    SDL_AudioSpec spec{};

    ~Buffer() noexcept = default;
    Buffer() noexcept = default;
    Buffer( const Buffer& ) noexcept = default;
    Buffer( Buffer&& ) noexcept = default;
    Buffer& operator=( const Buffer& ) noexcept = default;
    Buffer& operator=( Buffer&& ) noexcept = default;
};

struct BufferPlayed {
    const Uint8* begin = nullptr;
    const Uint8* end = nullptr;
};

class SDLAudioEngine : public Audio {
    SDL_AudioSpec m_spec{};
    SDL_AudioDeviceID m_device{};

    static constexpr uint64_t c_maxSlots = 8;
    Indexer<c_maxSlots> m_slotMachine{};
    std::array<std::atomic<Buffer*>, c_maxSlots> m_audioSlots{};

    std::pmr::vector<BufferPlayed> m_nowPlaying{};
    std::mutex m_playingAccess{};

    static void callback( void*, Uint8*, int );

public:
    virtual ~SDLAudioEngine() override;
    SDLAudioEngine();

    virtual void play( Slot ) override;
    virtual Slot load( std::string_view ) override;
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
    for ( auto& it : m_audioSlots ) {
        delete it.load();
    }
}

SDLAudioEngine::SDLAudioEngine()
{
    ZoneScoped;
    SDL_InitSubSystem( SDL_INIT_AUDIO );
    using size_type = std::vector<std::string>::size_type;
    std::vector<std::string> audioDrivers( static_cast<size_type>( SDL_GetNumAudioDrivers() ) );
    for ( size_t i = 0; i < audioDrivers.size(); ++i ) {
        audioDrivers[ i ] = SDL_GetAudioDriver( (int)i );
    }

    [[maybe_unused]]
    const bool initOK = []( auto& drivers )
    {
        for ( const std::string& it : drivers ) {
            if ( SDL_AudioInit( it.c_str() ) == 0 ) {
                return true;
            }
        }
        return false;
    }( audioDrivers );
    assert( initOK );

    std::vector<std::string> audioDevices( static_cast<size_type>( SDL_GetNumAudioDevices( false ) ) );
    assert( !audioDevices.empty() );
    {
        int idx = 0;
        for ( auto& it : audioDevices ) {
            it = SDL_GetAudioDeviceName( idx++, false );
        }
    }

    const SDL_AudioSpec want{
        .freq = 48000_Hz,
        .format = AUDIO_S16LSB,
        .channels = 2,
        .samples = 4096,
        .callback = &SDLAudioEngine::callback,
        .userdata = this,
    };

    for ( const auto& it : audioDevices ) {
        m_device = SDL_OpenAudioDevice( audioDevices.front().c_str(), 0, &want, &m_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE );
        if ( m_device != 0 ) { break; }
        std::cout << "Failed to open audio device " << it << " : " << SDL_GetError() << std::endl;
    }
    if( m_device == 0 ) {
        std::cout << "Failed to initialize audio" << std::endl;
        std::abort();
    }

    SDL_PauseAudioDevice( m_device, 0 );
}

void SDLAudioEngine::callback( void* userData, Uint8* stream, int len )
{
    ZoneScoped;
    assert( userData );
    assert( stream );

    std::fill_n( stream, len, (Uint8)0 );
    SDLAudioEngine* instance = reinterpret_cast<SDLAudioEngine*>( userData );

    std::lock_guard<std::mutex> lg( instance->m_playingAccess );
    for ( BufferPlayed& it : instance->m_nowPlaying ) {
        assert( it.begin );
        assert( it.end );
        assert( it.begin < it.end );
        const Uint32 sizeRemaining = static_cast<Uint32>( it.end - it.begin );
        const Uint32 playLength = std::min( static_cast<Uint32>( len ), sizeRemaining );
        SDL_MixAudioFormat( stream
            , it.begin
            , instance->m_spec.format
            , playLength
            , SDL_MIX_MAXVOLUME
        );
        it.begin += playLength;
    }

    const auto finishedPlaying = []( const BufferPlayed& sound )
    {
        return sound.begin == sound.end;
    };

    const auto it = std::remove_if( instance->m_nowPlaying.begin(), instance->m_nowPlaying.end(), finishedPlaying );
    instance->m_nowPlaying.erase( it, instance->m_nowPlaying.end() );
}


Audio::Slot SDLAudioEngine::load( std::string_view file )
{
    ZoneScoped;
    Buffer* buffer = new Buffer{};
    Uint8* tmpBuff = nullptr;
    Uint32 tmpLen = 0;

    [[maybe_unused]]
    SDL_AudioSpec* specPtr = SDL_LoadWAV( file.data(), &buffer->spec, &tmpBuff, &tmpLen );
    assert( specPtr );
    assert( &buffer->spec == specPtr );

    SDL_AudioCVT cvt{};
    SDL_BuildAudioCVT( &cvt, buffer->spec.format, buffer->spec.channels, buffer->spec.freq, m_spec.format, m_spec.channels, m_spec.freq );
    assert( cvt.needed );

    buffer->data.resize( static_cast<Buffer::size_type>( tmpLen ) * static_cast<Buffer::size_type>( cvt.len_mult ) );
    cvt.buf = buffer->data.data();
    cvt.len = static_cast<int>( tmpLen );
    std::copy( tmpBuff, tmpBuff + tmpLen, buffer->data.begin() );

    [[maybe_unused]]
    const int convertErr = SDL_ConvertAudio( &cvt );
    assert( convertErr == 0 );
    buffer->data.resize( static_cast<Buffer::size_type>( cvt.len_cvt ) );

    buffer->spec = m_spec;
    SDL_FreeWAV( tmpBuff );

    uint16_t slot = static_cast<uint16_t>( m_slotMachine.next() );
    assert( slot < m_audioSlots.size() );
    auto* previous = m_audioSlots[ slot ].exchange( buffer );
    assert( !previous );
    return slot + 1;
}

void SDLAudioEngine::play( Audio::Slot idx )
{
    assert( idx > 0 );
    idx--;
    assert( idx < m_audioSlots.size() );
    Buffer* buffer = m_audioSlots[ idx ];
    assert( buffer );

    BufferPlayed b{
        .begin = buffer->data.data(),
        .end = buffer->data.data() + buffer->data.size(),
    };
    std::lock_guard<std::mutex> lg( m_playingAccess );
    m_nowPlaying.push_back( b );
}

