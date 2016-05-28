#pragma once

#include <string>

// TODO: remove these defines after porting is done
#define setAllTexturesFiltering( x )
#define setTextureFiltering( x )
#define FILTERING_NONE 0
#define FILTERING_LINEAR 1
#define FILTERING_BILINEAR 2
#define FILTERING_TRILINEAR 3
#define FILTERING_ANISOTROPIC_X2 4
#define FILTERING_ANISOTROPIC_X4 5
#define FILTERING_ANISOTROPIC_X8 6
#define FILTERING_ANISOTROPIC_X16 7
#define LoadTexture( x ) 0


class Texture {
//private:
public:
    uint32_t m_width, m_height;
    uint32_t m_owidth, m_oheight;
    uint32_t m_type;
    uint8_t m_bpp;
    uint32_t m_ID;
    bool m_resized;
    std::string m_fileName;

public:
    Texture();
    void use() const;
    void erase();
    void load( const std::string& src );
    void fromData( uint8_t* data, uint32_t w, uint32_t h, uint32_t b, uint32_t t );

    bool operator<( const std::string& txt ) const;
    bool operator>( const std::string& txt ) const;
    bool operator==( const std::string& txt ) const;
};
