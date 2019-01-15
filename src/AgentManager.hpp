#pragma once

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <mutex>
#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "json.hpp"
#include "MySqlJdbcConnector.hpp"
#include "pugixml.hpp"
#include "Configuration.hpp"

using json = nlohmann::json;


class AgentManager
{
private:
	std::mutex m_control_mutex;
	boost::thread m_main_thread;

	uint16_t m_discover_port;
	uint16_t m_server_port;

    Configuration m_config;

	MySqlJdbcConnector m_db;

	std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
	boost::asio::io_service m_io_service;

	std::map<std::string, std::unique_ptr<boost::asio::ip::tcp::socket>> m_connections;

	// If agent with that name doesn't exist, create a new record
	// If it does exist, update last_updated
	void addAgentToDb(const std::string &agent);
	bool updateAgentStatus(const std::string &agent, int status);

	static const int MAX_BUFFER_SIZE{ 1024 };

public:
	AgentManager(uint16_t discover_port, uint16_t server_port);

	bool connectToDb();
	void discoverAgents();

    bool loadConfiguration(const std::string &xml_config);

	void run();
	void join();
	void lock() { m_control_mutex.lock(); }
	void unlock() { m_control_mutex.unlock(); }
	
	void refreshAgentStatuses();
	bool updateAgentProcesses(const std::string &agent, bool print = false);
	bool ping(const std::string &agent);

	bool sendMessage(const std::string &agent, const std::string &msg);
	bool recvMessage(const std::string &agent, json &out);
	
	bool isConnected(const std::string &agent) const { return m_connections.count(agent); }
	void addConnection(const std::string &agent, std::unique_ptr<boost::asio::ip::tcp::socket> conn);
	std::string getAgentIp(const std::string &agent) const { return m_connections.at(agent)->remote_endpoint().address().to_string(); }
	std::vector<std::string> getAgents() const;
};
