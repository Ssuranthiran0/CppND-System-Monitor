#include <string>

#include "format.h"

using std::string;
using std::to_string;

// TODO: Complete this helper function DONE
// INPUT: Long int measuring seconds DONE
// OUTPUT: HH:MM:SS DONE
// REMOVE: [[maybe_unused]] once you define the function DONE
string Format::ElapsedTime(long seconds) {
    long total_minutes = seconds / 60;
    int hrs = total_minutes / 60;
    int minutes = total_minutes % 60;
    int rem_seconds = seconds % 60;
    string min = to_string(minutes), hr = to_string(hrs), sec = to_string(rem_seconds);
    if(minutes < 10){
        min = "0" + min;
    }
    if(hrs < 10){
        hr = "0" + hr;
    }
    if(rem_seconds < 10){
        sec = "0" + sec;
    }
    return hr + ":" + min + ":" + sec; 
}