project (Client)

cmake_minimum_required (VERSION 2.8.6)
set (Client_VERSION_MAJOR 1)
set (Client_VERSION_MINOR 0)

set (TARGET_NAME Agent)

file (GLOB CPP_FILES src/*.cpp)
file (GLOB HPP_FILES src/*.hpp)

set (SOURCE_FILES ${CPP_FILES} ${HPP_FILES})

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

find_package(Boost 1.68.0 COMPONENTS system thread)

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif (Boost_FOUND)

add_executable(ClientBin ${SOURCE_FILES})
target_link_libraries(ClientBin ${LIBS} ${Boost_LIBRARIES})
