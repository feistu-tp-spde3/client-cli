#include "AgentManager.hpp"
#include "json.hpp"


using json = nlohmann::json;


AgentManager::AgentManager(uint16_t discover_port, uint16_t server_port) :
	m_discover_port{ discover_port },
	m_server_port{ server_port }
{
	m_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), server_port));
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

					addConnection(agent, std::move(conn));
				}
			}
			catch (boost::system::system_error &e)
			{
				std::cerr << "[AgentManager] Failed to receive message: " << e.what() << "\n";
			}
		}
	});
}


void AgentManager::join()
{
	m_main_thread.join();
}


void AgentManager::addConnection(const std::string &name, std::unique_ptr<boost::asio::ip::tcp::socket> conn)
{
	m_connections[name] = std::move(conn);
}


void AgentManager::refresh()
{
	// https://stackoverflow.com/questions/8234779/how-to-remove-from-a-map-while-iterating-it
	for (auto it = m_connections.cbegin(); it != m_connections.cend() ; )
	{
		const std::string &agent = (*it).first;
		if (!ping(agent))
		{
			std::cout << "Agent \"" << agent << "\" not responding\n";
			m_connections.erase(it++);
		}
		else
		{
			++it;
		}
	}
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

			// This is important
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


std::vector<std::string> AgentManager::getAgents() const
{
	std::vector<std::string> agents;

	for (auto &conn : m_connections)
	{
		agents.push_back(conn.first);
	}

	return agents;
}


bool AgentManager::isConnected(const std::string &agent) const
{
	return m_connections.count(agent);
}