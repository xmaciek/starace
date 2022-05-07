add_library( cxx::flags INTERFACE IMPORTED )

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
target_compile_options( cxx::flags INTERFACE
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Wno-missing-field-initializers
    -Wpedantic
    -fno-exceptions
    -fno-rtti
    -mcx16
)
elseif ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
target_compile_options( cxx::flags INTERFACE
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Wno-missing-field-initializers
    -Wno-string-conversion
    -Wpedantic
    -fno-exceptions
    -fno-rtti
    -mcx16
)
elseif ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
else ()
    message( FATAL_ERROR "Unsuported compiler" )
endif()
