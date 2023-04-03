//
// Created by Administrator on 3/29/2023.
//

#include "reactor.h"
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
//#include<stdlib.h>
#include<unistd.h>

#include <linux/tcp.h> // struct tcp_info
#include <sys/epoll.h>
#include <sys/resource.h>		/* 设置最大的连接数需要setrlimit */

#include "transaction.h"

#define LISTEN_Q 5


int get_socket_fd(char* ip, int port, int is_server){
    struct sockaddr_in serv_addr;
    int listen_fd;

    if((listen_fd = socket(AF_INET , SOCK_STREAM , 0)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    bzero(&serv_addr , sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if(is_server){
        if(bind(listen_fd , (struct sockaddr*)&serv_addr , sizeof(serv_addr)) < 0)
        {
            perror("bind error");
            exit(1);
        }

        if(listen(listen_fd , LISTEN_Q) < 0)
        {
            perror("listen error");
            exit(1);
        }
    }else{
        if(connect(listen_fd , (struct sockaddr *)&serv_addr , sizeof(serv_addr)) < 0)
        {
            perror("connect error");
            return 0;
        }
    }

    return listen_fd;
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

int init_db_server(struct db_server_t* db_sv){
    int rlt;

    rlt = create_thread_pool(1, 1, 1024, &db_sv->connection_pool);
    if(rlt){
        destroy_thread_pool(&db_sv->connection_pool);
        return 1;
    }
    rlt = create_thread_pool(1, 1, 1024, &db_sv->worker_pool);
    if(rlt){
        destroy_thread_pool(&db_sv->connection_pool);
        destroy_thread_pool(&db_sv->worker_pool);
        return 2;
    }

    db_sv->connection_pool.task_thread_body = (void *(*)(void *)) socket_handler;
    db_sv->worker_pool.task_thread_body = (void *(*)(void *)) task_handler;

    pthread_mutex_init(&db_sv->lock, 0);
    db_sv->current_connect_num = 0;
    db_sv->listen_events = calloc(db_sv->max_connection_num, sizeof(struct epoll_event));
    return 0;
}

int stop_db_server(struct db_server_t* db_sv){
    destroy_thread_pool(&db_sv->worker_pool);
    destroy_thread_pool(&db_sv->connection_pool);
//    pthread_mutex_destroy(&db_sv->lock);
    pthread_join(db_sv->listen_thread_id, 0);
    free(db_sv->listen_events);
}

int start_db_server(struct db_server_t* db_sv){
    start_thread_pool(&db_sv->worker_pool);
    start_thread_pool(&db_sv->connection_pool);
    pthread_create(&db_sv->listen_thread_id, 0, (void *(*)(void *)) listen_thread, db_sv);
}

void listen_thread(struct db_server_t* db_sv){
    int     conn_fd;
    int 	wait_fds;
    struct sockaddr_in cli_addr;
    struct 	epoll_event	ev;
//    struct 	rlimit	rlt;		//!> 设置连接数所需
    socklen_t	len = sizeof( struct sockaddr_in );
    struct task_connect_t* connect_t=0;
    int task_connect_t_size = sizeof(struct task_connect_t) + sizeof(char)* MAX_STRING_LENGTH;

    int i;

//    db_sv->listen_events = calloc(db_sv->max_connection_num, sizeof(struct 	epoll_event));
    db_sv->listen_fd = get_socket_fd(db_sv->ip, db_sv->port, 1);

    db_sv->listen_epoll_fd = epoll_create(db_sv->max_connection_num );
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = db_sv->listen_fd;
    if( epoll_ctl( db_sv->listen_epoll_fd, EPOLL_CTL_ADD, db_sv->listen_fd, &ev ) < 0 )
    {
        exit( EXIT_FAILURE );
    }
    db_sv->current_connect_num = 1;

    while( 1 ) {
        if ((wait_fds = epoll_wait(db_sv->listen_epoll_fd, db_sv->listen_events, db_sv->current_connect_num, -1)) == -1) {
            exit(EXIT_FAILURE);
        }
        for( i = 0; i < wait_fds; i++ ) {
            if (db_sv->listen_events[i].data.fd== db_sv->listen_fd && db_sv->current_connect_num < db_sv->max_connection_num) {
                if ((conn_fd = accept(db_sv->listen_fd, (struct sockaddr *) &cli_addr, &len)) == -1) {
                    continue;
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_fd;
                if (epoll_ctl(db_sv->listen_epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev) < 0) {
                    exit(EXIT_FAILURE);
                }

                pthread_mutex_lock(&db_sv->lock);
                db_sv->current_connect_num++;
                pthread_mutex_unlock(&db_sv->lock);
                puts("new connect");
                continue;
            }else if(db_sv->listen_events[i].events & EPOLLIN){
                connect_t = calloc(1, task_connect_t_size);
                memset(connect_t, 0, task_connect_t_size);
                connect_t->is_read = 1;
                connect_t->socket_fd = conn_fd;
                connect_t->db_sv = db_sv;

                fifo_queue_put(db_sv->connection_pool.task_queue, connect_t, 0);
//                connect_t = 0;
            }
        }
    }
}

void socket_handler(struct task_connect_t* connect_t){
    int n_read;
    struct 	epoll_event ev;

    if(connect_t==0)
        return;
    if(connect_t->is_read){
        n_read = (int)read(connect_t->socket_fd, connect_t->msg, MAX_STRING_LENGTH );
        connect_t->msg_length = n_read;
        connect_t->msg[n_read] = 0;
        if(n_read <= 0 )						//!> 结束后者出错
        {
            ev.data.fd = connect_t->socket_fd;
            ev.events = EPOLLIN | EPOLLET;
            close( connect_t->socket_fd );
            epoll_ctl( connect_t->db_sv->listen_epoll_fd, EPOLL_CTL_DEL, connect_t->socket_fd, &ev );	//!> 删除计入的fd
            pthread_mutex_lock(&connect_t->db_sv->lock);
            connect_t->db_sv->current_connect_num -- ;
            pthread_mutex_unlock(&connect_t->db_sv->lock);
            free(connect_t);
            puts("drop connect");
            return;
        }
        puts("rev");
//        write( connect_t->socket_fd, connect_t->msg, connect_t->msg_length );
//        return;
        fifo_queue_put(connect_t->db_sv->worker_pool.task_queue, connect_t, 0);
    }else{
        puts("send");
        write( connect_t->socket_fd, connect_t->msg, connect_t->msg_length );
        free(connect_t);
    }
}

void task_handler(struct task_connect_t* connect_t){
    int rlt;
    struct tx_result_select view_data;
    struct task_connect_t* new_connect_t=0;
    char* tmp_string=0;

    if(connect_t==0)
        return;

//    connect_t->is_read = 0;
//    fifo_queue_put(connect_t->db_sv->connection_pool.task_queue, connect_t, 0);
//    return;
    tmp_string = calloc(connect_t->msg_length+1, sizeof(char));
    memcpy(tmp_string, connect_t->msg, connect_t->msg_length * sizeof(char));
    tmp_string[connect_t->msg_length] = 0;

    rlt = run_sql(tmp_string, connect_t->db_sv->tx_manager, &view_data);
    if(rlt){
        free(connect_t);
        printf("%d\n", rlt);
        return;
    }
//    free(connect_t);
//    printf("hi");
//    return;

    new_connect_t = calloc(1, sizeof(struct task_connect_t)+sizeof(char)*view_data.num);
    memset(new_connect_t, 0, sizeof(struct task_connect_t)+sizeof(char)*view_data.num);
    memcpy(new_connect_t, connect_t, sizeof(struct task_connect_t));
    new_connect_t->msg_length = view_data.num;
    new_connect_t->msg_type = view_data.type;
    new_connect_t->is_read = 0;
    memcpy(new_connect_t->msg, view_data.data, view_data.num);
    fifo_queue_put(connect_t->db_sv->connection_pool.task_queue, new_connect_t, 0);
    print_view_data(get_table_fields(&connect_t->db_sv->tx_manager->base, 1), 2, &view_data);
    free(connect_t);
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
