add_library( cxx::flags INTERFACE IMPORTED )

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
target_compile_options( cxx::flags INTERFACE
    -Wall
    #-Wconversion # disabled because bit-field narrowing conversion
    -Werror
    -Wextra
    -Wno-multichar
    -Wno-interference-size
    -Wno-missing-field-initializers
    -Wpedantic
    -fno-exceptions
    -fno-rtti
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
)

elseif ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
else ()
    message( FATAL_ERROR "Unsuported compiler" )
endif()
