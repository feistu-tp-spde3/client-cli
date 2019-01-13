# monitor 
Tool to monitor agents on a network

## New features
- Two-way communication with agent (previouly: monitor -> agent)
- Communication format is JSON (previously: basic text)
- Added "discover" command to discover agents on a network (previously this was done only on startup)
- Added "list" command that lists connected agents & checks their online status (via ping/pong messages)
- Added "filter get" command to retrieve current agent filter
- "filter set" command informs the user if the filter was installed or not
- Added "proc" command to get, add and remove monitored processes on agent (get checks their status: running/not running)
- Connection to Mysql database
- Adding/updating agents and their statuses in DB

## Build

You will need these external packages to build Monitor:
- boost
- https://dev.mysql.com/downloads/connector/cpp/8.0.html (NOTE: if you are using Debian, don't install from repo!)

### Linux

If you compiled Boost yourself, change this path in `CMakeLists.txt`:
```
set (BOOST_ROOT "/home/user/boost_1_68_0")
```

If you didn't compile Boost yourself, change this line to FALSE:
```
# Set to TRUE if you custom-compiled Boost and then change BOOST_ROOT
set (Boost_NO_SYSTEM_PATHS TRUE)
```

```
cmake .
make
```

### Windows

1. Download/install Boost
2. Download https://dev.mysql.com/downloads/connector/cpp/8.0.html
3. Add environment variables `BOOST_INCLUDE_PATH`, `BOOST_LIB_PATH`, sample values:
```
BOOST_INCLUDE_PATH=C:\boost_1_68_0_32
BOOST_LIB_PATH=C:\boost_1_68_0_32\lib32-msvc-14.1
```
4. Add environment variables `MYSQL_CONNECTOR_INCLUDE_PATH`, `MYSQL_CONNECTOR_LIB_PATH`, sample values:
```
MYSQL_CONNECTOR_INCLUDE_PATH=C:\mysql-connector-c++-8.0.13-win32\include
MYSQL_CONNECTOR_LIB_PATH=C:\mysql-connector-c++-8.0.13-win32\lib\vs14
```
5. Note that to actually run the aplication you will need these DLLs in the same directory as the .exe:
```
mysqlcppconn-7-vs14.dll
ssleay32.dll
libeay32.dll
```
