find_package( GTest )
if ( NOT GTest_FOUND )
    return()
endif()

function( add_run_test )
    cmake_parse_arguments( TEST "" "SRC" "LINK" ${ARGN} )
    get_filename_component( TEST_NAME ${TEST_SRC} NAME_WE )
    add_executable( ${TEST_NAME} )
    set_target_properties( ${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests"
    )
    target_sources( ${TEST_NAME} PRIVATE ${TEST_SRC} )
    target_link_libraries( ${TEST_NAME}
        cxx::flags
        GTest::GTest
        GTest::Main
        ${TEST_LINK}
    )
    add_custom_command( TARGET ${TEST_NAME}
        POST_BUILD
        COMMAND ${TEST_NAME} --gtest_brief=1
    )
endfunction()

add_run_test( SRC test_ccmd.cpp LINK ccmd )
add_run_test( SRC test_config.cpp LINK config )
add_run_test( SRC test_config_json.cpp LINK config )
add_run_test( SRC test_fixed_map.cpp LINK shared )
add_run_test( SRC test_fixed_map_view.cpp LINK shared )
add_run_test( SRC test_hash.cpp LINK shared )
add_run_test( SRC test_max_score_element.cpp LINK shared )
add_run_test( SRC test_pool.cpp LINK shared )
add_run_test( SRC test_savesystem.cpp LINK engine )
add_run_test( SRC test_stack_vector.cpp LINK shared )
add_run_test( SRC test_unicode.cpp LINK unicode )
add_run_test( SRC test_unique_add.cpp LINK shared )
