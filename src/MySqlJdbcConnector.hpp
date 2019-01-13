#pragma once

#include <memory>
#include <string>
#include <set>
#include <vector>

#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>

class MySqlJdbcConnector
{
private:
	std::shared_ptr<sql::Driver> driver;
	std::shared_ptr<sql::Connection>connection;
	std::set<std::string> GetColumns(const std::string& tableName, bool IsIdAutoIncrement);
	std::string FormInsertStatement(const std::string& tableName, std::set<std::string> columns);

public:
	MySqlJdbcConnector(const std::string& url, const std::string& user, const std::string& password, const std::string& database);

	/*
	*    Inserts values into the given table and returns the number of rows affected
	* NOTE:
	*    Values have to be in the order in which we are about to store them into the database
	*    eg. table columns: name, somenumber, email ... values: "Big Chungus", 420, "idk[at]gmail.com" ...
	*/
	int Insert(std::string tableName, std::vector<std::string> values, bool IsIdAutoIncrement = false);
};