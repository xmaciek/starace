add_library( cxx::flags INTERFACE IMPORTED )

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
target_compile_options( cxx::flags INTERFACE
    -Wall
    #-Wconversion # disabled because bit-field narrowing conversion
    -Werror
    -Wextra
    -Wno-interference-size
    -Wno-missing-field-initializers
    -Wno-multichar
    -Wpedantic
    # -Wshadow # disabled because false-positive shadowing in lambdas
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
    -Wno-multichar
    -Wno-string-conversion
    -Wpedantic
    -Wshadow
    -fno-exceptions
    -fno-rtti
)

elseif ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
target_compile_options( cxx::flags INTERFACE
    /D _ENABLE_EXTENDED_ALIGNED_STORAGE=1
    /wd4566
)

else ()
    message( FATAL_ERROR "Unsuported compiler" )
endif()
