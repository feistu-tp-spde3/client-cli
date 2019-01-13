#include "CmdLine.hpp"
#include "AgentManager.hpp"
#include "json.hpp"


using json = nlohmann::json;


const std::string CmdLine::HELP_USAGE = "\
Help:\n\
discover -> discover agents on network\n\
list -> list of all connected agents (checks if alive)\n\
stop <agent> -> stop agent\n\
start <agent> -> start agent\n\
filter <agent> get|set <filter> -> get/set filter on agent\n\
proc <agent> get|add <process>|del <process> -> manipulate monitored processes on agent\n\
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
					std::cout << c << ". " << agent << " (" << m_manager.getAgentIp(agent) << ")\n";
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

				json d;
				d["cmd"] = "stop";
				d["action"] = "";
				d["data"] = "";

				m_manager.sendMessage(agent, d.dump());
			}
			else if (cmd == "start")
			{
				if (tokens.size() < 2)
				{
					std::cerr << "Invalid start command syntax\n";
					continue;
				}

				json d;
				d["cmd"] = "start";
				d["action"] = "";
				d["data"] = "";

				m_manager.sendMessage(agent, d.dump());
			}
			else if (cmd == "filter")
			{
				cmd_filter(agent, tokens);
			}
			else if (cmd == "proc")
			{
				cmd_proc(agent, tokens);
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
		json msg;
		msg["cmd"] = "filter";
		msg["action"] = action;
		msg["data"] = "";

		if (!m_manager.sendMessage(agent, msg.dump()))
		{
			std::cerr << "Failed to send get filter command to agent \"" << agent << "\"\n";
			return false;
		}

		json response;
		if (!m_manager.recvMessage(agent, response))
		{
			std::cerr << "Failed to receive filter from agent \"" << agent << "\"\n";
			return false;
		}

		std::cout << "Current agent \"" << agent << "\" filter: \"" << response["response"] << "\"\n";
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

		json msg;
		msg["cmd"] = "filter";
		msg["action"] = action;
		msg["data"] = filter;

		if (!m_manager.sendMessage(agent, msg.dump()))
		{
			std::cerr << "Failed to send filter to agent " << agent << "\n";
			return false;
		}

		json response;
		if (!m_manager.recvMessage(agent, response))
		{
			std::cerr << "Failed to receive response to filter change from agent " << agent << "\n";
			return false;
		}

		if (response["response"] == "ok")
		{
			std::cout << "Filter changed\n";
			return true;
		}
		else
		{
			std::cerr << "Failed to change filter\n";
			return false;
		}
	}
	else
	{
		std::cerr << "Unknown filter action: \"" << action << "\"\n";
		return false;
	}

	return true;
}


bool CmdLine::cmd_proc(const std::string &agent, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 3)
	{
		std::cerr << "Invalid proc command syntax, check help\n";
		return false;
	}

	const std::string &action = tokens.at(2);
	if (action == "get")
	{
		json msg;
		msg["cmd"] = "proc";
		msg["action"] = "get";
		msg["data"] = "";

		if (!m_manager.sendMessage(agent, msg.dump()))
		{
			std::cerr << "Failed to send request to get monitored processes from agent\n";
			return false;
		}

		bool ret = false;
		json response;
		if (!(ret = m_manager.recvMessage(agent, response)))
		{
			std::cerr << "Failed to get monitored processes from agent\n";
			return false;
		}

		for (auto &el : response["response"].items())
		{
			std::cout << "Process: \"" << el.key() << "\": " << (el.value() ? "running" : "not running") << "\n";
		}
		
		return ret;
	}
	else if (action == "add")
	{
		if (tokens.size() < 4)
		{
			std::cerr << "Invalid proc command syntax\n";
			return false;
		}

		const std::string &process = tokens.at(3);

		json msg;
		msg["cmd"] = "proc";
		msg["action"] = "add";
		msg["data"] = process;

		if (!m_manager.sendMessage(agent, msg.dump()))
		{
			std::cerr << "Failed to send request to add a monitored process\n";
			return false;
		}

		json response;
		if (!m_manager.recvMessage(agent, response))
		{
			std::cerr << "Failed to receive response to process add\n";
			return false;
		}

		if (response["response"] == "ok")
		{
			std::cout << "Monitored process added\n";
			return true;
		}
		else
		{
			std::cerr << "Failed to add monitored process\n";
			return false;
		}
	}
	else if (action == "del")
	{
		if (tokens.size() < 4)
		{
			std::cerr << "Invalid proc command syntax\n";
			return false;
		}

		const std::string &process = tokens.at(3);

		json msg;
		msg["cmd"] = "proc";
		msg["action"] = "del";
		msg["data"] = process;

		if (!m_manager.sendMessage(agent, msg.dump()))
		{
			std::cerr << "Failed to send request to remove a monitored process\n";
			return false;
		}

		json response;
		if (!m_manager.recvMessage(agent, response))
		{
			std::cerr << "Failed to receive response to process remove\n";
			return false;
		}

		if (response["response"] == "ok")
		{
			std::cout << "Monitored process removed\n";
			return true;
		}
		else
		{
			std::cerr << "Failed to remove monitored process\n";
			return false;
		}
	}

	return true;
}