#pragma once

#include <string>


class Configuration
{
private:
    std::string m_db_url;
    std::string m_db_user;
    std::string m_db_password;
    std::string m_db_name;

	// Agent status and monitored processes are updated in this interval
	unsigned int m_agent_update_interval{ 10 };

public:
	Configuration();
    bool parse(const std::string &xml_config);

    const std::string &getDbUrl() const { return m_db_url; }
    const std::string &getDbUser() const { return m_db_user; }
    const std::string &getDbPassword() const { return m_db_password; }
    const std::string &getDbName() const { return m_db_name; }
	unsigned int getAgentUpdateInterval() const { return m_agent_update_interval; }
};