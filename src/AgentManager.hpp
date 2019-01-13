#pragma once

#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "json.hpp"


using json = nlohmann::json;


class AgentManager
{
private:
	std::thread m_main_thread;

	uint16_t m_discover_port;
	uint16_t m_server_port;

	std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
	boost::asio::io_service m_io_service;

	std::map<std::string, std::unique_ptr<boost::asio::ip::tcp::socket>> m_connections;

	static const int MAX_BUFFER_SIZE{ 1024 };

public:
	AgentManager(uint16_t discover_port, uint16_t server_port);

	void discoverAgents();

	void run();
	void join();
	
	void addConnection(const std::string &name, std::unique_ptr<boost::asio::ip::tcp::socket> conn);
	bool sendMessage(const std::string &agent, const std::string &msg);
	bool recvMessage(const std::string &agent, json &out);

	std::vector<std::string> getAgents() const;
	bool isConnected(const std::string &agent) const;
	void refresh();
	bool ping(const std::string &agent);
};
