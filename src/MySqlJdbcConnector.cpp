#include "MySqlJdbcConnector.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

#include <boost/format.hpp>

#include "pugixml.hpp"


MySqlJdbcConnector::MySqlJdbcConnector() :
	m_driver{ sql::mysql::get_driver_instance() }
{
	;
}


bool MySqlJdbcConnector::connect(const std::string &xml_db_config)
{
	// TODO: move config parsing to AgentManager and make config more general
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file(xml_db_config.c_str());

    if (result.status != pugi::xml_parse_status::status_ok)
    {
		std::cerr << "[MysqlConnector] Could not parse configuration file \"" << xml_db_config << "\"\n";
		return false;
    }

    pugi::xml_node configuration = xml.child("Configuration");
	if (!configuration)
	{
		std::cerr << "[MysqlConnector] Invalid configuration\n";
		return false;
	}

	pugi::xml_node database = configuration.child("MysqlDatabase");
    if (!database)
    {
		std::cerr << "[MysqlConnector] Invalid configuration\n";
		return false;
    }
    
    std::string url, user, password, name;
    if (database.child("Url"))
    {
        url = database.child("Url").text().as_string();
    }

    if (database.child("User"))
    {
		user = database.child("User").text().as_string();
    }

    if (database.child("Password"))
    {
		password = database.child("Password").text().as_string();
    }

    if (database.child("Name"))
    {
		name = database.child("Name").text().as_string();
    }

	try
	{
		m_connection = std::unique_ptr<sql::Connection>(m_driver->connect(url, user, password));
		m_connection->setSchema(name);
		return true;
	}
	catch (sql::SQLException &e)
	{
		std::cerr << "[MysqlConnector] " << e.what() << "\n";
		return false;
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