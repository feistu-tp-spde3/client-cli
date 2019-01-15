#include "AgentManager.hpp"
#include "CmdLine.hpp"
#include "MySqlJdbcConnector.hpp"


int main(int argc, char **argv)
{
	AgentManager manager(8888, 9999);

    if (!manager.loadConfiguration("config_monitor.xml"))
    {
        return EXIT_FAILURE; 
    }

	// Connect to database
	if (!manager.connectToDb())
	{
		return EXIT_FAILURE;
	}

	// Search agents in the network via UDP broadcast on port 8888
	manager.discoverAgents();

	// Run a TCP server on port 9999 so the agents can connect to it
	manager.run();
	
	// prikazovy riadok na ovladanie agentov
	CmdLine cmd(manager);
	cmd.run();

	manager.join();
	cmd.join();

	return EXIT_SUCCESS;
}