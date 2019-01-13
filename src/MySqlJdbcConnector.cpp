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


int MySqlJdbcConnector::insert(const std::string &table_name, const std::vector<std::string> &values, bool auto_increment)
{
	std::vector<std::string> columns = getColumns(table_name, auto_increment);

	std::shared_ptr<sql::PreparedStatement> statement(
		m_connection->prepareStatement(formatInsertStatement(table_name, columns)));

	for (size_t i = 1; i <= values.size(); i++)
	{
		statement->setString(i, values[i - 1]);
	}

	return statement->executeUpdate();
}


std::vector<std::string> MySqlJdbcConnector::getColumns(const std::string &table_name, bool auto_increment)
{
	std::unique_ptr<sql::PreparedStatement> columns_statement(
		m_connection->prepareStatement("SELECT column_name FROM information_schema.columns WHERE table_name = ?"));

	// Determine which column to ignore because it's a primary key
	std::unique_ptr<sql::PreparedStatement> pkey_statement(
		m_connection->prepareStatement("SELECT k.COLUMN_NAME FROM information_schema.table_constraints t LEFT JOIN information_schema.key_column_usage k USING(constraint_name, table_schema, table_name) WHERE t.constraint_type = 'PRIMARY KEY' AND t.table_schema = DATABASE() AND t.table_name = ?"));

	columns_statement->setString(1, table_name);
	pkey_statement->setString(1, table_name);

	std::unique_ptr<sql::ResultSet> column_result(columns_statement->executeQuery());
	std::unique_ptr<sql::ResultSet> pkey_result(pkey_statement->executeQuery());

	std::string pkey;

	if (pkey_result->next())
	{
		pkey = pkey_result->getString(1);
	}

	std::vector<std::string> out_columns;
	while (column_result->next())
	{
		out_columns.push_back(column_result->getString(1));
	}

	if (auto_increment)
	{
		auto it = std::find(out_columns.begin(), out_columns.end(), pkey);
		if (it != out_columns.end())
		{
			out_columns.erase(it);
		}
	}

	return out_columns;
}


std::string MySqlJdbcConnector::formatInsertStatement(const std::string &table_name, const std::vector<std::string> &columns)
{
	std::stringstream names;
	std::stringstream placeholders;

	int commas = columns.size() - 1;

	for (const std::string &col : columns)
	{
		names << col;
		placeholders << "?";

		if (commas > 0)
		{
			names << ", ";
			placeholders << ", ";
			commas--;
		}
	}

	boost::format format("INSERT INTO %1%(%2%) VALUES (%3%)");
	format % table_name % names.str() % placeholders.str();

	return format.str();
}


std::unique_ptr<sql::Statement> MySqlJdbcConnector::createStatement()
{
	return std::unique_ptr<sql::Statement>(m_connection->createStatement());
}


std::unique_ptr<sql::PreparedStatement> MySqlJdbcConnector::prepareStatement(const std::string &statement)
{
	return std::unique_ptr<sql::PreparedStatement>(m_connection->prepareStatement(statement));
}