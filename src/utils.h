//
// Created by Administrator on 3/26/2023.
//

#ifndef MINIDB_UTILS_H
#define MINIDB_UTILS_H
#include <stdio.h>
#include <stdlib.h>

//typedef   signed          char int8_t;
//typedef   short    int16_t;
//typedef   signed           int int32_t;
//typedef   signed       long int64_t;

//typedef   unsigned          char u_int8_t;
//typedef   unsigned short      u_int16_t;
//typedef   unsigned           int u_int32_t;
//typedef   unsigned       long u_int64_t;

typedef   float float32_t;
typedef   double float64_t;

int8_t b2int8 (u_int8_t *b_arr);
int16_t b2int16 (u_int8_t *b_arr);
int32_t b2int32 (u_int8_t *b_arr);
int64_t b2int64 (u_int8_t *b_arr);

u_int8_t b2uint8 (u_int8_t *b_arr);
u_int16_t b2uint16 (u_int8_t *b_arr);
u_int32_t b2uint32 (u_int8_t *b_arr);
u_int64_t b2uint64 (u_int8_t *b_arr);

// use iee 75

float32_t b2float32 (u_int8_t *b_arr);
float64_t b2float64 (u_int8_t *b_arr);

///------------------------------------------------------
u_int64_t int8tob(u_int8_t* b_arr, int8_t v);
u_int64_t int16tob(u_int8_t* b_arr, int16_t v);
u_int64_t int32tob(u_int8_t* b_arr, int32_t v);
u_int64_t int64tob(u_int8_t* b_arr, int64_t v);

u_int64_t uint8tob(u_int8_t* b_arr, u_int8_t v);
u_int64_t uint16tob(u_int8_t* b_arr, u_int16_t v);
u_int64_t uint32tob(u_int8_t* b_arr, u_int32_t v);
u_int64_t uint64tob(u_int8_t* b_arr, u_int64_t v);

// use iee 75

u_int64_t float32tob (u_int8_t* b_arr, float32_t v);
u_int64_t float64tob (u_int8_t* b_arr, float64_t v);

#endif //MINIDB_UTILS_H
