#include "Configuration.hpp"

#include "pugixml.hpp"

bool Configuration::parseConfiguration(const std::string& xml_config)
{
    boost::filesystem::path path(boost::filesystem::current_path());
    std::string fullpath = path.string() + "/" + xml_config;

    pugi::xml_document xml;
    pugi::xml_parse_result result = xml.load_file(fullpath.c_str());

    if (result.status != pugi::xml_parse_status::status_ok)
    {
        std::cerr << "[MysqlConnector] Could not parse configuration file \"" << fullpath << "\"\n";
        return false;
    }

    pugi::xml_node configuration = xml.child("Configuration");
    if (!configuration)
    {
        std::cerr << "[MysqlConnector] Invalid configuration\n";
        return false;
    }

    if (configuration.child("UpdateInterval"))
    {
        m_agentUpdateInterval = database.child("Url").text().as_int();
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
        m_dbUrl = database.child("Url").text().as_string();
    }

    if (database.child("User"))
    {
        m_dbUser = database.child("User").text().as_string();
    }

    if (database.child("Password"))
    {
        m_dbPassword = database.child("Password").text().as_string();
    }

    if (database.child("Name"))
    {
        m_dbName = database.child("Name").text().as_string();
    }
}

std::string Configuration::getDbUrl() const
{
    return m_dbUrl;
}

std::string Configuration::getDbUser() const
{
    return m_dbUser;
}

std::string Configuration::getDbPassword() const
{
    return m_dbPassword;
}

std::string Configuration::getDbName() const
{
    return m_dbName;
}

int Configuration::getAgentUpdateInterval() const
{
    return m_agentUpdateInterval;
}