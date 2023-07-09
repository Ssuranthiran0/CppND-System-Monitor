#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using namespace std;
/*You need to complete the mentioned TODOs in order to satisfy the rubric criteria "The student will be able to extract and display basic data about the system."

You need to properly format the uptime. Refer to the comments mentioned in format. cpp for formatting the uptime.*/

// TODO: Return the system's CPU DONE
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes DONE
vector<Process>& System::Processes() { 
    processes_.clear(); // blank slate
    vector<int> pids = LinuxParser::Pids(); // get all current pids from linuxparser from /proc
    for(int pid : pids){ // loop through all pids
        processes_.emplace_back(pid); // add element to the back
    }
    sort(processes_.begin(), processes_.end(), less<Process>()); // sort it
    return processes_; // return
}

// for the rest: get the corresponding info from cache or linuxparser

// TODO: Return the system's kernel identifier (string) DONE
std::string System::Kernel() { 
    if(kernel_ == ""){
        kernel_ = LinuxParser::Kernel(); 
    }
    return kernel_;
}

// TODO: Return the system's memory utilization DONE
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// TODO: Return the operating system name DONE
std::string System::OperatingSystem() { 
    if(os_ == ""){
        os_ = LinuxParser::OperatingSystem();
    }
    return os_;
}

// TODO: Return the number of processes actively running on the system DONE
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// TODO: Return the total number of processes on the system DONE
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// TODO: Return the number of seconds since the system started running DONE
long int System::UpTime() { return LinuxParser::UpTime(); }
