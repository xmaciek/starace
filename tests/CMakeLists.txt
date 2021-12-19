find_package( GTest )
if ( NOT GTest_FOUND )
    return()
endif()

add_executable( tests )
target_link_libraries( tests
    GTest::GTest
    GTest::Main
    shared
)
target_sources( tests
    PRIVATE
    test_fixed_map.cpp
)
