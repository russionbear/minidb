//
// Created by Administrator on 4/2/2023.
//

#ifndef MINIDB_TRANSACTION_H
#define MINIDB_TRANSACTION_H

#include <pthread.h>
#include "table.h"
#include "utils.h"
#include <string.h>


enum SQL_KEY_WORLDS{
    S_KW_CREATE=0x5fc,
    S_KW_DROP=0x315,
    S_KW_USE=0x1ca,
    S_KW_ALTER=0x483,
    S_KW_SELECT=0x61c,
    S_KW_INSERT=0x684,
    S_KW_UPDATE=0x607,
    S_KW_DELETE=0x608,
    S_KW_SHOW=0x32c,

    S_KW_BOOL=0x2fd,
    S_KW_INT=0x1e1,
    S_KW_LONG=0x2f0,
    S_KW_FLOAT=0x473,
    S_KW_DOUBLE=0x603,
    S_KW_CHAR=0x2de,
    S_KW_STRING=0x645,
    S_KW_BLOB=0x2cf,

    S_KW_DATABASE=0xa03,
    S_KW_DATABASES=0xcee,
    S_KW_UNIQUE=0x657,
    S_KW_AUTO_INCREMENT=0x1f9b,
    S_KW_DEFAULT=0x861,
    S_KW_PRIMARY=0x8a3,
    S_KW_KEY=0x1e0,
    S_KW_TABLE=0x425,
    S_KW_TABLES=0x617,
    S_KW_COLUMN=0x66e,
    S_KW_RENAME=0x5e9,
    S_KW_TO=0xf2,
    S_KW_ADD=0x195,
    S_KW_FROM=0x30b,
    S_KW_AS=0xe7,
    S_KW_AND=0x1a9,
    S_KW_OR=0xf3,
    S_KW_LIMIT=0x48d,
    S_KW_ORDER_BY=0xb3d,
    S_KW_DESC=0x2d3,
    S_KW_ASC=0x1b0,
    S_KW_SET=0x1d9,
    S_KW_VALUES=0x65b,

    S_KW_COUNT=0x4bc,
    S_KW_SUM=0x1e4,
    S_KW_AVG=0x1c2,
    S_KW_IN=0xe5,
    S_KW_CONTAINS=0xad0,
    S_KW_NOT=0x1e8,
    S_KW__STAR__=42
};

struct SQL_STRUCTURE{
    int full_sql_length;
    char* full_sql;

    int can_be_used;

    // 切割成片，多数语句用到这里就ok了,一下数据 //采用world_offset_size中的index
    int world_length;
    int* world_offset_size;
    unsigned char * str_value_mark_arr;

    // create table 相关
    int create_table_range[2];
    int create_table_field_split_len;
    int* create_table_field_split_offset; // 逗号的位置

    // about insert
    int insert_field_range[2];
    int insert_value_length;
    int *insert_value_ranges;
    u_int8_t use_custom_fields;

    // select 相关
    int select_range[2];

    // about where
    int where_range[2];

    int limit_index;

    struct R_arith_nodes* condition;
};



/***************************************************************************
 * sql parse
 **************************************************************************/

void free_sql_structure(struct SQL_STRUCTURE * sql_s);

int pare_sql_from_string(struct SQL_STRUCTURE * sql_s, struct D_base *base);

int learn_regex();



/***************************************************************************
 * 事务 transaction
 **************************************************************************/


/**
 * 由于base存放在内存当中，采用这种结构有助于防止频繁变动base的结构
 */
struct sql_transaction_manager{
    struct D_base base;
    pthread_mutex_t db_mutex;
};


struct tx_result_select{
    int num;
    u_int8_t * data;
};

char* get_world_from_sql_s(struct SQL_STRUCTURE* sql_s, int index);
int get_field_id_by_pure_name(struct D_base* base, char* table_name, char* field_name);
int str2type2b(struct D_field * field, char* s0, int s0_len, u_int8_t* b);

int tx_create_database(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_drop_database(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_use_database(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);

//int tx_show_tables(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, struct tx_result_tables* result);
int tx_create_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_drop_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_alter_rename_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
//int desc_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);

int tx_alter_table_add_column(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_alter_table_drop_column(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_alter_table_rename_column(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);

//唯一一个支持并发的事务
int tx_select(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, struct tx_result_select* result);
int tx_insert(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_delete(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);
int tx_update(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result);


int run_sql(char* input_str, struct sql_transaction_manager* tx_manager, void* result);

int print_view_data(struct D_field* fields, int field_len, struct tx_result_select* view_data);

int test_equation();
/**/

// join select

//struct Q_join_select{
//
//    struct R_arith_nodes condition;
//
//    int joined_table_nu;
//    int *table_ids;
//    int *table_row_offset;
//    struct Q_select_rows * table_fields; // 只用中间三个字段
//    FILE **fps;
//
//    struct Q_select_rows result_row_field;
//};
//
//
//struct Q_yield_func_args{
//    int y_page_max_rows;
//    int y_page_header_size;
//    int y_query_row_size;
//
//    u_int8_t *y_row_data;
//
//    int y_page_b_offset;
//    int y_row_b_offset;
//    struct D_page *y_page;
//};
//
//int m_join_select(struct D_base* base, struct Q_join_select* join_select);
//
//int m_join2table_select(struct D_base* base, struct Q_join_select* join_select);

// -------------------------------------
#endif //MINIDB_TRANSACTION_H
