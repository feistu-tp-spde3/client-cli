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

    std::string getDbUrl() const; 
    std::string getDbUser() const; 
    std::string getDbPassword() const; 
    std::string getDbName() const;
    int getAgentUpdateInterval() const;
};