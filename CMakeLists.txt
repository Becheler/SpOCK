# Note: CMake version has to be newer than the Boost version: see https://stackoverflow.com/a/43885372/10360134
# Boost 1.79.0 (13 April 2022) is too recent and does not link
# Boost 1.76.0 is too old and does not compile with GCC 10
# Boost 1.77.0 (13th August 2021) works with:
#     - CMake 3.23.2 (May 25, 2022)
#     - GCC 10.3.0 (May 7, 2020)
#     - mp-units 0.7.0
# BE SUPER CAREFUL IF YOU CHANGE ANY VERSION ~ Captain SpOCK, 45th August 3045
cmake_minimum_required(VERSION 3.23.2)

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

set(Boost_DEBUG ON)

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
                        cmake
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

# Since GCC >= 5, the compiler is likely to be using the new CXX11 ABI by default (libstdc++11)
# See https://docs.conan.io/en/latest/howtos/manage_gcc_abi.html
conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter bincrafters
                    SETTINGS
                      ${settings}
                      compiler.libcxx=libstdc++11
                    )

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

#find_package(Boost 1.79 REQUIRED COMPONENTS program_options unit_test_framework REQUIRED)

#find_package(mp-units REQUIRED)

add_subdirectory(src)

add_subdirectory(doc)

enable_testing()

add_subdirectory(tests)
