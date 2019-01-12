# monitor 
Tool to monitor agents on a network

## New features
- Two-way communication with agent
- Added "discover" command to discover agents on a network (previously this was done only on startup)
- Added "list" command that lists connected agents & checks their online status (via ping/pong messages)
- Get/set filter from/on agent

## Build

cmake .
make
