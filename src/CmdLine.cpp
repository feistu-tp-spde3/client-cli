#include "CmdLine.hpp"
#include "AgentManager.hpp"


const std::string CmdLine::HELP_USAGE = "\
Help:\n\
discover -> discover agents on network\n\
list -> list of all connected agents (checks if alive)\n\
stop [agent] -> stop agent\n\
start [agent] -> start agent\n\
filter [agent] [filter] -> change filter on agent\n\
";


CmdLine::CmdLine()
{
	;
}


void CmdLine::run(AgentManager &manager)
{
	m_main_thread = std::thread([this, &manager]()
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
				if (!manager.isConnected(agent))
				{
					std::cout << "Agent \"" << agent << "\" is not connected\n";
					continue;
				}
			}

			if (cmd == "help")
			{
				std::cout << CmdLine::HELP_USAGE;
				continue;
			}
			else if (cmd == "discover")
			{
				manager.discoverAgents();
			}
			else if (cmd == "list")
			{
				// Check if agents are actually connected first
				manager.refresh();

				std::vector<std::string> agents = manager.getAgents();

				int c = 1;
				for (auto &agent : agents)
				{
					std::cout << c << ". " << agent << "\n";
					c++;
				}

				continue;
			}
			else if (cmd == "stop")
			{
				if (tokens.size() < 2)
				{
					std::cout << "Invalid stop command syntax\n";
					continue;
				}

				manager.sendMessage(agent, "stop");
				continue;
			}
			else if (cmd == "start")
			{
				if (tokens.size() < 2)
				{
					std::cout << "Invalid start command syntax\n";
					continue;
				}

				manager.sendMessage(agent, "start");
				continue;
			}
			else if (cmd == "filter")
			{
				if (tokens.size() < 3)
				{
					std::cout << "Invalid filter command syntax\n";
					continue;
				}

				std::string filter;
				for (size_t i = 2; i < tokens.size(); i++)
				{
					filter += tokens.at(i);
					filter += " ";
				}

				manager.sendMessage(agent, "filter//" + filter);
				continue;
			}
			else {
				std::cout << "Wrong command\n";
				continue;
			}
		}
	});
}


void CmdLine::join()
{
	m_main_thread.join();
}