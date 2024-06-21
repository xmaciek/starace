#include <ui/remapper.hpp>

#include <ui/input.hpp>

#include <algorithm>

namespace ui {

uint32_t Remapper::apply( char32_t chr, std::span<char32_t> out ) const
{
    switch ( (ui::Action::Enum)chr ) {
    case ui::Action::eMenuConfirm: std::copy_n( U"[Enter]", 7, out.begin() ); return 7;
    case ui::Action::eMenuCancel: std::copy_n( U"[Esc]", 5, out.begin() ); return 5;
    case ui::Action::eMenuApply: std::copy_n( U"[Space]", 7, out.begin() ); return 7;
    default: break;
    }
    return 0;
}

}
