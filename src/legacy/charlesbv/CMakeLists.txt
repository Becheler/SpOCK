find_package(cspice)

add_executable(legacy_spock spock/src/main.cpp)

target_link_libraries(legacy_spock
                      CONAN_PKG::cspice
                      CONAN_PKG::openmpi
                      CONAN_PKG::gsl
                    )
