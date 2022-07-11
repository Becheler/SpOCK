include_directories(headeronly_src)
include_directories(spock/include)

add_executable(spock main.cpp)

target_link_libraries(spock
                      CONAN_PKG::boost
                      CONAN_PKG::mp-units
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

add_subdirectory(legacy/charlesbv)
