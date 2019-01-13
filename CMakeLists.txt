project (Client)

cmake_minimum_required (VERSION 2.8.6)
set (Client_VERSION_MAJOR 1)
set (Client_VERSION_MINOR 0)

set (TARGET_NAME ClientBin)

file (GLOB CPP_FILES src/*.cpp)
file (GLOB HPP_FILES src/*.hpp)

set (SOURCE_FILES ${CPP_FILES} ${HPP_FILES})

if (EXISTS /usr/lib/x86_64-linux-gnu/libmysqlcppconn.so)
    set (LIBS /usr/lib/x86_64-linux-gnu/libmysqlcppconn.so)
else ()
    set (LIBS /usr/lib/i386-linux-gnu/libmysqlcppconn.so)
endif ()

set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME OFF)

# Set to TRUE if you custom-compiled Boost and then change BOOST_ROOT
set (Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
    set (BOOST_ROOT "/home/user/boost_1_68_0")
    set (BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
    set (BOOST_LIBRARY_DIRS "${BOOST_ROOT}/stage/lib")
endif (Boost_NO_SYSTEM_PATHS)

find_package(Boost 1.68.0 COMPONENTS system thread chrono filesystem)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} ${LIBS} ${Boost_LIBRARIES})

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
    target_compile_options(${TARGET_NAME} PRIVATE -std=c++11)
endif ()
