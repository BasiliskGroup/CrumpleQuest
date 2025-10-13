#include "util/time.h"

std::chrono::time_point<std::chrono::high_resolution_clock> timeNow() { return std::chrono::high_resolution_clock::now(); }

void sleepS(uint seconds) { return std::this_thread::sleep_for(std::chrono::seconds(seconds)); }
void sleepMS(uint milliseconds) { return std::this_thread::sleep_for(std::chrono::seconds(milliseconds)); }
void sleepUS(uint microseconds) { return std::this_thread::sleep_for(std::chrono::microseconds(microseconds)); }

void printDurationUS(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2, std::string title) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    std::cout << title << duration.count() << "us" << std::endl;
}

void printPrimalDuration(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    std::cout << "Primal: " << duration.count() << "us" << "\t";
}

void printDualDuration(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    std::cout << "Dual:\t" << duration.count() << "us" << std::endl;
}