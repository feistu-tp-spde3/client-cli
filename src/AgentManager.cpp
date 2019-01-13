#include <chrono>

#include "AgentManager.hpp"
#include "json.hpp"


using json = nlohmann::json;


AgentManager::AgentManager(uint16_t discover_port, uint16_t server_port) :
	m_discover_port{ discover_port },
	m_server_port{ server_port },
	m_db{ MySqlJdbcConnector() }
{
	;
}


bool AgentManager::connectToDb(const std::string &xml_db_config)
{
	if (!m_db.connect(xml_db_config))
	{
		std::cerr << "[AgentManager] Couldn't connect to Mysql database\n";
		return false;
	}

	std::cout << "[AgentManager] Connected to Mysql database\n";
	return true;
}


void AgentManager::discoverAgents()
{
	std::cout << "[AgentManager] Searching for agents" << std::endl;

	boost::asio::ip::udp::socket udp_socket(m_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));
	udp_socket.set_option(boost::asio::socket_base::broadcast(true));

	boost::asio::ip::udp::endpoint broadcast_endpoint(boost::asio::ip::address_v4::broadcast(), m_discover_port);

	std::string discover_msg = "agentSearch/" + std::to_string(m_server_port);
	std::cout << "[AgentManager] Broadcasting message: \"" << discover_msg << "\"\n";

	try
	{
		udp_socket.send_to(boost::asio::buffer(discover_msg.c_str(), discover_msg.size()), broadcast_endpoint);
	}
	catch (std::exception &e)
	{
		std::cerr << "[AgentManager] Exception: " << e.what() << std::endl;
	}

	udp_socket.close();
}


void AgentManager::run()
{
	m_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_server_port));

	m_main_thread = std::thread([this]()
	{
		std::cout << "[AgentManager] Listening on port " << m_server_port << "\n";

		while (true)
		{
			std::unique_ptr<boost::asio::ip::tcp::socket> conn = std::make_unique<boost::asio::ip::tcp::socket>(m_io_service);
			m_acceptor->accept(*conn);

			// Receive identification message from the agent
			try
			{
				boost::system::error_code ec;
				char buffer[MAX_BUFFER_SIZE] = { 0 };
				size_t n_received = conn->read_some(boost::asio::buffer(buffer), ec);

				if (n_received)
				{
					std::string agent(buffer, n_received);
					std::cout << "[AgentManager] Establishing tcp connection with agent \"" << agent << "\"\n";

					m_control_mutex.lock();
					addConnection(agent, std::move(conn));
					m_control_mutex.unlock();

					addAgentToDb(agent);
				}
			}
			catch (sql::SQLException &e)
			{
				std::cerr << "[AgentManager] Failed to add agent to DB: " << e.what() << "\n";
			}
			catch (boost::system::system_error &e)
			{
				std::cerr << "[AgentManager] Failed to receive message: " << e.what() << "\n";
			}
		}
	});

	std::thread checking_thread = std::thread([this]()
	{
		while (true)
		{
			// Not using mutex here because refresh does that
			refresh();
			std::this_thread::sleep_for(std::chrono::seconds(AGENT_REFRESH_INTERVAL));
		}
	});

	checking_thread.detach();
}


void AgentManager::join()
{
	m_main_thread.join();
}


bool AgentManager::ping(const std::string &agent)
{
	json request;
	request["cmd"] = "ping";
	request["action"] = "";
	request["data"] = "";

	if (!sendMessage(agent, request.dump()))
	{
		return false;
	}

	json response;
	if (!recvMessage(agent, response))
	{
		return false;
	}

	return response["response"] == "pong";
}


