include_directories(spock/include)

add_executable(spock main.cpp)

find_package(cspice REQUIRED)
find_package(openmpi REQUIRED)
find_package(ZLIB REQUIRED)

target_include_directories(
  spock PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
               MPI_CXX_INCLUDE_DIRS
               cspice_DIR)

target_link_libraries(spock
                      CONAN_PKG::boost
                      CONAN_PKG::mp-units
                      CONAN_PKG::openmpi
                      CONAN_PKG::cspice
                      ZLIB::ZLIB
                    )

# We need C++ 20 activated with the concepts library
target_compile_features(spock PUBLIC cxx_std_20)

install(
  TARGETS
    spock
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT bin
  )
