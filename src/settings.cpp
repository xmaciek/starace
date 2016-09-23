#include "settings.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

Settings::Settings( const std::string& fileName ) :
    m_fileName( fileName )
{
    if ( !m_fileName.empty() ) {
        open( m_fileName );
    }
}

static bool startsWith( std::string& str, int8_t c ) {
    std::string::const_iterator it = str.begin();
    while ( it != str.end() ) {
        if ( isblank( *it ) ) {
            it++;
            continue;
        }
        return *it == c;
    }
    return false;
}

static std::vector<std::string> split( const std::string& line )
{
    std::stringstream ss( line );
    std::istream_iterator<std::string> begin( ss );
    std::istream_iterator<std::string> end;
    std::vector<std::string> vec;
    vec.insert( vec.end(), begin, end );
    return vec;
}

bool Settings::open( const std::string& fileName )
{
    if ( !fileName.empty() ) {
        m_fileName = fileName;
    }
    if ( m_fileName.empty() ) {
        return false;
    }

    std::fstream file( m_fileName, std::fstream::in );
    std::string line;
    while ( file && !file.eof() && getline( file, line ) ) {
        if ( line.empty() || startsWith( line, '#' ) ) {
            continue;
        }
        std::vector<std::string> splittedLine = split( line );
        if ( splittedLine.size() > 1 ) {
            set( splittedLine[0], splittedLine[1] );
        }
    }
    file.close();
    return true;
}

bool Settings::save() const
{
// TODO: implement
    return false;
}

std::string Settings::get( const std::string& key ) const
{
    std::map<std::string, std::string>::const_iterator it = m_map.find( key );
    return ( it != m_map.end() ) ? it->second : std::string();
}

void Settings::set( const std::string& key, const std::string& value )
{
    m_map[ key ] = value;
}
