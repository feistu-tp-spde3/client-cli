#pragma once

#include <string>

class Configuration
{
private:
    std::string m_dbUrl;
    std::string m_dbUser;
    std::string m_dbPassword;
    std::string m_dbName;
    int m_agentUpdateInterval;

public:
    bool parseConfiguration(const std::string& xml_config);

    std::string getDbUrl() const { return m_dbUrl; }

    std::string getDbUser() const { return m_dbUser; }

    std::string getDbPassword() const { return m_dbPassword; }

    std::string getDbName() const { return m_dbName; }

    int getAgentUpdateInterval() const { return m_agentUpdateInterval; }
};