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

## Build

cmake .
make
