# CMake version has to be newer than the Boost version: see https://stackoverflow.com/a/43885372/10360134
# BE SUPER CAREFUL IF YOU CHANGE ANY VERSION ~ Captain SpOCK, 45th August 3045
cmake_minimum_required(VERSION 3.23.2)

# CMake ctest does not allow parallel execution, so it has to become an option
OPTION(BUILD_PARALLEL "Enables parallel execution with MPI" OFF) # Disabled by default
IF(BUILD_PARALLEL)
    ADD_DEFINITIONS(-BUILD_PARALLEL)
ENDIF(BUILD_PARALLEL)
message(STATUS "Parallel build is: ${PARALLEL}. Enable/Disable MPI with -D BUILD_PARALLEL=ON/OFF")

# Project name and language
project(SpOCK LANGUAGES CXX)

# We default to Release build type if option -D CMAKE_BUILD_TYPE not provided
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
                        zlib/1.2.12
                        mp-units/0.7.0
                        boost/1.79.0
                        openmpi/4.1.0
                        gsl/2.7
                        cspice/0067
                      GENERATORS
                        cmake
                        # For cspice
                        CMakeDeps
                        CMakeToolchain
                      )

#  Detect settings like OS and architecture
# I think it also detects CMake settings like gcc, gcc-version; cppstd, build_type etc
conan_cmake_autodetect(settings)

# Since GCC >= 5, the compiler is likely to be using the new CXX11 ABI by default (libstdc++11)
# See https://docs.conan.io/en/latest/howtos/manage_gcc_abi.html
conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS
                      ${settings}
                      compiler.libcxx=libstdc++11
                      compiler.cppstd=20
                    )

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_subdirectory(src)

add_subdirectory(doc)

enable_testing()

add_subdirectory(tests)
