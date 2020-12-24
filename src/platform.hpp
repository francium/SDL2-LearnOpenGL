#pragma once


#include <stdint.h>
#include <stdbool.h>


#define internal static


typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t imm;
typedef uintptr_t umm;

typedef size_t memory_index;

typedef float f32;
typedef double f64;


template <class T>
struct Result
{
    bool ok;
    T value;
};
