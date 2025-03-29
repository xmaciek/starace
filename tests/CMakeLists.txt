find_package( GTest )
if ( NOT GTest_FOUND )
    return()
endif()

add_executable( tests )

target_link_libraries( tests
    cxx::flags
    GTest::GTest
    GTest::Main
    shared
    config
    extra
    engine
)

target_sources( tests
    PRIVATE
    test_config.cpp
    test_fixed_map.cpp
    test_fixed_map_view.cpp
    test_hash.cpp
    test_savesystem.cpp
    test_unicode.cpp
)
