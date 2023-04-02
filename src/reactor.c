//
// Created by Administrator on 3/29/2023.
//

#include "reactor.h"
#include <string.h>

//#include<stdlib.h>
#include<unistd.h>

int daemon_thread_body(struct thread_pool_t* tp){

}

int worker_thread_body(struct thread_pool_t* tp){
    struct thread_pool_t *pool = (struct thread_pool_t *)tp;
    struct thread_pool_task_t task;

    while (1)
    {

    }

    pthread_exit(NULL);
}


void print_queue(struct fifo_queue_t* queue){
    int i;
    for(i=queue->header;i!=queue->front;i=(i+1)%queue->queue_size)
        printf("%d, ", *(int*)queue->data[i]);
    printf("\n");
}


//void* fifo_queue_get(struct fifo_queue_t* queue, u_int8_t noblock){
//    void* rlt = 0;
//    pthread_mutex_lock(&queue->lock);
//    while (queue->front==queue->header)
//    {
//        if(noblock){
//            pthread_mutex_unlock(&queue->lock);
//            return rlt;
//        }
//        pthread_cond_wait(&queue->empty_lock, &queue->lock);
//    }
//    rlt = queue->data[queue->header];
//    queue->header = (queue->header+1) % queue->queue_size;
//
//    pthread_mutex_unlock(&queue->lock);
//    pthread_cond_signal(&queue->full_lock);
//    return rlt;
//}
//
//int fifo_queue_put(struct fifo_queue_t* queue, void* value, u_int8_t noblock){
//    void* rlt = 0;
//    pthread_mutex_lock(&queue->lock);
//    while ((queue->front+1)%queue->queue_size==queue->header)
//    {
//        if(noblock){
//            pthread_mutex_unlock(&queue->lock);   //解锁
//            return rlt;
//        }
//        pthread_cond_wait(&queue->full_lock, &queue->lock);
//
//        printf("now: ");
//        print_queue(queue);
//    }
//    queue->data[queue->front] = value;
//    queue->front = (queue->front+1) % queue->queue_size;
//
//    pthread_mutex_unlock(&queue->lock);
//    pthread_cond_signal(&queue->empty_lock);
//
//    return rlt;
//}


void worker_thread(struct fifo_queue_t* queue){
    int i;

    while ((i=*((int*) fifo_queue_get(queue, 0))))
    {
        printf("out: %d, ", i);
        print_queue(queue);
        sleep(1);
    }
}

void master_thread(struct fifo_queue_t* queue){
    int i=0;

    while (1)
    {
        i++;
        fifo_queue_put(queue, &i, 0);

    }
}

int learn_thread(){
    int max_length=4;
    struct fifo_queue_t* task_queue;
    task_queue = calloc(1, sizeof(struct fifo_queue_t) + sizeof(void*) * max_length);
    memset(task_queue, 0, sizeof(struct fifo_queue_t) + sizeof(void*) * max_length);
    task_queue->queue_size = max_length;
    pthread_mutex_init(&task_queue->lock, 0);
    pthread_cond_init(&task_queue->empty_lock, 0);
    pthread_cond_init(&task_queue->full_lock, 0);

    int i1=1, i2=2, i3=3;
    pthread_t pid, m_pid;
//    pthread_create(&pid, 0, worker_thread, task_queue);
    pthread_create(&m_pid, 0, master_thread, task_queue);

//    fifo_queue_put(task_queue, &i1, 0);
//    print_queue(task_queue);
//    fifo_queue_put(task_queue, &i2, 0);
//    print_queue(task_queue);
//    fifo_queue_put(task_queue, &i3, 0);
//    print_queue(task_queue);
//    fifo_queue_put(task_queue, &i3, 0);
//    sleep(2);
//    printf("get: %d\n", *(int*)fifo_queue_get(task_queue, 0));
//    printf("get: %d\n", *(int*)fifo_queue_get(task_queue, 0));
//    printf("get: %d\n", *(int*)fifo_queue_get(task_queue, 0));
//    printf("get: %d", *(int*)fifo_queue_get(task_queue, 0));
//    pthread_join(pid, 0);
    pthread_join(m_pid, 0);
}
