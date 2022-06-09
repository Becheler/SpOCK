include_directories(headeronly_src)
include_directories(spock/include)

add_executable(spock main.cpp)

target_link_libraries(spock PRIVATE ${Boost_LIBRARIES}
                                    Boost::headers
                                    mp-units::mp-units
                                    stdc++fs
                      )

# We need C++ 20 activated with the concepts library
target_compile_features(spock PUBLIC cxx_std_20)
target_compile_options(spock PRIVATE "-fconcepts")

install(
  TARGETS
    spock
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT bin
  )
