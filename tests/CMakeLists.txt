find_package( GTest )
if ( NOT GTest_FOUND )
    return()
endif()

add_executable( tests )
set_cppver( tests 20 )

target_link_libraries( tests
    cxx::flags
    GTest::GTest
    GTest::Main
    shared
    config
)

target_sources( tests
    PRIVATE
    test_config.cpp
    test_fixed_map.cpp
    test_fixed_map_view.cpp
    test_bit_index_iterator.cpp
    test_hash.cpp
)
