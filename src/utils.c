//
// Created by Administrator on 3/26/2023.
//

#include "utils.h"
#include <string.h>
#include <unp.h>
#include <ctype.h>

#include <regex.h>

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

// string
u_int8_t is_string_equal(const char* s1, const char* s2, int str_len, int flag){
    int i=0;
    for(;i<str_len;i++){
        if(flag&1){
            if(tolower(s1[i])!= tolower(s2[i]))
                return 0;
        }else{
            if(s1[i]!= s2[i])
                return 0;
        }
    }
    return 1;
}


/**
 * python code
import re

def _hash(v: str):
    rlt = 0
    for i1, i in enumerate(v.upper()):
        rlt += (i1+1) * ord(i)
    return rlt


l1 = re.findall(r'\w+', s0)
l2 = [_hash(i[5:]) for i in l1]
s0 = ''
for i, j in zip(l1, l2):
    s0 += f"{i}={hex(j)},\n"
print(s0)
print(len(set(l2))==len(l2) and 42 not in l2)
 *
 *
 * @param s1
 * @param str_len
 * @return
 */
int get_str_hash(const char* s1, int str_len){
    int i=0;
    int rlt = 0;
    for(;i<str_len;i++){
        rlt += (i+1) * toupper(s1[i]);
//        printf("%d\n", toupper(s1[i]));
    }
    return rlt;
}

/**
 *
 * @param s_p
 * @param length
 * @return remember to free is not null
 */
inline char* get_string_from_p(char* s_p, int length){
    char* rlt = calloc(length, sizeof(char));
    memcpy(rlt, s_p, sizeof(char)*length);
    return rlt;
}


int count_appeared_nu(const char* input_str, const char* pattern, int* match_offset_size, char is_char){
    int i=0;
    regex_t reg;
    const char* p1=0;
    const size_t match_sz = 1;
    regmatch_t p_match[1];
    int c = regcomp(&reg, pattern, REG_EXTENDED);
    if (0 != c)
    {
        return -1;
    }
    /** 起始匹配的偏移量 */
    int offset = 0;
    int match_count = 0;
    do {
        p1 = input_str + offset;
        c = regexec(&reg, p1, match_sz, p_match, 0);

//        printf("%d, %d, %d, %d\n", p_match[0].rm_so, p_match[0].rm_eo, p_match[1].rm_so, p_match[1].rm_eo);

        if (REG_NOMATCH == c)
            break;
        else if (0 == c)
        {
            if(match_offset_size!=0){
                if(is_char){
                    match_offset_size[i] = p_match[0].rm_so + offset + 1;
                }else{
                    match_offset_size[i*2] = p_match[0].rm_so + offset;
                    match_offset_size[i*2+1] = p_match[0].rm_eo - p_match[0].rm_so;
                }
                i++;
            }
            ++match_count;
            offset += p_match[0].rm_eo;
            continue;
        }

    } while (1);
    regfree(&reg);
    return match_count;
}
void print_re_worlds(int len, int* match_offset_size, char* input_str){
    int i;
    char tmp_string[1024]={0};
    for(i=0;i<len;i++){
        memcpy(tmp_string, input_str+match_offset_size[i*2], match_offset_size[i*2+1]);
        tmp_string[match_offset_size[i*2+1]] = 0;
        printf("%d: [%d, %d];%s\n", i+1, match_offset_size[i*2], match_offset_size[i*2+1], tmp_string);
    }
}

// ---------------------------------

void* fifo_queue_get(struct fifo_queue_t* queue, u_int8_t noblock){
    void* rlt = 0;
    pthread_mutex_lock(&queue->lock);
    while (queue->front==queue->header)
    {
        if(noblock){
            pthread_mutex_unlock(&queue->lock);
            return rlt;
        }
        pthread_cond_wait(&queue->empty_lock, &queue->lock);
    }
    rlt = queue->data[queue->header];
    queue->header = (queue->header+1) % queue->queue_size;

    pthread_mutex_unlock(&queue->lock);
    pthread_cond_signal(&queue->full_lock);
    return rlt;
}

