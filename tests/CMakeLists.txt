###############################################################################
### Command Line interface test
###############################################################################

add_test(
  NAME test_help
COMMAND ${PROJECT_BINARY_DIR}/bin/spock --help)

add_test(
  NAME test_version
COMMAND ${PROJECT_BINARY_DIR}/bin/spock --version)

# add_test(
#   NAME test_application
# COMMAND ${PROJECT_BINARY_DIR}/src/spock ${CMAKE_CURRENT_SOURCE_DIR}/data/spock.config ${CMAKE_CURRENT_SOURCE_DIR}/data/other_input_file.toto)

###############################################################################
### UNIT TEST
###############################################################################


# Keep test files in a separate source directory called unit_test
file(GLOB TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} unit_test/*.cpp)

# Run through each source
foreach(testSrc ${TEST_SRCS})
        # Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        # Add compile target
        add_executable(${testName} ${testSrc} ${version_file})

        # Link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName}
                              CONAN_PKG::boost
                              CONAN_PKG::mp-units
                              )

        # I like to move testing binaries into a testBin directory
        set_target_properties(${testName} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/build/testBin)

        target_include_directories(
          ${testName} PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

        #Finally add it to test execution. Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName}
                 WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bin
                 COMMAND ${PROJECT_BINARY_DIR}/bin/${testName} )

endforeach(testSrc)
