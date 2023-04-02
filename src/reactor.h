//
// Created by Administrator on 3/29/2023.
//

#ifndef MINIDB_REACTOR_H
#define MINIDB_REACTOR_H
#include <pthread.h>
#include "table.h"
#include "utils.h"

// tools

struct dp_p_array{
    int element_count;
    int array_size;
    void* data[0];
};

int dp_p_array_add_ele(int v);
int dp_p_array_remove_ele(int v);
int dp_p_array_pop(int index);
int dp_p_array_clear();


// lock


struct DIS_TASK{
    char is_read;
};


struct PAGE_DATA_LOCK{
    pthread_spinlock_t lock;
    struct dp_p_array task_arr;
    struct dp_p_array page_wait_arr;
    struct dp_p_array page_run_arr;

    // 0: read 1: write
    char page_state[MAX_PAGE_NU];
};


int daemon_thread_body(struct thread_pool_t* tp);

int worker_thread_body(struct thread_pool_t* tp);

int learn_thread();


#endif //MINIDB_REACTOR_H
