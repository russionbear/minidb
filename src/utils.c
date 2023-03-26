//
// Created by Administrator on 3/26/2023.
//

#include "utils.h"


inline int8_t b2int8 (u_int8_t *b_arr){ return (int8_t)b_arr[0]; }
inline int16_t b2int16 (u_int8_t *b_arr){ return (int16_t )((b_arr[0] << 8) + (b_arr[1])); }
inline int32_t b2int32 (u_int8_t *b_arr) {return (b_arr[0] << 24) + (b_arr[1] << 16) + (b_arr[2] << 8) + (b_arr[3]);}
inline int64_t b2int64 (u_int8_t *b_arr) {return ((int64_t)b_arr[0] << 56) + ((int64_t)b_arr[1] << 48) + ((int64_t)b_arr[2] << 40) + ((int64_t)b_arr[3] << 32) + ((int64_t)b_arr[4] << 24) + ((int64_t)b_arr[5] << 16) + ((int64_t)b_arr[6] << 8) + ((int64_t)b_arr[7]);}

inline u_int8_t b2uint8 (u_int8_t *b_arr){ return b_arr[0]; }
inline u_int16_t b2uint16 (u_int8_t *b_arr){ return (u_int16_t )((b_arr[0] << 8) + (b_arr[1])); }
inline u_int32_t b2uint32 (u_int8_t *b_arr) {return ((u_int32_t)b_arr[0] << 24) + ((u_int32_t)b_arr[1] << 16) + ((u_int32_t)b_arr[2] << 8) + ((u_int32_t)b_arr[3]);}
inline u_int64_t b2uint64 (u_int8_t *b_arr) {return ((u_int64_t)b_arr[0] << 56) + ((u_int64_t)b_arr[1] << 48) + ((u_int64_t)b_arr[2] << 40) + ((u_int64_t)b_arr[3] << 32) + ((u_int64_t)b_arr[4] << 24) + ((u_int64_t)b_arr[5] << 16) + ((u_int64_t)b_arr[6] << 8) + ((u_int64_t)b_arr[7]);}


float32_t b2float32 (u_int8_t *b_arr){
    u_int8_t i;
    float32_t rlt;
    void *pf = &rlt;
    u_int8_t * px = b_arr;
    for(i=0;i<4;i++)
    {
        *((u_int8_t*)pf + i)=*(px + i);
    }
    return rlt;
}
float64_t b2float64 (u_int8_t *b_arr){
    u_int8_t i;
    float64_t rlt;
    void *pf = &rlt;
    u_int8_t * px = b_arr;
    for(i=0;i<8;i++)
    {
        *((u_int8_t*)pf + i)=*(px + i);
    }
    return rlt;
}


/// 字节截断

inline u_int64_t int8tob(u_int8_t* b_arr, int8_t v){
    return uint8tob(b_arr, v);
}
inline u_int64_t int16tob(u_int8_t* b_arr, int16_t v){
    return uint16tob(b_arr, v);
}
inline u_int64_t int32tob(u_int8_t* b_arr, int32_t v){
    return uint32tob(b_arr, v);
}
inline u_int64_t int64tob(u_int8_t* b_arr, int64_t v){
    return uint64tob(b_arr, v);
}

inline u_int64_t uint8tob(u_int8_t* b_arr, u_int8_t v){
    b_arr[0] = (u_int8_t)v;
    return 1;
}
inline u_int64_t uint16tob(u_int8_t* b_arr, u_int16_t v){
    b_arr[0] = (u_int8_t)(v >> 8);
    b_arr[1] = (u_int8_t)v;
    return 2;
}
inline u_int64_t uint32tob(u_int8_t* b_arr, u_int32_t v){
    b_arr[0] = (u_int8_t)(v >> 24);
    b_arr[1] = (u_int8_t)(v >> 16);
    b_arr[2] = (u_int8_t)(v >> 8);
    b_arr[3] = (u_int8_t)v;
    return 4;
}
inline u_int64_t uint64tob(u_int8_t* b_arr, u_int64_t v){
    b_arr[0] = (u_int8_t)(v >> 56);
    b_arr[1] = (u_int8_t)(v >> 48);
    b_arr[2] = (u_int8_t)(v >> 40);
    b_arr[3] = (u_int8_t)(v >> 32);
    b_arr[4] = (u_int8_t)(v >> 24);
    b_arr[5] = (u_int8_t)(v >> 16);
    b_arr[6] = (u_int8_t)(v >> 8);
    b_arr[7] = (u_int8_t)v;
    return 8;

}

// use iee 75

u_int64_t float32tob (u_int8_t* b_arr, float32_t v){
    u_int8_t i;
    u_int8_t *pd = (u_int8_t*)&v;
    for(i=0;i<4;i++)
        b_arr[i]=*pd++;
    return 4;
}
u_int64_t float64tob (u_int8_t* b_arr, float64_t v){
    u_int8_t i;
    u_int8_t *pd = (u_int8_t*)&v;
    for(i=0;i<8;i++)
        b_arr[i]=*pd++;
    return 8;
}