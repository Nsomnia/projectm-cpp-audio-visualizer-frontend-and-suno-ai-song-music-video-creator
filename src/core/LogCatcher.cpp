#include "LogCatcher.h"

LogCatcher::LogCatcher() {
    m_cout_rdbuf = std::cout.rdbuf();
    std::cout.rdbuf(m_ss.rdbuf());

    m_cerr_rdbuf = std::cerr.rdbuf();
    std::cerr.rdbuf(m_ss.rdbuf());
}

LogCatcher::~LogCatcher() {
    std::cout.rdbuf(m_cout_rdbuf);
    std::cerr.rdbuf(m_cerr_rdbuf);
}

std::string LogCatcher::getAndClear() {
    std::string content = m_ss.str();
    m_ss.str("");
    m_ss.clear();
    return content;
}

bool LogCatcher::checkForErrors() {
    std::string log = getAndClear();
    return log.find("void") != std::string::npos || log.find("shader") != std::string::npos;
}
