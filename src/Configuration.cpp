#include <iostream>
#include <boost/filesystem.hpp>

#include "Configuration.hpp"
#include "pugixml.hpp"


Configuration::Configuration()
{
	;
}


bool Configuration::parse(const std::string &xml_config)
{
	std::cout << "[Configuration] Loading configuration from " << xml_config << "\n";

	boost::filesystem::path path(boost::filesystem::current_path());
	std::string fullpath = path.string() + "/" + xml_config;

	pugi::xml_document xml;
	pugi::xml_parse_result result = xml.load_file(fullpath.c_str());

	if (result.status != pugi::xml_parse_status::status_ok)
	{
		std::cerr << "[Configuration] Could not parse configuration file \"" << fullpath << "\"\n";
		return false;
	}

	pugi::xml_node configuration = xml.child("Configuration");
	if (!configuration)
	{
		std::cerr << "[Configuration] Invalid configuration: missing Configuration section\n";
		return false;
	}

	if (configuration.child("UpdateInterval"))
	{
		m_agent_update_interval = configuration.child("UpdateInterval").text().as_uint();
	}

	pugi::xml_node database = configuration.child("MysqlDatabase");
	if (!database)
	{
		std::cerr << "[Configuration] Invalid configuration: missing MysqlDatabase section in Configuration\n";
		return false;
	}

	std::string url, user, password, name;
	if (database.child("Url"))
	{
		m_db_url = database.child("Url").text().as_string();
	}

	if (database.child("User"))
	{
		m_db_user = database.child("User").text().as_string();
	}

	if (database.child("Password"))
	{
		m_db_password = database.child("Password").text().as_string();
	}

	if (database.child("Name"))
	{
		m_db_name = database.child("Name").text().as_string();
	}

	return true;
}
