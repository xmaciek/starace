#pragma once

#include <cstdint>

namespace ui {

class ScrollIndex {
public:
    using value_type = uint16_t;

private:
    value_type m_offset = 0;
    value_type m_maxOffset = 0;
    value_type m_index = 0;
    value_type m_max = 0;
    value_type m_maxVisible = 8;

public:
    ~ScrollIndex() = default;
    ScrollIndex() = default;
    ScrollIndex( value_type current, value_type max ) noexcept;

    value_type current() const;
    value_type currentVisible() const;
    value_type maxVisible() const;
    value_type offset() const;
    void increase();
    void decrease();
    void scrollUp( value_type );
    void scrollDown( value_type );
    void selectVisible( value_type );
};

}
