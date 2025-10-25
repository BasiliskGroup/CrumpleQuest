#ifndef RANDOM_H
#define RANDOM_H

#include <random>

#include "includes.h"

float uniform(float min, float max);
float uniform();
int randint(int min, int max);
int randrange(int min, int max);
int randint();
int randomIntNormal(double mean, double stdev);

#endif