#include "CmdLine.hpp"
#include "AgentManager.hpp"


const std::string CmdLine::HELP_USAGE = "\
Help:\n\
discover -> discover agents on network\n\
list -> list of all connected agents (checks if alive)\n\
stop <agent> -> stop agent\n\
start <agent> -> start agent\n\
filter <agent> [set|get] [filter] -> get/set filter on agent\n\
";


CmdLine::CmdLine(AgentManager &manager) :
	m_manager{ manager }
{
	;
}


void CmdLine::run()
{
	m_main_thread = std::thread([this]()
	{
		std::cout << "[CmdLine] Starting command line\n";

		while (true)
		{
			std::string input;
			std::cout << "> ";
			std::getline(std::cin, input);

			// rozsekanie vstupu
			std::size_t pos = 0;
			std::vector<std::string> tokens;
			std::string delimiter = " ";

			while ((pos = input.find(delimiter)) != std::string::npos)
			{
				std::string token = input.substr(0, pos);
				tokens.push_back(token);
				input.erase(0, pos + delimiter.length());
			}

			tokens.push_back(input);

			std::string cmd = tokens.at(0);
			// We assume the 1st argument is always agent name
			std::string agent;

			if (tokens.size() > 1)
			{
				agent = tokens.at(1);

				// We check if the agent is connected so we can bail early and dont check it on every command
				if (!m_manager.isConnected(agent))
				{
					std::cerr << "Agent \"" << agent << "\" is not connected\n";
					continue;
				}
			}

			if (cmd == "help")
			{
				std::cout << CmdLine::HELP_USAGE;
			}
			else if (cmd == "discover")
			{
				m_manager.discoverAgents();
			}
			else if (cmd == "list")
			{
				// Check if agents are actually connected first
				m_manager.refresh();

				std::vector<std::string> agents = m_manager.getAgents();

				int c = 1;
				for (auto &agent : agents)
				{
					std::cout << c << ". " << agent << "\n";
					c++;
				}
			}
			else if (cmd == "stop")
			{
				if (tokens.size() < 2)
				{
					std::cerr << "Invalid stop command syntax\n";
					continue;
				}

				m_manager.sendMessage(agent, "stop");
			}
			else if (cmd == "start")
			{
				if (tokens.size() < 2)
				{
					std::cerr << "Invalid start command syntax\n";
					continue;
				}

				m_manager.sendMessage(agent, "start");
			}
			else if (cmd == "filter")
			{
				cmd_filter(agent, tokens);
			}
			else {
				std::cerr << "Wrong command\n";
			}
		}
	});
}


void CmdLine::join()
{
	m_main_thread.join();
}


bool CmdLine::cmd_filter(const std::string &agent, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 3)
	{
		std::cerr << "Invalid filter command syntax, check help\n";
		return false;
	}

	const std::string &action = tokens.at(2);
	if (action == "get")
	{
		if (!m_manager.sendMessage(agent, "filter"))
		{
			std::cerr << "Failed to send get filter command to agent \"" << agent << "\"\n";
			return false;
		}

		std::string response;
		if (!m_manager.recvMessage(agent, response))
		{
			std::cerr << "Failed to receive filter from agent \"" << agent << "\"\n";
			return false;
		}

		std::cout << "Current agent \"" << agent << "\" filter: \"" << response << "\"\n";
	}
	else if (action == "set")
	{
		if (tokens.size() < 4)
		{
			std::cerr << "No filter to set entered\n";
			return false;
		}

		std::string filter;
		for (size_t i = 3; i < tokens.size(); i++)
		{
			filter += tokens.at(i);

			if (i != tokens.size() - 1)
			{
				filter += " ";
			}
		}

		if (!m_manager.sendMessage(agent, "filter//" + filter))
		{
			std::cerr << "Failed to send filter to agent " << agent << "\n";
			return false;
		}

		std::string response;
		if (!m_manager.recvMessage(agent, response))
		{
			std::cerr << "Failed to receive response to filter change from agent " << agent << "\n";
			return false;
		}

		if (response == "ok")
		{
			std::cout << "Filter changed\n";
		}
		else
		{
			std::cerr << "Failed to change filter\n";
		}
	}
	else
	{
		std::cerr << "Unknown filter action: \"" << action << "\"\n";
		return false;
	}

	return true;
}