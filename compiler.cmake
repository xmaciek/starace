add_library( cxx::flags INTERFACE IMPORTED )

if ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
target_compile_options( cxx::flags INTERFACE
    -Werror
    -Wall
    -Wextra
    -Wno-missing-field-initializers
    -fno-rtti
    -fno-exceptions
)
elseif ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
target_compile_options( cxx::flags INTERFACE
    -Werror
    -Wall
    -Wextra
    -Wno-missing-field-initializers
    -fno-rtti
    -fno-exceptions
)
elseif ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
else ()
    message( FATAL_ERROR "Unsuported compiler" )
endif()
