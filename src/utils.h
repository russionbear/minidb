//
// Created by Administrator on 3/26/2023.
//

#ifndef MINIDB_UTILS_H
#define MINIDB_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define MAX_CHILD_THREAD_NUM 1024

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


// string

u_int8_t is_string_equal(const char* s1, const char* s2, int str_len, int flag);

int get_str_hash(const char* s1, int str_len);

char* get_string_from_p(char* s_p, int length);

/**
 *
 * @param s1
 * @param s1_len
 * @param s2
 * @param s2_len
 * @param flag 1 << 1: 是否忽略大小写
 * @return
 */
int count_appeared_nu(const char* input_str, const char* pattern, int* match_offset_size, char is_char);
void print_re_worlds(int len, int* match_offset_size, char* input_str);

// ------------------------------

struct fifo_queue_t{
    pthread_mutex_t lock;
    pthread_cond_t empty_lock, full_lock;

    int header, front;
    int queue_size;
    void *data[0];
};


void init_fifo_queue(struct fifo_queue_t* queue, int size);
void* fifo_queue_get(struct fifo_queue_t* queue, u_int8_t noblock);
int fifo_queue_put(struct fifo_queue_t* queue, void* value, u_int8_t noblock);
void free_fifo_queue(struct fifo_queue_t* queue);

// about thread pool

struct thread_pool_task_t {
    void *(*function)(void *);
    void *arg;
    int task_id;
} ;

/*线程池管理*/
struct thread_pool_t{
    pthread_mutex_t lock;                 /* 锁住整个结构体 */

    int pid;

    void *(*daemon_thread_body)(struct thread_pool_t *);
    void *(*child_thread_body)(struct thread_pool_t *);

    pthread_t *child_threads;                   /* 存放线程的tid,实际上就是管理了线 数组 */
    pthread_cond_t *child_thread_cond;
    u_int8_t *child_thread_state; // 1 or task_id: is working
    pthread_cond_t task_empty, task_full;

    int working_thr_num;                     /* 忙线程，正在工作的线程 */
    int min_thr_num;                      /* 线程池中最小线程数 */
    int max_thr_num;                      /* 线程池中最大线程数 */
    int current_thr_num;                     /* 线程池中存活的线程数 */

    u_int8_t is_ready;
    u_int8_t is_running;                         /* true为关闭 */
};

int create_thread_pool(int min_thr_num, int max_thr_num, struct thread_pool_t* tp);

/**
 * 阻塞
 * @param tp
 * @return
 */
int start_thread_pool(struct thread_pool_t* tp);
int free_thread_pool(struct thread_pool_t* tp);
int destroy_thread_pool(struct thread_pool_t* tp);


#endif //MINIDB_UTILS_H
