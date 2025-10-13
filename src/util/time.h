#ifndef TIME_H
#define TIME_H

#include "util/includes.h"
#include <chrono>
#include <thread>

std::chrono::time_point<std::chrono::high_resolution_clock> timeNow();

void sleepS(uint seconds);
void sleepMS(uint milliseconds);
void sleepUS(uint microseconds);

void printDurationUS(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2, std::string title);

void printPrimalDuration(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2);

void printDualDuration(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2);

#endif