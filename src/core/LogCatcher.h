#pragma once

#include <iostream>
#include <sstream>
#include <string>

class LogCatcher {
public:
    LogCatcher();
    ~LogCatcher();

    std::string getAndClear();
    bool checkForErrors();

private:
    std::stringstream m_ss;
    std::streambuf* m_cout_rdbuf;
    std::streambuf* m_cerr_rdbuf;
};
