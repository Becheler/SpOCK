include_directories(headeronly_src)
include_directories(spock/include)

add_executable(spock spock.cpp)
target_link_libraries(spock LINK_PUBLIC ${Boost_LIBRARIES} stdc++fs)

install(
  TARGETS
    spock
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    COMPONENT bin
  )
