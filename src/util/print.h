#ifndef PRINT_H
#define PRINT_H

#include "util/includes.h"

void print(std::string str);
void print(char* str);
void print(int n);
void print(uint n);
void print(float f);
void print(const vec2& vec);
void print(const vec3& vec);
void print(const quat& quat);
void print(const mat3x3& mat);

#endif