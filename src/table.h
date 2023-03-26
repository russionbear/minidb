#define __TABLE__
#ifdef __TABLE__

#include <stdio.h>
#include <stdlib.h>

#define MAX_NAME_LENGTH 256
#define MAX_STRING_LENGTH 256
#define MAX_FILED_NU 1024
#define MAX_TABLE_NU 256  // 默认不能超过2**12 - 16
#define PAGE_SIZE 1048576//2**20
#define MAX_PAGE_NU 1048576//2**20
#define MAX_SELECT_LIMIT 1048576//2**20

// 变长+最大长度限制, don't support null

//?> about columnus


int get_random(int min, int max, int *not_in_arr, int not_in_arr_len);

/// 内存操作, 直接操作没有回滚，不考虑unique


enum D_COL_TYPE{
    BOOL = 0,
    INT = 1,
    LONG = 2,
    FLOAT = 3,
    DOUBLE = 4,
    CHAR,
    STRING, // max 255
    BLOB
};


struct D_field{
    int table_id;
    int field_id; // = index + 1
    char name[MAX_NAME_LENGTH]; // default 256
    enum D_COL_TYPE type;
    int length;
    int filed_size;

    // is auto increase, allow null, is default_null, is unique, is primary
    // is auto increase, is unique, is primary
    unsigned char field_mask;
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

    struct D_table tables[MAX_TABLE_NU];
    struct D_field fields[MAX_FILED_NU];

    // page_id  13(): is full =1 else 0, 12-1: table_id
    u_int8_t page_ids[MAX_PAGE_NU];
};


struct D_page{
    int table_id;
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
int m_page_row_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, struct D_page *page_header);

// D_row

int m_row(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, char* data);
int m_row_field(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, int *field_offset_size, int field_len, char* data);
int m_rows(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, char* data);
int m_rows_fields(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, int *field_offset_size, int field_len, char* data);


/// 辅助操作
int as_check_unique(){

}


/// 高级操作

// context

struct D_context{
    struct D_base database;
    struct D_table* tables;
    struct D_field* fields;
};

// about query

// == >= <= != > < like in contains
enum R_LOGICAL_TYPE{
    AND = 1,
    OR = 2
};

// value + - * / %
enum R_ARI_TYPE{
    VALUE,
    ADD,
    SUB,
    MULTIPLY,
    DIVIDE,

    L_EQ = 0x10,
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
    int table_id;
    void* value;
    char* field_name;
};

struct R_arith{
    enum R_ARI_TYPE type;
    struct R_value* params[2];
};

struct R_logical{
    enum R_LOGICAL_TYPE * logical;
    struct R_arith* arith_arr;
};


struct R_query{
    int* table_ids;
    struct R_logical condition;
};

struct R_field_opera{
    int table_id;
    char* field_name;
    struct R_arith opera;
};


struct R_view{
    struct R_value fields;
    void* data;
};


int create_database(char *name);
int delete_database(char *name);
int load_database(char* name, struct D_context * ctx);

int create_table(struct D_context * ctx, char* table_name, struct D_field* fields, int field_len);
int delete_table(struct D_context * ctx, char* table_name);
int rename_table(struct D_context * ctx, char* old_name, char* new_name);


int add_field(struct D_context * ctx, int table_id, struct D_field* field);
int delete_field(struct D_context * ctx, int table_id, char * field_name);
int rename_field(struct D_context * ctx, int table_id, char* old_name, char* new_name);


int insert_rows(struct D_context * ctx, int table_id, int field_len, char** field_names, int value_len, void** values);
int delete_rows(struct D_context * ctx, int * table_ids, struct R_query * query);
int update_rows(struct D_context * ctx, int field_opera_len, struct R_field_opera* field_opera, struct R_query * query);
int do_select(struct D_context * ctx, struct R_query* query, struct R_view* view);


#endif
