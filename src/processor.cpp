#include "processor.h"
#include "linux_parser.h"
// TODO: Return the aggregate CPU utilization
float Processor::Utilization() { // NOTE ONLY USING FIRST 4 in jiffies
    long active = LinuxParser::ActiveJiffies(); // get current active jiffies
    long total = LinuxParser::Jiffies();// get current total jiffies.

    long dactive = active - old_active; // change in the amount of active jiffies between now and previous calc
    long dtotal = total - old_total; // change in amount of total jiffies between now and then

    float util = float(dactive)/float(dtotal); // used = active/total; but used with the recent changes to be ... recent

    old_active = active; // update
    old_total = total; // update
    return util;

}