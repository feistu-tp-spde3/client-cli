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
	std::thread m_main_thread;

	// Handle commands
	bool cmd_filter(const std::string &agent, const std::vector<std::string> &tokens);
	bool cmd_proc(const std::string &agent, const std::vector<std::string> &tokens);

	static const std::string HELP_USAGE;

public:
	CmdLine(AgentManager &manager);

	void run();
	void join();
};
