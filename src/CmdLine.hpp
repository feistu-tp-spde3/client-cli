#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <thread>


class AgentManager;


class CmdLine
{
private:
	// hlavne vlakno 
	std::thread m_main_thread;

	static const std::string HELP_USAGE;

public:
	CmdLine();

	// spustenie prikazoveho riadku
	void run(AgentManager &manager);
	void join();
};
