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
)

target_sources( tests
    PRIVATE
    test_fixed_map.cpp
    test_bit_index_iterator.cpp
    test_cfg_iterator.cpp
)