void AgentManager::refresh()
{
	m_control_mutex.lock();

	// https://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
	for (auto it = m_connections.cbegin(); it != m_connections.cend(); )
	{
		// Copy by value because we delete the pair from the map in one if
		std::string agent = (*it).first;

		try
		{
			bool running = true;
			if (!ping(agent))
			{
				running = false;
				m_connections.erase(it++);
			}
			else
			{

				++it;
			}

			if (!updateAgentStatus(agent, (running ? "running" : "not running")))
			{
				std::cerr << "[AgentManager] Failed to update agent \"" << agent << "\" status\n";
			}
		}
		catch (sql::SQLException &e)
		{
			std::cerr << "[AgentManager] SQL error while updating status: " << e.what() << "\n";
		}
	}

	m_control_mutex.unlock();
}


bool AgentManager::sendMessage(const std::string &agent, const std::string &msg)
{
	auto find = m_connections.find(agent);
	if (find == m_connections.end())
	{
		std::cerr << "[AgentManager] Agent: " << agent << " not found!\n";
		return false;
	}

	try
	{
		boost::system::error_code ec;
		return boost::asio::write(*find->second, boost::asio::buffer(msg), ec);
	}
	catch (boost::system::system_error &e)
	{
		std::cerr << "[AgentManager] Failed to send message: " << e.what() << "\n";
		return false;
	}
}


bool AgentManager::recvMessage(const std::string &agent, json &out)
{
	auto find = m_connections.find(agent);
	if (find == m_connections.end())
	{
		std::cerr << "[AgentManager] Agent: " << agent << " not found!" << std::endl;
		return false;
	}

	try
	{
		boost::system::error_code error;
		char buffer[MAX_BUFFER_SIZE] = { 0 };

		size_t no_received = find->second->read_some(boost::asio::buffer(buffer), error);
		if (no_received)
		{
			std::string outbuf(buffer, no_received);
			out = json::parse(outbuf);

			// Every received message that doesn't contain "response" key is invalid
			if (!out.count("response"))
			{
				return false;
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	catch (json::exception &e)
	{
		std::cerr << "[AgentManager] Failed to parse message with JSON: " << e.what() << "\n";
		return false;
	}
	catch (boost::system::system_error &e)
	{
		std::cerr << "[AgentManager] Failed to receive message: " << e.what() << "\n";
		return false;
	}
}


void AgentManager::addAgentToDb(const std::string &agent)
{
	auto stat = m_db.prepareStatement("SELECT id FROM agents WHERE name = ?");
	stat->setString(1, agent);

	std::unique_ptr<sql::ResultSet> res(stat->executeQuery());
	if (!res->first())
	{
		auto insert = m_db.prepareStatement("INSERT INTO agents (name, ip, status) VALUES (?, ?, ?)");
		insert->setString(1, agent);
		insert->setString(2, getAgentIp(agent));
		insert->setString(3, "running");
		insert->execute();
	}
	else
	{
		auto update = m_db.prepareStatement("UPDATE agents SET last_updated = now() WHERE id = ?");
		update->setInt(1, res->getInt("id"));
		update->execute();
	}
}


bool AgentManager::updateAgentStatus(const std::string &agent, const std::string &status)
{
	auto stat = m_db.prepareStatement("SELECT id FROM agents WHERE name = ?");
	stat->setString(1, agent);

	std::unique_ptr<sql::ResultSet> res(stat->executeQuery());
	if (!res->first())
	{
		return false;
	}

	auto update = m_db.prepareStatement("UPDATE agents SET last_updated = now(), status = ? WHERE id = ?");
	update->setString(1, status);
	update->setInt(2, res->getInt("id"));

	// ->execute() actually returns 0 on success and 1 on fail, nice library
	return !update->execute();
}


void AgentManager::addConnection(const std::string &agent, std::unique_ptr<boost::asio::ip::tcp::socket> conn)
{
	m_connections[agent] = std::move(conn);
}


std::vector<std::string> AgentManager::getAgents() const
{
	std::vector<std::string> agents;

	for (auto &conn : m_connections)
	{
		agents.push_back(conn.first);
	}

	return agents;
}