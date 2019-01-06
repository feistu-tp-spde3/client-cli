#include "AgentManager.hpp"
#include "CmdLine.hpp"


int main(int argc, char **argv)
{
	AgentManager manager(8888, 9999);

	// Search agents in the network via UDP broadcast on port 8888
	manager.discoverAgents();

	// Run a TCP server on port 9999 so the agents can connect to it
	manager.run();
	
	// prikazovy riadok na ovladanie agentov
	CmdLine cmd;
	cmd.run(manager);

	manager.join();
	cmd.join();

	return 0;
}