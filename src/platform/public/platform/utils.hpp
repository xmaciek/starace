#include <string>

namespace platform {

[[noreturn]]
void showFatalError( const std::string& title, const std::string& message );

}
