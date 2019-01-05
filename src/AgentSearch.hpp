#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <exception>


class AgentSearch
{
private:
	// sprava ktora sa broadcastuje
	std::string m_message{ "agentSearch" };

	static const uint16_t PORT{ 9999 };

public:
	// spustenie vyhladavanie agentov na sieti
	void run(int tcpPort);
};
