#include <iostream>
#include <boost/filesystem.hpp>

#include "MySqlJdbcConnector.hpp"


MySqlJdbcConnector::MySqlJdbcConnector() :
	m_driver{ sql::mysql::get_driver_instance() }
{
	;
}


bool MySqlJdbcConnector::connect(const Configuration &config)
{
	try
	{
		m_connection = std::unique_ptr<sql::Connection>(m_driver->connect(config.getDbUrl(), config.getDbUser(), config.getDbPassword()));
		m_connection->setSchema(config.getDbName());
		return true;
	}
	catch (sql::SQLException &e)
	{
		std::cerr << "[MysqlConnector] " << e.what() << "\n";
		return false;
	}

	return true;
}


bool MySqlJdbcConnector::tryReconnect()
{
	if (!m_connection->isValid())
	{
		return m_connection->reconnect();
	}

	return true;
}


std::unique_ptr<sql::Statement> MySqlJdbcConnector::createStatement()
{
	return std::unique_ptr<sql::Statement>(m_connection->createStatement());
}


std::unique_ptr<sql::PreparedStatement> MySqlJdbcConnector::prepareStatement(const std::string &statement)
{
	return std::unique_ptr<sql::PreparedStatement>(m_connection->prepareStatement(statement));
}


