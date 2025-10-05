#include "util/print.h"

void print(std::string str) {
    std::cout << str << std::endl;
}

void print(char* str) {
    std::cout << str << std::endl;
}

void print(int n) {
    std::cout << n << std::endl;
}

void print(uint n) {
    std::cout << n << std::endl;
}

void print(float f) {
    std::cout << f << std::endl;
}

void print(const vec2& vec) {
    std::cout << "<" << vec.x << "\t" << vec.y << ">" << std::endl;
}

void print(const vec3& vec) {
    std::cout << "<" << vec.x << "\t" << vec.y << "\t" << vec.z << ">" << std::endl;
}

void print(const quat& quat) {
    std::cout << "<" << quat.w << "\t" << quat.x << "\t" << quat.y << "\t" << quat.z << ">" << std::endl;
}

void print(const mat3x3& mat) {
    for (int i = 0; i < 3; i++) print(mat[i]);
}