#pragma once

#include <cstdint>

namespace medialoader
{

typedef uint16_t float16;
typedef uint16_t float11;
typedef uint16_t float10;

void float16Init();

float float16to32(float16 f);
float16 float32to16(float f);

float float11to32(float11 f);
float11 float32to11(float f);

float float10to32(float10 f);
float10 float32to10(float f);

} // medialoader
