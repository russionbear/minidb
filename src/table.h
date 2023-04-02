
#ifndef MINIDB_TABLE_H
#define MINIDB_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define MAX_NAME_LENGTH 256
#define MAX_STRING_LENGTH 256
#define MAX_FILED_NU 1024
#define MAX_TABLE_NU 256  // 默认不能超过2**12 - 16
#define PAGE_SIZE 1048576//2**20
#define MAX_PAGE_NU 1048576//2**20
#define MAX_SELECT_LIMIT 1024//2**10

// 变长+最大长度限制, don't support null, 单主键
// 为提供速度，减少系统调用，一次读一行或多行
// 页内索引
//?> about columnus


int get_random(int min, int max, int *not_in_arr, int not_in_arr_len);

/// 内存操作, 直接操作没有回滚，不考虑unique


enum D_COL_TYPE{
    BOOL = 0, //int8
    INT = 1,  // int32
    LONG = 2,   // int 64
    FLOAT = 3,  //float 32
    DOUBLE = 4, // float 32
    CHAR,      // int8
    STRING, // max 255, int8*
    BLOB    // int64
};


struct D_field{
    int table_id;
    int field_id; // = index + 1
    int index_field_id;
    char name[MAX_NAME_LENGTH]; // default 256
    enum D_COL_TYPE type;
    int length;
    int filed_size;

    // is auto increase, allow null, is default_null, is unique, is primary
    // is auto increase, is unique, is primary
    unsigned char field_mask;
    u_int8_t* default_value;
    int current_auto_id;  // must be int
//    char default_value[MAX_STRING_LENGTH];
};

struct D_table{
    int table_id; // = index + 1
    int row_size;
    char name[MAX_NAME_LENGTH];
};

struct D_base{
    char name[MAX_NAME_LENGTH];

    int table_nu;
    int field_nu;
    int page_nu;

    // 以上时base_info

    struct D_table tables[MAX_TABLE_NU];
    struct D_field fields[MAX_FILED_NU];

    // page_id  13(): is full =1 else 0, 12-1: table_id
    u_int8_t page_ids[MAX_PAGE_NU];
};


struct D_page{
    int table_id;
    int row_nu;
    // 0: not used
    u_int8_t m_row_mask[0];
};



// D_base

int m_base(FILE *fp, u_int8_t is_read, struct D_base* base);
int m_base_info(FILE *fp, u_int8_t is_read, struct D_base* base);
int m_base_page_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int start_index, int length);

// D_table

int m_table(FILE *fp, u_int8_t is_read, struct D_base* base, int table_index);

// D_field

int m_field(FILE *fp, u_int8_t is_read, struct D_base* base, int field_index);

// D_page

int m_page(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, struct D_page *page_header);
int m_page_info(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, struct D_page *page_header);
int m_page_row_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, struct D_page *page_header);

// D_row

int m_row(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, u_int8_t * data);
int m_row_field(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, int *field_offset_size, int field_len, u_int8_t * data);
int m_rows(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, u_int8_t * data);
int m_rows_fields(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, int *field_offset_size, int field_len, u_int8_t * data);


/// 辅助操作

// about query


// == >= <= != > < like in contains
// value + - * / %
enum R_ARI_TYPE{
    VALUE,
    ADD,
    SUB,
    MULTIPLY,
    DIVIDE,

    AND,
    OR,

    L_NOT = 0x10,
    L_EQ,
    L_GE,
    L_LE,
    L_NE,
    L_GT,
    L_LT,
    L_LIKE,
    L_IN,
    L_CONTAINS
};


struct R_value{
    int field_id;
    u_int8_t* value;
};

struct R_arith_node{
    enum R_ARI_TYPE type;
    int param_node_ids[2];

    int field_id;
    struct D_field custom_field;
    u_int8_t* value;
};

/**
 * 当被用于logical时， 最后一个节点(mid_value_range)的数据表示最终取值
 */
struct R_arith_nodes{
    int node_arr_length;
    struct R_arith_node* nodes;
    int field_value_offset_len[2];
    int mid_value_offset_len[2];
    int update_field_nu;
    int *update_field_id_map_node_index; //
};


struct R_query{
    int* table_ids;
    struct R_arith_nodes condition;
};



struct R_view{
    struct R_value fields;
    void* data;
};


// 0 continue, 1 break, 2 exit
typedef int(*iter_arg_func)(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t *row_data, void* argv);
int get_field_size(struct D_field* field);
int count_row_size(struct D_field* fields, int field_len, int table_id);
u_int64_t count_page_max_rows(int row_size);
u_int64_t count_page_header_size(int row_size);
int count_field_offset_size(struct D_base* base, int table_id, struct D_field* fields, int length, int * rlt);

int get_table_field_num(struct D_base* base, int table_id);
struct D_field* get_table_fields(struct D_base* base, int table_id);
int get_rest_table_index(struct D_base* base, int start_index);
int get_rest_field_index(struct D_base* base, int start_index);


int is_memory_equal(const u_int8_t *d1, const u_int8_t * d2, int length);
int m_string_in(const u_int8_t* s1, int len1, const u_int8_t* s2, int len2);
u_int64_t b_arith_opera(struct D_field* d1_field, struct D_field* d2_field, enum R_ARI_TYPE type, u_int8_t *d1, u_int8_t *d2, u_int8_t* rlt);


/// 高级操作

int create_database(char *name);
int delete_database(char *name);
int load_database(char* name, struct D_base* base);

int m_create_table(FILE *fp, struct D_base* base, char* table_name, struct D_field* fields, int field_len);
int m_delete_table(FILE *fp, struct D_base* base, char* table_name);
int m_rename_table(FILE *fp, struct D_base* base, char* old_name, char* new_name);


int m_add_field(FILE *fp, struct D_base* base, struct D_field* field);
int m_delete_field(FILE *fp, struct D_base* base, int field_id);
int m_rename_field(FILE *fp, struct D_base* base, int field_id, char* new_name);


enum Q_OPERA_TYPE{
    OP_SELECT,
    OP_DELETE,
    OP_UPDATE,
};

struct Q_select_rows{
    struct R_arith_nodes condition;

    int prim_field_len;
    int *prim_field_offset_size;
    int prim_field_row_size;

    int need_field_len;
    int *need_field_offset_size;
    int need_field_row_size;

    enum Q_OPERA_TYPE opera_type;
    FILE *write_fp;

    int rest_skip_row_num;
    int result_length;
    int result_offset;
    u_int8_t * result_data[0];

};

struct Q_insert_rows{
    FILE *write_fp;
    int row_size;
    int max_page_rows;

    int current_offset;
    int insert_data_length;
    u_int8_t * insert_data[0];

};


int m_iter_page(FILE *fp, struct D_base* base, u_int8_t is_insert, int table_id, iter_arg_func arg_func, void* arg_func_arg);
int m_i_query_rows(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t* row_data, void* argv);
int m_i_query_rows2(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t* row_data, void* argv);
int m_i_insert_rows(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t* row_data, void* argv);

#endif  // MINIDB_TABLE_H