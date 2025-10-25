#include "util/random.h"

float uniform(float min, float max) {
    static std::mt19937 rng(std::random_device{}());  // Seed the random engine once
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

float uniform() {
    static std::mt19937 rng(std::random_device{}());  // Seed the random engine once
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(rng);
}

int randint(int min, int max) {
    static std::mt19937 rng(std::random_device{}());  // Seed once
    std::uniform_int_distribution<int> dist(min, max); // inclusive on both ends
    return dist(rng);
}

int randrange(int min, int max) {
    static std::mt19937 rng(std::random_device{}());  // Seed once
    std::uniform_int_distribution<int> dist(min, max - 1); // inclusive on both ends
    return dist(rng);
}

int randint() {
    static std::mt19937 rng(std::random_device{}());  // Seed once
    std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    return dist(rng);
}

int randomIntNormal(double mean, double stdev) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<double> dist(mean, stdev);
    return static_cast<int>(dist(gen));
}