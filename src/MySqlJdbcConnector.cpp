#include "MySqlJdbcConnector.hpp"

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/format.hpp>

#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/sqlstring.h>
#include <jdbc/cppconn/prepared_statement.h>

#include "pugixml.hpp"

MySqlJdbcConnector::MySqlJdbcConnector(const std::string& url, const std::string& user, const std::string& password, const std::string& database)
{
	driver = std::shared_ptr<sql::Driver>(sql::mysql::get_driver_instance());
	connection = std::shared_ptr<sql::Connection>(driver->connect(url, user, password));
	connection->setSchema(database);
}

MySqlJdbcConnector::MySqlJdbcConnector(const std::string& pathToConfiguration)
{
    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file(pathToConfiguration.c_str());

    if (result.status != pugi::xml_parse_status::status_ok)
    {
        throw std::runtime_error("Could not parse configuration file: " + pathToConfiguration);
    }

    pugi::xml_node clientConfiguration = xml.child("Configuration");
    
    pugi::xml_node mysqlConfiguration;

    if (clientConfiguration)
    {
        mysqlConfiguration = xml.child("MySqlConnector");
    }
    
    std::string url, user, password, database;

    if (mysqlConfiguration)
    {
        if (mysqlConfiguration.child("Url"))
        {
            url = mysqlConfiguration.child("Url").text().as_string();
        }

        if (mysqlConfiguration.child("User"))
        {
            url = mysqlConfiguration.child("User").text().as_string();
        }

        if (mysqlConfiguration.child("Password"))
        {
            url = mysqlConfiguration.child("Password").text().as_string();
        }

        if (mysqlConfiguration.child("Database"))
        {
            url = mysqlConfiguration.child("Database").text().as_string();
        }
    }

    driver = std::shared_ptr<sql::Driver>(sql::mysql::get_driver_instance());
    connection = std::shared_ptr<sql::Connection>(driver->connect(url, user, password));
    connection->setSchema(database);
}

std::set<std::string> MySqlJdbcConnector::GetColumns(const std::string& tableName, bool IsIdAutoIncrement)
{
	std::shared_ptr<sql::PreparedStatement> columnsStatement(connection->prepareStatement("SELECT column_name FROM information_schema.columns WHERE table_name = ?"));
	std::shared_ptr<sql::PreparedStatement> primaryKeyStatement(connection->prepareStatement("SELECT k.COLUMN_NAME FROM information_schema.table_constraints t LEFT JOIN information_schema.key_column_usage k USING(constraint_name, table_schema, table_name) WHERE t.constraint_type = 'PRIMARY KEY' AND t.table_schema = DATABASE() AND t.table_name = ?"));

	columnsStatement->setString(1, tableName);
	primaryKeyStatement->setString(1, tableName);

	std::shared_ptr<sql::ResultSet> columnResults(columnsStatement->executeQuery());
	std::shared_ptr<sql::ResultSet> primaryKeyResult(primaryKeyStatement->executeQuery());

	std::string primaryKey;

	if (primaryKeyResult->next())
	{
		primaryKey = primaryKeyResult->getString(1);
	}

	std::set<std::string> columns;

	while (columnResults->next())
	{
		columns.insert(columnResults->getString(1));
	}

	if (columns.size() == 0)
	{
		throw std::runtime_error("Failed to get table details, check if table exists in the database.");
	}

	if (IsIdAutoIncrement)
	{
		if (columns.find(primaryKey) != columns.end())
		{
			columns.erase(primaryKey);
		}
	}

	return columns;
}

std::string MySqlJdbcConnector::FormInsertStatement(const std::string& tableName, std::set<std::string> columns)
{
	std::stringstream ssColumns;
	std::stringstream ssPlaceHolders;

	int commas = columns.size() - 1;

	for (std::string column : columns)
	{
		ssColumns << column;
		ssPlaceHolders << "?";

		if (commas > 0)
		{
			ssColumns << ", ";
			ssPlaceHolders << ", ";
			commas--;
		}
	}

	std::string columNamesFormat = ssColumns.str();
	std::string columnValuesPlaceHolders = ssPlaceHolders.str();

	boost::format insertFormat("INSERT INTO %1%(%2%) VALUES (%3%)");
	insertFormat % tableName % columNamesFormat % columnValuesPlaceHolders;

	return insertFormat.str();
}


int MySqlJdbcConnector::Insert(std::string tableName, std::vector<std::string> values, bool IsIdAutoIncrement)
{
	std::set<std::string> columns = GetColumns(tableName, IsIdAutoIncrement);

	std::shared_ptr<sql::PreparedStatement> preparedInsertStatement(connection->prepareStatement(FormInsertStatement(tableName, columns)));

	for (size_t i = 1; i <= values.size(); i++)
	{
		preparedInsertStatement->setString(i, values[i - 1]);
	}

	return preparedInsertStatement->executeUpdate();
}