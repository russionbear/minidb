#include "table.h"
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "reactor.h"
#include "table.h"
#include "transaction.h"

void print_database(struct D_base* base){
    int i=0, j=0;

    printf("db_name=%s,table_nu=%d,field_nu=%d\n", base->name, base->table_nu, base->field_nu);
    for(i=0, j=0;j<base->table_nu;i++){
        if(base->tables[i].table_id==0)
            continue;
        printf("table_name=%s, table_id=%d, row_size=%d\n", base->tables[i].name, base->tables[i].table_id, base->tables[i].row_size);
        j++;
    }

    for(i=0, j=0;j<base->field_nu;i++){
        if(base->fields[i].field_id==0)
            continue;
        printf("field_table=%d field_name=%s, field_type=%d\n", base->fields[i].table_id, base->fields[i].name, base->fields[i].type);
        j++;
    }

}

int test_byte_convert(){
    u_int8_t b_arr[9];

    int8_t i8=90;
    int8tob(b_arr, i8);
    printf("int8=%d,%d\n", i8, b2int8(b_arr));


    int16_t i16=500;
    int16tob(b_arr, i16);
    printf("int16=%d,%d\n", i16, b2int16(b_arr));

    int32_t i32=123456;
    int32tob(b_arr, i32);
    printf("int32=%d,%d\n", i32, b2int32(b_arr));

    int64_t i64=12345678901;
    int64tob(b_arr, i64);
    printf("int64=%ld,%ld\n", i64, b2int64(b_arr));


    u_int8_t u_i8=90;
    uint8tob(b_arr, u_i8);
    printf("u_int8=%d,%d\n", u_i8, b2uint8(b_arr));

    u_int16_t u_i16=500;
    uint16tob(b_arr, u_i16);
    printf("u_int16=%d,%d\n", u_i16, b2uint16(b_arr));

    u_int32_t u_i32=123456;
    uint32tob(b_arr, u_i32);
    printf("u_int32=%d,%d\n", u_i32, b2uint32(b_arr));

    u_int64_t u_i64=12345678901;
    uint64tob(b_arr, u_i64);
    printf("u_int64=%ld,%ld\n", u_i64, b2uint64(b_arr));


    float32_t f32=123.456f;
    float32tob(b_arr, f32);
    printf("float32=%f,%f\n", f32, b2float32(b_arr));

    float64_t f64=123.45678901;
    float64tob(b_arr, f64);
    printf("float64=%lf,%lf\n", f64, b2float64(b_arr));

}


int test_m_table(){

    int rlt = 0;
    char name[MAX_NAME_LENGTH] = "hello";
    struct D_base base;
    FILE* fp;

    printf("test database\n");
    create_database(name);

    openlog("minidb", LOG_PID | LOG_PERROR , LOG_USER);

    if((rlt=load_database(name, &base))!=0){

        printf("db error%d", rlt);
        return 1;
    }

    print_database(&base);

    printf("test table and field\n");
    if((fp=fopen(base.name, "r+b"))==0)
        return 1;
    struct D_field * fields = calloc(3, sizeof(struct D_field));
    strcpy(fields[0].name, "name1");
    strcpy(fields[1].name, "name2");

    printf("create_table\n");
    if((rlt= m_create_table(fp, &base, "stu", fields, 2)) != 0){
        printf("error%d", rlt);
        return 1;
    }
    print_database(&base);

    printf("rename_table\n");
    if((rlt= m_rename_table(fp, &base, "stu", "stu_new")) != 0){
        printf("error%d", rlt);
        return 1;
    }
    print_database(&base);

    printf("add_field\n");
    strcpy(fields[2].name, "name3");
    fields[2].table_id = 1;
    if((rlt= m_add_field(fp, &base, fields+2))!=0)
        return 4;
    print_database(&base);

    printf("rename_field\n");
    if((rlt= m_rename_field(fp, &base, 3, "new_name3"))!=0)
        return 4;
    print_database(&base);

    printf("del_field\n");
    if((rlt= m_delete_field(fp, &base, 3))!=0)
        return 4;
    print_database(&base);

    printf("del_table\n");
    if((rlt= m_delete_table(fp, &base, "stu_new"))!=0)
        return 4;
    print_database(&base);
}


int test_tx_table(){
    int rlt;
    struct sql_transaction_manager tx_manager;
    struct tx_result_select result;
    // sql parse
    char* s1 = "create database ui";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }


    s1 = "use ui";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "create table stu(id int default 8 unique primary key auto_increment, 'name' string(16))";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "alter table ui rename to stu_new";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);


    s1 = "alter table stu_new add grade int unique";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "alter table stu_new rename column grade to grade_new";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "alter table stu_new drop grade_new";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "drop table stu_new";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);
}

