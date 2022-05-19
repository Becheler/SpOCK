cmake_minimum_required(VERSION 3.12)

add_test(
  NAME test_help
COMMAND ${PROJECT_BINARY_DIR}/src/spock --help)

add_test(
  NAME test_version
COMMAND ${PROJECT_BINARY_DIR}/src/spock --version)

add_test(
  NAME test_application
COMMAND ${PROJECT_BINARY_DIR}/src/spock ${CMAKE_CURRENT_SOURCE_DIR}/data/spock.config ${CMAKE_CURRENT_SOURCE_DIR}/data/other_input_file.toto)

install(DIRECTORY data/ DESTINATION ${CMAKE_INSTALL_PREFIX})
