#pragma once

#include <string>
#include <map>

class Settings {
private:
    std::string m_fileName;
    std::map<std::string, std::string> m_map;

public:
    Settings( const std::string& fileName = std::string() );
    bool open( const std::string& fileName = std::string() );
    bool save() const;
    std::string get( const std::string& key ) const;
    void set( const std::string& key, const std::string& value );
};
