#include "AgentManager.hpp"


AgentManager::AgentManager(uint16_t discover_port, uint16_t server_port) :
	m_discover_port{ discover_port },
	m_server_port{ server_port }
{
	m_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), server_port));
}


void AgentManager::discoverAgents()
{
	std::cout << "[AgentManager] Searching for agents" << std::endl;

	boost::asio::io_service io_service;
	boost::asio::ip::udp::socket udpSocket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));

	udpSocket.set_option(boost::asio::socket_base::broadcast(true));

	boost::asio::ip::udp::endpoint broadcastEndpoint(boost::asio::ip::address_v4::broadcast(), m_discover_port);

	// pridanie portu do spravy aby sa klient vedel spravne pripojit
	std::string discover_msg = "agentSearch/" + std::to_string(m_server_port);
	std::cout << "[AgentManager] Broadcasting message: " << discover_msg << std::endl;

	try
	{
		udpSocket.send_to(boost::asio::buffer(discover_msg.c_str(), discover_msg.size()), broadcastEndpoint);
	}
	catch (std::exception &e)
	{
		std::cout << "[AgentManager] Exception: " << e.what() << std::endl;
	}

	udpSocket.close();
}


void AgentManager::run()
{
	m_main_thread = std::thread([this]()
	{
		std::cout << "[AgentManager] Listening on port " << m_server_port << std::endl;

		while (true)
		{
			// cakanie na spojenie
			std::unique_ptr<boost::asio::ip::tcp::socket> conn = std::make_unique<boost::asio::ip::tcp::socket>(m_io_service);
			m_acceptor->accept(*conn);

			// prijatie identifikacnej spravy
			try
			{
				boost::system::error_code ec;
				char buffer[MAX_BUFFER_SIZE] = { 0 };
				size_t n_received = conn->read_some(boost::asio::buffer(buffer), ec);

				if (n_received)
				{
					std::string agent(buffer, n_received);
					std::cout << "[AgentManager] Establishing tcp connection with agent \"" << agent << "\"\n";

					this->addConnection(agent, std::move(conn));
				}
			}
			catch (boost::system::system_error &e)
			{
				std::cout << "[AgentManager] Failed to receive message: " << e.what() << "\n";
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


bool AgentManager::sendMessage(const std::string &agent, const std::string &msg)
{
	auto find = m_connections.find(agent);
	if (find == m_connections.end())
	{
		std::cout << "[AgentManager] Agent: " << agent << " not found!\n";
		return false;
	}

	try
	{
		std::cout << "[AgentManager] Sending message to agent(" << agent << "): " << msg << "\n";

		boost::system::error_code ec;
		size_t n_sent = boost::asio::write(*find->second, boost::asio::buffer(msg), ec);
		if (ec == boost::asio::error::eof)
		{
			std::cout << "[AgentManager] Client dropped\n";
			return false;
		}

		if (!n_sent)
		{
			return false;
		}

		std::cout << "[AgentManager] Message sent!\n";
		return true;
	}
	catch (boost::system::system_error &e)
	{
		std::cout << "[AgentManager] Failed to send message: " << e.what() << "\n";
		return false;
	}
}


bool AgentManager::recvMessage(const std::string &agent, std::string &out)
{
	auto find = m_connections.find(agent);
	if (find == m_connections.end())
	{
		std::cout << "[AgentManager] Agent: " << agent << " not found!" << std::endl;
		return false;
	}

	boost::system::error_code error;
	char buffer[MAX_BUFFER_SIZE] = { 0 };

	try
	{
		size_t n_received = find->second->read_some(boost::asio::buffer(buffer), error);
		if (n_received)
		{
			out.assign(buffer, n_received);
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (boost::system::system_error &e)
	{
		std::cout << "[AgentManager] Failed to receive message: " << e.what() << std::endl;
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