/* 测试insert，select*/

int test_tx_page(){
    int rlt;
    struct sql_transaction_manager tx_manager;
    struct tx_result_select result;
    memset(&tx_manager.base, 0, sizeof(struct D_base));
    char* s1 = "drop database ui";
//    don't delete this code block
//    printf("%s\n", s1);
//    rlt = run_sql(s1, &tx_manager, 0);
//    if(rlt){
//        printf("%d\n", rlt);
//        return rlt;
//    }

    // sql parse
    s1 = "create database ui";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }


    s1 = "use ui";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "create table stu(id int default 8 unique primary key auto_increment, 'name' string(16))";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "insert into stu values(1, 'qwe'), (2, 'aaa'), (999, '998f')";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    print_database(&tx_manager.base);

    s1 = "select * from stu;";
    printf("%s\n", s1);
    rlt = run_sql(s1, &tx_manager, &result);
    if(rlt){
        printf("%d\n", rlt);
        return rlt;
    }
    free(s1);
    print_database(&tx_manager.base);
    print_view_data(get_table_fields(&tx_manager.base, 1), 2, &result);
}

int test_opera(){
    int rlt = 0;
    char name[MAX_NAME_LENGTH] = "hello";
    struct D_base base;
    FILE* fp;

    create_database(name);

    if((rlt=load_database(name, &base))!=0){

        printf("db error%d", rlt);
        return 1;
    }

    if((fp=fopen(base.name, "r+b"))==0)
        return 1;

    struct D_field * fields = calloc(3, sizeof(struct D_field));
    strcpy(fields[0].name, "id");
    fields[0].type = INT;
    strcpy(fields[1].name, "name");
    fields[1].type = STRING;
    fields[1].length = 16;
    strcpy(fields[2].name, "grade");
    fields[2].type = BOOL;

    if((rlt= m_create_table(fp, &base, "stu", fields, 3)) != 0){
        printf("error%d", rlt);
        return 1;
    }
    print_database(&base);


    return 0;
}

/* 测试线程池*/

int test_thread_pool_task(void* v){
    if(v==0)
        return 1;
    sleep(1);
    printf("task: %d\n", *(int*)v);
    free(v);
}

int test_thread_pool(){
    int rlt;
    int i;
    int *number_p;
    struct thread_pool_t thread_pool;
    rlt = create_thread_pool(2, 10, 16, &thread_pool);
    if(rlt){
        destroy_thread_pool(&thread_pool);
        return 1;
    }
    thread_pool.task_thread_body = (void *(*)(void *)) test_thread_pool_task;
    start_thread_pool(&thread_pool);

    i=0;
    while(1){
        i ++;
        number_p = calloc(1, sizeof(int));
        *number_p = i;
        rlt = fifo_queue_put(thread_pool.task_queue, number_p, 0);
        printf("%i, should_drop_res=%d, alive=%d, working=%d rlt=%d\n", i,
               thread_pool.rest_drop_thread_num, thread_pool.alive_thr_num, thread_pool.working_thr_num, rlt);
        if(i==20){
            printf("123sdf sdf  sdf 89\n");
            break;
        }
    }

    pthread_join(thread_pool.daemon_tid, 0);
}

/* 测试db服务*/

int test_db_server(){
    int rlt;
    struct db_server_t db_server;
    struct tx_result_select result;
    char* s0 = "use ui";
    db_server.ip = "192.168.209.128";
    db_server.port = 8088;
    db_server.max_connection_num = 200;
    db_server.tx_manager = calloc(1, sizeof(struct sql_transaction_manager));
//    memset(&db_server.tx_manager->base, 0, sizeof(struct D_base));
    rlt = init_db_server(&db_server);
    if(rlt){
        printf("%d\n", rlt);
        return 1;
    }
    start_db_server(&db_server);
    run_sql(s0, db_server.tx_manager, &result);
    pthread_join(db_server.listen_thread_id, 0);
}


int main(int argc, char const *argv[])
{

//    printf("%d,", sizeof(&d1[1]));
//    printf("%d", sizeof(d1));
//    return test_m_table();
//    return test_byte_convert();
//    return test_opera();
//    return learn_regex();
//    learn_thread();


//    char a1[] = "12345";
//    a1[0] = 81;
//    u_int8_t a2[10];
//    memcpy(a2, a1, strlen(a1));
//    printf("%s, %s", a1, a2);
//    test_equation();
//    test_tx_table();
//    test_tx_page();
    test_db_server();
//    test_thread_pool();
}

