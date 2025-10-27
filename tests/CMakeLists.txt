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
    ccmd
    unicode
)

target_sources( tests
    PRIVATE
    test_ccmd.cpp
    test_config.cpp
    test_fixed_map.cpp
    test_fixed_map_view.cpp
    test_hash.cpp
    test_max_score_element.cpp
    test_savesystem.cpp
    test_stack_vector.cpp
    test_unicode.cpp
)
