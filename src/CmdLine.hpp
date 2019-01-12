#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <thread>


class AgentManager;


class CmdLine
{
private:
	AgentManager &m_manager;

	// hlavne vlakno 
	std::thread m_main_thread;

	// Handle the "filter" command
	bool cmd_filter(const std::string &agent, const std::vector<std::string> &tokens);

	static const std::string HELP_USAGE;

public:
	CmdLine(AgentManager &manager);

	// spustenie prikazoveho riadku
	void run();
	void join();
};
