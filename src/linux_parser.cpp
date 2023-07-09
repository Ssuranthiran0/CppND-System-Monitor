#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization DONE 
//just took the given code for finding os and modified it by a bit
float LinuxParser::MemoryUtilization() { 
  string line; // string for storing each line
  string key; // string for storing the first "word" of each line (key)
  string value; // string for storing useful data
  long memTotal, memFree; // total memory, free memory
  std::ifstream filestream(kProcDirectory + kMeminfoFilename); // open file stream to read from /proc/meminfo
  if (filestream.is_open()) { // able to access file
    while (std::getline(filestream, line)) { // filestream -> line
      std::istringstream linestream(line); // stringstream for utility
      while (linestream >> key) { // every character
        if (key == "MemTotal:") { // MemTotal:             Value
          linestream >> value; // put the string(number) into the var
          memTotal = stol(value)/1000; // kb -> mb
        }
        if (key == "MemFree:") { 
          linestream >> value; // put the string(number) into val
          memFree = stol(value)/1000; // kb -> mb
          return float(memTotal-memFree)/float(memTotal); // memutil = used/total = (total-free)/total
        }
      }
    }
  }

  return 0.0;
}
// TODO: Read and return the system uptime DONE
// modified the given kernel code
long LinuxParser::UpTime() {
  string line; // string for storing each line
  string uptime, idleUptime; // uptime is total uptime. idleuptime is time spent in idle(obv)
  std::ifstream filestream(kProcDirectory + kUptimeFilename); // file
  if(std::getline(filestream, line)){ // can a line be read from the file?
    std::istringstream linestream(line);
    if(linestream >> uptime >> idleUptime){ // if correct file,
      return stol(uptime); // return the long of the uptime
    }
  }
  return 0;
}

// TODO: Read and return the number of jiffies for the system DONE
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); } // total = active + inactive

// TODO: Read, calculate, and return the Cpu Utilization for a given process DONE
float LinuxParser::CpuUtilization(int pid) {
  string line, token; // token is placeholder/value(as used before) line is line of file
  long uptime, starttime; // uptime is total process uptime
  float usage;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {// if open
    if (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      for (int i = 0; i < 22; i++) {//1-12, 14-20 are useless
        linestream >> token;
        if (i == 13) { // 13th is uptime
          uptime += std::stol(token);
        } else if (i == 21) { // 21st is start time
          starttime = std::stol(token);
        }
      }
      long system_uptime = LinuxParser::UpTime(); // self explanatory
      long hertz = sysconf(_SC_CLK_TCK);
      if (system_uptime > 0 && hertz > 0) { // did anything break?
        float total_time = uptime / hertz; // calc from https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
        float seconds = system_uptime - (starttime / hertz);
        usage = (total_time / seconds);
      }
    }
  }
  return usage;
}



// TODO: Read and return the number of active jiffies for the system DONE
long LinuxParser::ActiveJiffies() {
  vector<string> cpu_util = CpuUtilization();// aggregate cpu util vector
  long sum = 0; // total jiffies
  for(int i=0;i<3;i++){
    sum += std::stol(cpu_util[i]); // first 3 are active
  }
  return sum; 
}

// TODO: Read and return the number of idle jiffies for the system DONE
long LinuxParser::IdleJiffies() { return std::stol(CpuUtilization()[3]); } // 4th is inactive(idle)

// TODO: Read and return CPU utilization DONE
vector<string> LinuxParser::CpuUtilization() {
  string line;
  vector<string> cpu_util; // return vector
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){ // has it opened?
    if(std::getline(filestream, line)){
      std::istringstream aggregate_cpu(line); //stringstream
      string token;
      aggregate_cpu >> token; // ignore first token (cpu0)
      for(int i=0;i<4;i++){ // first 4 only
        aggregate_cpu >> token;
        cpu_util.emplace_back(token); // add to back of array
      }
    }
  }
  return cpu_util; 
}

// TODO: Read and return the total number of processes DONE
// again, just took the given starter code for finding os and changed the key(PrettyName->procs) and filepath
int LinuxParser::TotalProcesses() {
  string line, key, value; // line, key, value. same as mem util
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) { // accessible?
    while (std::getline(filestream, line)) { // can get line?
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "processes") { // right line?
          linestream >> value;
          return std::stoi(value); 
        }
      }
    }
  }
  return 0; 
}

// TODO: Read and return the number of running processes DONE (same as above but different key) DONE
// just modified totalprocs
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "procs_running") { // only difference to above function
          linestream >> value;
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// TODO: Read and return the command associated with a process DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
// just modified totalprocs and runningprocs
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename); // filestream for accessing file
  if(filestream.is_open()){ // did it work?
    if(std::getline(filestream, line)){ // can access file?
      std::istringstream linestream(line);
      string cmd; 
      if(linestream >> cmd){ // is it the right line? (can it be shoveled into cmd?)
        return cmd;
      }
    }
  }
  return string(); 
}

// TODO: Read and return the memory used by a process DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
// modified command
string LinuxParser::Ram(int pid) { 
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line); // access line
      while (linestream >> key) { // check every line
        if (key == "VmSize:") { // very similar to above 2 functions
          linestream >> value;
          return std::to_string(std::stol(value)/1000); // kb -> mb
        }
      }
    }
  }
  return string(); 
}

// TODO: Read and return the user ID associated with a process DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
// modified command. all just searching code
string LinuxParser::Uid(int pid) { 
  string line; // line str
  string key; // first element of line
  string value; // data
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename); // open proc/pid/status
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) { // true if filestream >> line, false if not
      std::istringstream linestream(line);
      while (linestream >> key) { // every line
        if (key == "Uid:") { // right line?
          linestream >> value; // shovel
          return value;
        }
      }
    }
  }
  return string(); 
}

// TODO: Read and return the user associated with a process DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
string LinuxParser::User(int pid) {
  string line, username;
  std::ifstream filestream(kPasswordPath);
  if(filestream.is_open()){
    while(std::getline(filestream, line)){
      std::istringstream linestream(line); // substr: https://www.geeksforgeeks.org/substring-in-cpp/
      int pos = line.find(":x:"+Uid(pid)); // structure is XXXX:x:YYYY     where XXXX is username, YYYY is pid
      if(pos){ // if right line
        username = line.substr(0, pos); // get the part of the string before the pos (get the XXXX)
        break;
      }
    }
  }
  return username; 
}

// TODO: Read and return the uptime of a process DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
long LinuxParser::UpTime(int pid) { 
  string line, token;
  long starttime;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(filestream.is_open()){
    if(std::getline(filestream, line)){
      std::istringstream linestream(line);
      for(int i=0;i<22;i++){
        linestream >> token;
      }
      starttime = std::stol(token);
      long seconds = UpTime() - (starttime/sysconf(_SC_CLK_TCK));
      return seconds;
    }
  }
  return 0; 
}
