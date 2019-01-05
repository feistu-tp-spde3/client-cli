#include "CmdLine.hpp"
#include "AgentManager.hpp"


const std::string CmdLine::HELP_USAGE = "\
Help:\n\
discover -> discover agents\n\
list -> list of all connected agents\n\
stop [agent] -> stops agent activity\n\
start [agent] -> starts agent activity\n\
filter [agent] [filter] -> resets windivert filter on agent\n\
";


void CmdLine::run(AgentManager &manager)
{
	std::cout << "[CmdLine] Starting command line" << std::endl;

	m_main_thread = std::thread([this, &manager]()
	{
		while (true)
		{
			std::string cmd;
			std::cout << "> ";
			std::getline(std::cin, cmd);

			// rozsekanie vstupu
			std::size_t pos = 0;
			std::vector<std::string> tokens;
			std::string delimiter = " ";

			while ((pos = cmd.find(delimiter)) != std::string::npos)
			{
				std::string  token = cmd.substr(0, pos);
				tokens.push_back(token);
				cmd.erase(0, pos + delimiter.length());
			}

			tokens.push_back(cmd);

			if (tokens.at(0) == "help")
			{
				std::cout << CmdLine::HELP_USAGE;
				continue;
			}
			else if (tokens.at(0) == "discover")
			{
				manager.discoverAgents();
			}
			else if (tokens.at(0) == "list")
			{
				std::vector<std::string> agents = manager.getAgents();

				int c = 1;
				for (auto &agent : agents)
				{
					std::cout << c << ". " << agent << std::endl;
					c++;
				}

				continue;
			}
			else if (tokens.at(0) == "stop")
			{
				std::string agent = tokens.at(1);

				auto agents = manager.getAgents();
				for (auto &a : agents)
				{
					if (a == agent)
					{
						manager.sendMessage(agent, "stop");
					}
				}

				continue;
			}
			else if (tokens.at(0) == "start")
			{
				std::string agent = tokens.at(1);

				auto agents = manager.getAgents();
				for (auto &a : agents)
				{
					if (a == agent)
					{
						manager.sendMessage(agent, "start");
					}
				}

				continue;
			}
			else if (tokens.at(0) == "filter")
			{
				std::string agent = tokens.at(1);
				std::string filter;

				for (std::size_t i = 2; i < tokens.size(); i++)
				{
					filter += tokens.at(i);
					filter += " ";
				}

				auto agents = manager.getAgents();
				for (auto &a : agents)
				{
					if (a == agent)
					{
						manager.sendMessage(agent, "filter//" + filter);
					}
				}

				continue;
			}
			else if (tokens.at(0) == "configlist")
			{
				std::string agent = tokens.at(1);

				auto agents = manager.getAgents();
				for (auto &a : agents)
				{
					if (a == agent)
					{
						manager.sendMessage(agent, "configlist");

						std::string response;
						if (manager.recvMessage(agent, response))
						{
							std::cout << response << "\n";
						}
					}
				}
			}
			else {
				std::cout << "> Wrong command" << std::endl;
				continue;
			}
		}
	});
}


void CmdLine::join()
{
	m_main_thread.join();
}