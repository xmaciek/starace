#include <audio/audio.hpp>

#include <Tracy.hpp>

#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <iostream>

constexpr unsigned long long operator""_Hz( unsigned long long hz ) noexcept
{
    return hz;
}

struct Buffer {
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
    Buffer* buffer = nullptr;
    Uint32 position = 0;
};

class SDLAudioEngine : public Audio {
    SDL_AudioSpec m_spec{};
    SDL_AudioDeviceID m_device{};
    std::map<void*, Buffer> m_loadedBuffers{};
    std::vector<BufferPlayed> m_nowPlaying{};
    std::mutex m_playingAccess{};

    static void callback( void*, Uint8*, int );

public:
    virtual ~SDLAudioEngine() override;
    SDLAudioEngine();

    virtual void play( const Chunk& ) override;
    virtual Chunk load( std::string_view ) override;
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
    std::vector<std::string> audioDrivers( SDL_GetNumAudioDrivers() );
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

    std::vector<std::string> audioDevices( SDL_GetNumAudioDevices( false ) );
    for ( size_t i = 0; i < audioDevices.size(); ++i ) {
        audioDevices[ i ] = SDL_GetAudioDeviceName( (int)i, false );
    }
    assert( !audioDevices.empty() );

    const SDL_AudioSpec want{
        .freq = 48000_Hz,
        .format = AUDIO_S16LSB,
        .channels = 2,
        .samples = 4096,
        .callback = &SDLAudioEngine::callback,
        .userdata = this,
    };

    m_device = SDL_OpenAudioDevice( audioDevices.front().c_str(), 0, &want, &m_spec, SDL_AUDIO_ALLOW_FORMAT_CHANGE );
    if ( m_device == 0 ) {
        std::cout << " Failed to open audio: " << SDL_GetError() << std::endl;
        return;
    }

    if ( want.format != m_spec.format ) {
        std::cout << "Audio format changed" << std::endl;
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
        assert( it.buffer );
        assert( it.buffer->data.size() >= it.position );
        const Uint32 lengthRemaining = it.buffer->data.size() - it.position;
        const Uint32 playLength = std::min<Uint32>( len, lengthRemaining );
        SDL_MixAudioFormat( stream, it.buffer->data.data() + it.position, instance->m_spec.format, playLength, SDL_MIX_MAXVOLUME );

        it.position += playLength;
    }

    const auto finishedPlaying =
        []( const BufferPlayed& sound ) {
            return sound.position >= sound.buffer->data.size();
        };

    const auto it = std::remove_if( instance->m_nowPlaying.begin(), instance->m_nowPlaying.end(), finishedPlaying );

    instance->m_nowPlaying.erase( it, instance->m_nowPlaying.end() );
}

Audio::Chunk SDLAudioEngine::load( std::string_view file )
{
    ZoneScoped;
    Buffer buffer{};
    Uint8* tmpBuff = nullptr;
    Uint32 tmpLen = 0;

    [[maybe_unused]]
    SDL_AudioSpec* specPtr = SDL_LoadWAV( file.data(), &buffer.spec, &tmpBuff, &tmpLen );
    assert( specPtr );
    assert( &buffer.spec == specPtr );

    SDL_AudioCVT cvt{};
    SDL_BuildAudioCVT( &cvt, buffer.spec.format, buffer.spec.channels, buffer.spec.freq, m_spec.format, m_spec.channels, m_spec.freq );
    assert( cvt.needed );

    buffer.data.resize( tmpLen * cvt.len_mult );
    cvt.buf = buffer.data.data();
    cvt.len = (int)tmpLen;
    std::copy( tmpBuff, tmpBuff + tmpLen, buffer.data.begin() );

    [[maybe_unused]]
    const int convertErr = SDL_ConvertAudio( &cvt );
    assert( convertErr == 0 );
    buffer.data.resize( cvt.len_cvt );

    buffer.spec = m_spec;
    SDL_FreeWAV( tmpBuff );

    void* bufferId = buffer.data.data();
    [[maybe_unused]]
    const auto ret = m_loadedBuffers.emplace( std::make_pair( bufferId, std::move( buffer ) ) );
    assert( ret.second );
    return Chunk{ bufferId };
}

void SDLAudioEngine::play( const Chunk& c )
{
    const auto& it = m_loadedBuffers.find( c.data );
    if ( it == m_loadedBuffers.cend() ) {
        return;
    }

    BufferPlayed buf{};
    buf.buffer = &it->second;
    std::lock_guard<std::mutex> lg( m_playingAccess );
    m_nowPlaying.emplace_back( buf );
}

