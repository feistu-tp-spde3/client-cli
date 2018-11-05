#include "AgentSearch.hpp"

void AgentSearch::run(int tcpPort)
{
	std::cout << "[AgentSearch] Searching for agents" << std::endl;
        boost::asio::io_service io_service;
        boost::asio::ip::udp::socket udpSocket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0));

	udpSocket.set_option(boost::asio::socket_base::broadcast(true));

        boost::asio::ip::udp::endpoint broadcastEndpoint(boost::asio::ip::address_v4::broadcast(), 8888);
	
	// pridanie portu do spravy aby sa klient vedel spravne pripojit
	mMessage += "/";
	mMessage += std::to_string(tcpPort);

	std::cout << "[AgentSearch] Broadcasting message: " << mMessage << std::endl;

	try {
		udpSocket.send_to(boost::asio::buffer(mMessage.c_str(), mMessage.size()), broadcastEndpoint);
	}
	catch (std::exception &e)
	{
		std::cout << "[AgentSearch] Exception: " << e.what() << std::endl;
	}
	udpSocket.close();
}
