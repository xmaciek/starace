#include "Texture.h"

#include "shader.hpp"

#include <string.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Texture::Texture() :
    m_width( 0 ),
    m_height( 0 ),
    m_owidth( 0 ),
    m_oheight( 0 ),
    m_type( 0 ),
    m_bpp( 0 ),
    m_ID( 0 ),
    m_resized( false )
{
}

Texture::operator Material () const
{
    return Material( m_ID, Material::Texture );
}

void Texture::use() const
{
    SHADER::setMaterial( *this );
}

void Texture::erase() {
  glDeleteTextures( 1, &m_ID );
  m_width = m_height = m_type = m_bpp = m_ID = 0;
}

static uint32_t pow2( uint32_t a ) {
    uint32_t r = 4;
    while ( r <= a ) { r *= 2; }
    return r;
}

void Texture::fromData( uint8_t* data, uint32_t w, uint32_t h, uint32_t b, uint32_t t ) {
    assert( data );
    m_fileName.clear();
    m_width = w;
    m_owidth = w;
    m_height = h;
    m_oheight = h;
//     m_width = pow2( m_width );
//     m_height = pow2( m_height );
    m_bpp = b;
//     if ( m_oheight != m_height || m_owidth != m_width ) {
//         m_resized = true;
//         uint8_t* newData = new uint8_t[ m_width * m_height * m_bpp ];
//         uint8_t* oldData = data;
//         memset( newData, 0, m_width * m_height * m_bpp );
//         for ( uint32_t i = 0; i < m_oheight; i++ ) {
//             memcpy( newData + ( i * m_width * m_bpp ), oldData + ( i * m_owidth * m_bpp), m_bpp * m_owidth );
//         }
//         data = newData;
//         delete[] oldData;
//     }

    m_type = t;
    glGenTextures( 1, &m_ID );
    glBindTexture( GL_TEXTURE_2D, m_ID );
    glTexImage2D( GL_TEXTURE_2D, 0, m_type, m_width, m_height, 0, m_type, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap( GL_TEXTURE_2D );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f );
    delete[] data;
}

void Texture::load( const std::string &src ) {
    m_fileName = src;
    std::fstream file( src.c_str(), std::fstream::in | std::fstream::binary );
    if ( !file.is_open() ) {
        fprintf( stderr, "No such file: %s\n", src.c_str() );
        return;
    }

    std::vector<uint8_t> header( 12, 0 );
    file.read( reinterpret_cast<char*>( &header[0] ), 12 );
    file.read( reinterpret_cast<char*>( &m_width ), 2 );
    file.read( reinterpret_cast<char*>( &m_height ), 2 );
    file.read( reinterpret_cast<char*>( &m_bpp ), 2 );

    m_bpp /= 8;
    m_type = ( m_bpp == 3 ) ? GL_RGB : GL_RGBA;

    const uint32_t size = m_width * m_height * m_bpp;
    uint8_t* loadedData = new uint8_t[size];
    file.read( reinterpret_cast<char*>( loadedData ), size );
    file.close();

    uint8_t swap;
    for ( uint32_t i = 0; i < size; i += m_bpp ) {
        swap = loadedData[i + 2];
        loadedData[i + 2] = loadedData[i];
        loadedData[i] = swap;
    }

    fromData( loadedData, m_width, m_height, m_bpp, m_type );
}

bool Texture::operator<( const std::string& txt ) const {
  return m_fileName < txt;
}

bool Texture::operator>( const std::string& txt ) const {
  return m_fileName > txt;
}

bool Texture::operator==( const std::string& txt ) const {
  return m_fileName == txt;
}



