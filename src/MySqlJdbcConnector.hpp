#pragma once

#include <memory>
#include <string>
#include <set>
#include <vector>

#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/sqlstring.h>
#include <jdbc/cppconn/prepared_statement.h>


class MySqlJdbcConnector
{
private:
	// m_driver is initialized with sql::mysql::get_driver_instance() <- this is a pointer to a static storage object
	// so we don't need a smart pointer to manage its lifetime
	sql::Driver *m_driver;
	std::unique_ptr<sql::Connection> m_connection;

	std::vector<std::string> getColumns(const std::string &table_name, bool auto_increment);
	std::string formatInsertStatement(const std::string &table_name, const std::vector<std::string> &columns);

public:
    MySqlJdbcConnector();

	bool connect(const std::string &xml_db_config);

	/*
	*    Inserts values into the given table and returns the number of rows affected
	* NOTE:
	*    Values have to be in the order in which we are about to store them into the database
	*    eg. table columns: name, somenumber, email ... values: "Big Chungus", 420, "idk[at]gmail.com" ...
	*/
	int insert(const std::string &table_name, const std::vector<std::string> &values, bool auto_increment = false);

	std::unique_ptr<sql::Statement> createStatement();

	// Executing a prepared statement takes less time than Statement because it 
	// parses,compiles the query + optimizes things in the constructor
	std::unique_ptr<sql::PreparedStatement> prepareStatement(const std::string &statement);
};