
#include <chrono>
#include <ctime>  
#include <iomanip>
#include <fstream>

#include "log.h"


std::tm localtime_xp(std::time_t timer)
{
    std::tm bt{};
#if defined(__unix__)
    localtime_r(&timer, &bt);
#elif defined(_MSC_VER)
    localtime_s(&bt, &timer);
#else
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    bt = *std::localtime(&timer);
#endif
    return bt;
}

// default = "YYYY-MM-DD HH:MM:SS"
std::string time_stamp(const std::string& fmt)
{
    auto bt = localtime_xp(std::time(0));
    char buf[64];
    return { buf, std::strftime(buf, sizeof(buf), fmt.c_str(), &bt) };
}


void log(const std::string& msg)
{
    std::ofstream outfile;
    outfile.open(LOG_FILE, std::fstream::in | std::fstream::out | std::fstream::app);
    if (!outfile)
    {
        outfile.open(LOG_FILE, std::fstream::in | std::fstream::out | std::fstream::trunc);
    }

    outfile << "[" << time_stamp() << "] : " << msg << std::endl;
    outfile.close();
}
