#pragma once

#include <boost/thread.hpp>

#include <iostream>
#include <vector>
#include <string>


class AgentManager;


class CmdLine
{
private:
	AgentManager &m_manager;
	boost::thread m_main_thread;

	// Handle commands
	bool cmd_start(const std::string &agent, const std::vector<std::string> &tokens);
	bool cmd_stop(const std::string &agent, const std::vector<std::string> &tokens);
	bool cmd_filter(const std::string &agent, const std::vector<std::string> &tokens);
	bool cmd_proc(const std::string &agent, const std::vector<std::string> &tokens);

	static const std::string HELP_USAGE;

public:
	CmdLine(AgentManager &manager);

	void run();
	void join();
};
