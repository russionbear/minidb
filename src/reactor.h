//
// Created by Administrator on 3/29/2023.
//

#ifndef MINIDB_REACTOR_H
#define MINIDB_REACTOR_H
#include <pthread.h>
#include "table.h"
#include "utils.h"

int get_socket_fd(char* ip, int port, int is_server);



// msg in,  result out

struct task_connect_t{
    struct db_server_t* db_sv;
    int is_read;
    int socket_fd;
    int msg_type;
    int msg_length;
    char msg[0];
};


struct db_server_t{
    pthread_mutex_t lock;

    char* ip;
    int port;
    int max_connection_num;
    int current_connect_num;

    struct sql_transaction_manager * tx_manager;

    int listen_epoll_fd;
    int listen_fd;
    pthread_t listen_thread_id;
    struct epoll_event* listen_events;

    struct thread_pool_t connection_pool;
    struct thread_pool_t worker_pool;

};

int init_db_server(struct db_server_t* db_sv);

int stop_db_server(struct db_server_t* db_sv);

int start_db_server(struct db_server_t* db_sv);


void listen_thread(struct db_server_t* db_sv);

void socket_handler(struct task_connect_t* connect_t);

void task_handler(struct task_connect_t* connect_t);

int learn_thread();


#endif //MINIDB_REACTOR_H
