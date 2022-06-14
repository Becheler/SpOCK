cmake_minimum_required(VERSION 3.23)

# project name and language
project(SpOCK LANGUAGES CXX)

# we default to Release build type if DCMAKE_BUILD_TYPE not provided
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ flags, Debug configuration: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ flags, Release configuration: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "C++ flags, Release configuration with Debug info: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "C++ flags, minimal Release configuration: ${CMAKE_CXX_FLAGS_MINSIZEREL}")

# Use modern C++ with support for concepts and mp-units
set(CMAKE_CXX_STANDARD 20)
# Prevent use of non-portable compiler extensions
set(CMAKE_CXX_EXTENSIONS OFF)
# This makes C++20 a requirement and prevents a "decay" to C++98 when the compiler does not support C++20.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Tell find_package() to first search using Config mode before falling back to Module mode (for conan)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

# add_subdirectory(.conan)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_BINARY_DIR}/conan.cmake"
                TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(
                      REQUIRES
                        mp-units/0.7.0
                        boost/1.79.0
                      GENERATORS
                        cmake_find_package
                      )

# By default, Conan only searches for packages from the two central repositories
# hosted and moderated by **Conan.io** staff: `conan-center` and `conan-transit`.
# We will need packages that are not hosted by these official repositories.
# The [Bincrafters](https://bincrafters.github.io/2017/06/06/using-bincrafters-conan-repository/)
# community posts new packages/versions every week in a separate Conan repository.
conan_add_remote(NAME bincrafters
                 URL https://bincrafters.jfrog.io/artifactory/api/conan/public-conan)

#  Detect settings like OS and architecture
# I think it also detects CMake settings like gcc, gcc-version; cppstd, build_type etc
conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter bincrafters
                    SETTINGS
                      ${settings}
                      compiler.libcxx=libstdc++
                    )
                    
find_package(Boost 1.79 REQUIRED COMPONENTS program_options unit_test_framework)

find_package(mp-units REQUIRED)

add_subdirectory(src)

add_subdirectory(doc)

enable_testing()

add_subdirectory(tests)