int fifo_queue_put(struct fifo_queue_t* queue, void* value, u_int8_t noblock){
    pthread_mutex_lock(&queue->lock);
    while ((queue->front+1)%queue->queue_size==queue->header)
    {
        if(noblock){
            pthread_mutex_unlock(&queue->lock);   //解锁
            return 1;
        }
        pthread_cond_wait(&queue->full_lock, &queue->lock);
    }
    queue->data[queue->front] = value;
    queue->front = (queue->front+1) % queue->queue_size;

    pthread_mutex_unlock(&queue->lock);
    pthread_cond_signal(&queue->empty_lock);

    return 0;
}



// about thread pool
void init_fifo_queue(struct fifo_queue_t* queue, int size){
    memset(queue, 0, sizeof(struct fifo_queue_t) + sizeof(void*) *( size+1));
    queue->queue_size = size+1;
    pthread_mutex_init(&queue->lock, 0);
    pthread_cond_init(&queue->empty_lock, 0);
    pthread_cond_init(&queue->full_lock, 0);
}

void free_fifo_queue(struct fifo_queue_t* queue){
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->empty_lock);
    pthread_cond_destroy(&queue->full_lock);
}

int create_thread_pool(int min_thr_num, int max_thr_num, struct thread_pool_t* tp){
    int i=0;
    memset(tp, 0, sizeof(struct thread_pool_t));
    tp->min_thr_num = min_thr_num;
    tp->max_thr_num = max_thr_num;
    tp->child_threads = calloc(max_thr_num, sizeof(pthread_t));
    tp->child_thread_cond = calloc(max_thr_num, sizeof(pthread_cond_t));
    tp->child_thread_state = calloc(max_thr_num, sizeof(uint8_t));

    if(pthread_mutex_init(&(tp->lock), NULL) != 0||
            pthread_cond_init(&tp->task_empty, NULL) != 0||
            pthread_cond_init(&tp->task_full, NULL) != 0
    ){
        return 1;
    }

    for(i=0;i<tp->max_thr_num;i++){
        if(pthread_cond_init(&tp->child_thread_cond[i], NULL) != 0)
            return 1;
    }
    tp->is_ready = 1;
    return 0;
}

/**
 * 阻塞
 * @param tp
 * @return
 */
int start_thread_pool(struct thread_pool_t* tp){
    int i;

    if(!tp->is_ready)
        return 1;
    for(i=0;i<tp->min_thr_num;i++){
        pthread_create(&(tp->child_threads[i]), NULL, (void *(*)(void *)) (tp->child_thread_body), (void *)tp);
    }
//    pthread_create(&(tp->daemon_tid), NULL, (void *(*)(void *)) tp->daemon_thread_body, (void *)tp);
    tp->daemon_thread_body(tp);
    return 0;
}


int free_thread_pool(struct thread_pool_t* tp){
    int i;

    for(i=0;i<tp->max_thr_num;i++){
        if(tp->child_threads[i]){
            pthread_cond_destroy(&tp->child_thread_cond[i]);
        }
    }

    pthread_mutex_destroy(&tp->lock);
    pthread_cond_destroy(&tp->task_empty);
    pthread_cond_destroy(&tp->task_full);
    if(tp->child_threads)
        free(tp->child_threads);
    if(tp->child_thread_cond)
        free(tp->child_thread_cond);
    if(tp->child_thread_state)
        free(tp->child_thread_state);
    tp->is_ready = 0;
}

int destroy_thread_pool(struct thread_pool_t* tp){
    int i;
    tp->is_running = 0;
    /*销毁管理者线程*/
//    pthread_join(tp->daemon_tid, NULL);

    /*等待线程结束 先是pthread_exit 然后等待其结束*/
    for (i=0; i<tp->max_thr_num; i++)
    {
        if(tp->child_threads[i])
            pthread_join(tp->child_threads[i], NULL);
    }
    free_thread_pool(tp);
    free(tp);
}

