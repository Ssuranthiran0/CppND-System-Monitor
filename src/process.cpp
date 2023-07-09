#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Process constructor 
Process::Process(int Pid) : pid_(Pid){
    // Receive and cache unchanging properties
    command_ = LinuxParser::Command(Pid);
    user_ = LinuxParser::User(Pid);
}
// essentially jsut take everything from cache or from linuxparser namespace

// TODO: Return this process's ID DONE
int Process::Pid() const { return pid_; }

// TODO: Return this process's CPU utilization DONE
float Process::CpuUtilization() const{ return LinuxParser::CpuUtilization(pid_); }

// TODO: Return the command that generated this process DONE
string Process::Command() { return command_; }

// TODO: Return this process's memory utilization DONE
string Process::Ram() const { return LinuxParser::Ram(pid_); }

// TODO: Return the user (name) that generated this process DONE
string Process::User() { return user_; }

// TODO: Return the age of this process (in seconds) DONE
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// TODO: Overload the "less than" comparison operator for Process objects DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
bool Process::operator<(Process const& a) const {
    //return LinuxParser::Ram(Pid()) < LinuxParser::Ram(a.Pid());
    return atoi(Ram().c_str()) > atoi(a.Ram().c_str());
}