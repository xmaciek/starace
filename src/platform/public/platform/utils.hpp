#include <string>

namespace platform {

[[noreturn]]
void ShowFatalError( const std::string& title, const std::string& message );

}
