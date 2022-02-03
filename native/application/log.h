#pragma once

const std::string LOG_FILE = "log.txt";

std::tm localtime_xp(std::time_t timer);
std::string time_stamp(const std::string& fmt = "%F %T");
void log(const std::string& msg);