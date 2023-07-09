#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
// fixes some things idk why cuz ^
using namespace LinuxParser; 

// using namespace std
// uncommenting this breaks the pids function so just a bunch of using statements instead
using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::ifstream;
using std::istringstream;
using std::getline;

// got this from the last reviewer. thanks!
template <typename Type> Type findValueFromKey(string const& keyFilter, string const& filename){
  string line, key;
  Type value;
  ifstream filestream(kProcDirectory + filename);
  if(filestream.is_open()){
    while(getline(filestream, line)){
      istringstream linestream(line);
      while(linestream >> key >> value){
        if(key == keyFilter){
          return value;
        }
      }
    }
  }
  return value;  
}

template <typename Type>  Type getValueOfFile(string const& filename){
  string line;
  Type value;
  ifstream filestream(kProcDirectory + filename);
  if(filestream.is_open()){
    if(getline(filestream, line)){
      istringstream linestream(line);
      linestream >> value;
    }
  }
  return value;
}

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ' ', '_');
      replace(line.begin(), line.end(), '=', ' ');
      replace(line.begin(), line.end(), '"', ' ');
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = atoi(filename.c_str());
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

//just took the given code for finding os and modified it by a bit
float LinuxParser::MemoryUtilization() {
  float memTotal = findValueFromKey<float>(filterMemTotalString, kMeminfoFilename);
  float memFree = findValueFromKey<float>(filterMemFreeString, kMeminfoFilename);; // total memory, free memory
  
  return (memTotal-memFree)/memTotal;
}
// modified the given kernel code
long LinuxParser::UpTime() {
  string line; // string for storing each line
  string uptime, idleUptime; // uptime is total uptime. idleuptime is time spent in idle(obv)
  ifstream filestream(kProcDirectory + kUptimeFilename); // file
  if(getline(filestream, line)){ // can a line be read from the file?
    istringstream linestream(line);
    if(linestream >> uptime >> idleUptime){ // if correct file,
      return stol(uptime); // return the long of the uptime
    }
  }
  return 0;
}


long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); } // total = active + inactive



float LinuxParser::CpuUtilization(int pid) {
  string line, token; // token is placeholder/value(as used before) line is line of file
  float uptime, starttime; // uptime is total process uptime
  float usage;

  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if (filestream.is_open()) {// if open
    if (getline(filestream, line)) {
      istringstream linestream(line);
      for (int i = 0; i < 22; i++) {//1-12, 14-20 are useless
        linestream >> token;
        if (i == 13) { // 13th is uptime
          uptime += stol(token);
        } else if (i == 21) { // 21st is start time
          starttime = stol(token);
        }
      }
      long system_uptime = UpTime(); // self explanatory
      float hertz = static_cast<float>(sysconf(_SC_CLK_TCK));
      if (system_uptime > 0 && hertz > 0) { // did anything break?
        float total_time = (uptime) / (hertz); // calc from https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
        float seconds = system_uptime - ((starttime) / hertz);
        usage = (total_time / seconds)/(hertz);
      }
    }
  }
  return usage * 100;
}



long LinuxParser::ActiveJiffies() {
  vector<string> cpu_util = CpuUtilization();// aggregate cpu util vector
  long sum = 0; // total jiffies
  for(int i=0;i<3;i++){
    sum += stol(cpu_util[i]); // first 3 are active
  }
  return sum; 
}

long LinuxParser::IdleJiffies() { return stol(CpuUtilization()[3]); } // 4th is inactive(idle)

vector<string> LinuxParser::CpuUtilization() {
  string line;
  vector<string> cpu_util; // return vector
  ifstream filestream(kProcDirectory + kStatFilename);
  if(filestream.is_open()){ // has it opened?
    if(getline(filestream, line)){
      istringstream aggregate_cpu(line); //stringstream
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


int LinuxParser::TotalProcesses() {
  return findValueFromKey<int>(filterProcesses, kStatFilename);
}

// just modified totalprocs
int LinuxParser::RunningProcesses() {
  return findValueFromKey<int>(filterRunningProcesses, kStatFilename);
}

// just modified totalprocs and runningprocs
string LinuxParser::Command(int pid) { 
  return getValueOfFile<string>(to_string(pid) + kCmdlineFilename);
}


// using template
string LinuxParser::Ram(int pid) { 
  return to_string(findValueFromKey<long>(filterProcMem, to_string(pid) + kStatusFilename)/1024);
}

// modified command. all just searching code
string LinuxParser::Uid(int pid) { 
  return findValueFromKey<string>(filterUID, to_string(pid) + kStatusFilename);
}


string LinuxParser::User(int pid) {
  string line, username;
  ifstream filestream(kPasswordPath);
  if(filestream.is_open()){
    while(getline(filestream, line)){
      istringstream linestream(line); // substr: https://www.geeksforgeeks.org/substring-in-cpp/
      int pos = line.find(":x:"+Uid(pid)); // structure is XXXX:x:YYYY     where XXXX is username, YYYY is pid
      if(pos){ // if right line
        username = line.substr(0, pos); // get the part of the string before the pos (get the XXXX)
        break; // checking result with htop, user seems to be root, which is accurate
      }
    }
  }
  return username; 
}

long LinuxParser::UpTime(int pid) { 
  string line, token;
  long starttime;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);
  if(filestream.is_open()){
    if(getline(filestream, line)){
      istringstream linestream(line);
      for(int i=0;i<22;i++){
        linestream >> token;
      }
      starttime = stol(token);
      long seconds = UpTime() - (starttime/sysconf(_SC_CLK_TCK));
      return seconds;
    }
  }
  return 0; 
}
