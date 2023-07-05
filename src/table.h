//
// 表的物理结构设计以及相应的底层操作：
// 
// 整个数据库存放在单文件中
// 最顶层是数据库，数据库包含表，表中表信息和数据信息
// 其中所有表的表信息存放在单文件的前面部分（固定范围，该范围编译时确定）
// PAGE: 数据信息n条记录组成一个PAGE, PAGE 的大小编译时确定，读写数据时，一般会一次性读取一个PAGE,以减少系统调用所耗时间
// 
//

#ifndef MINIDB_TABLE_H
#define MINIDB_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define MAX_NAME_LENGTH 256    // 名称（表名、字段名）的最大长度
#define MAX_STRING_LENGTH 256  // 字符串最大长度
#define MAX_FILED_NU 1024    // 一个表的最大字段个数
#define MAX_TABLE_NU 256  // 一个数据库的最大表的个数  默认不能超过2**12 - 16
#define PAGE_SIZE 1048576//2**20 ， 一个PAGE的最大内存大小
#define MAX_PAGE_NU 1048576//2**20,一个表的最大PAGE数量
#define MAX_SELECT_LIMIT 1024//2**10，一次性查询的最大数据条数


/**
 * 获取随机数
 * @param not_in_arr 约束 获取的随机数 不位于该数组中
 */
int get_random(int min, int max, int *not_in_arr, int not_in_arr_len);

/**
 * 支持的字段类型
 */
enum D_COL_TYPE{
    BOOL = 0, //int8
    INT = 1,  // int32
    LONG = 2,   // int 64
    FLOAT = 3,  //float 32
    DOUBLE = 4, // float 32
    CHAR,      // int8
    STRING, // length: MAX_STRING_LENGTH, int8*
    BLOB    // int64
};


/**
字段信息
*/
struct D_field{
    int table_id;
    int field_id; // = index + 1
    int index_field_id;
    char name[MAX_NAME_LENGTH];
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

/**
表信息
*/
struct D_table{
    int table_id; // = index + 1
    int row_size;
    char name[MAX_NAME_LENGTH];
};

/**
数据库信息
*/
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


/**
PAGE信息，也是数据
*/
struct D_page{
    int table_id;
    int row_nu;
    // 0: not used
    u_int8_t m_row_mask[0]; // 行掩码，表示该内存位置是否有数据
};


/**
创建数据库文件或读取数据库信息到base中
*/
int m_base(FILE *fp, u_int8_t is_read, struct D_base* base);

/**
读写数据库信息
*/
int m_base_info(FILE *fp, u_int8_t is_read, struct D_base* base);

/**
 * 读写一个数据表的page_id(参见表结构)
 */
int m_base_page_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int start_index, int length);


/**
 * 读写表信息
 */
int m_table(FILE *fp, u_int8_t is_read, struct D_base* base, int table_index);


/**
 * 读写表的字段信息
 */
int m_field(FILE *fp, u_int8_t is_read, struct D_base* base, int field_index);

// D_page， PAGE相关操作

int m_page(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, struct D_page *page_header);
int m_page_info(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, struct D_page *page_header);
int m_page_row_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, struct D_page *page_header);

// D_row， 行 相关操作

int m_row(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, u_int8_t * data);
int m_row_field(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, int *field_offset_size, int field_len, u_int8_t * data);
int m_rows(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, u_int8_t * data);
int m_rows_fields(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, int *field_offset_size, int field_len, u_int8_t * data);


/// 辅助操作

// 定义query需要的数据结构


/**
 * 运算符类型
// == >= <= != > < like in contains
// value + - * / %
 */
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

/**
 * 储存值
 */
struct R_value{
    int field_id;
    u_int8_t* value;
};

/**
 * 储存 一次运算
 */
struct R_arith_node{
    enum R_ARI_TYPE type; // 运算类型
    int param_node_ids[2];

    int field_id;
    struct D_field custom_field;
    u_int8_t* value;
};

/**
 * 存储一个多个运算节点
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


/**
 * query 的数据结构
 */
struct R_query{
    int* table_ids;
    struct R_arith_nodes condition;
};



struct R_view{
    struct R_value fields;
    void* data;
};


/**
 * 有时query需要遍历每条记录，这里定义遍历行时 函数 数据结构
 * @return 0 continue, 1 break, 2 exit
 */
typedef int(*iter_arg_func)(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t *row_data, void* argv);
int get_field_size(struct D_field* field);
int count_row_size(struct D_field* fields, int field_len, int table_id);

/**
 * 铜鼓row_size 计算每个PAGE最多有多少条记录
 * @param row_size
 * @return
 */
u_int64_t count_page_max_rows(int row_size);
u_int64_t count_page_header_size(int row_size);

/**
 * 计算表中的某个字段的数据在一条记录中存放的起始位置
 */
int count_field_offset_size(struct D_base* base, int table_id, struct D_field* fields, int length, int * rlt);

int get_table_field_num(struct D_base* base, int table_id);
struct D_field* get_table_fields(struct D_base* base, int table_id);

/**
 * 由表结构可是，字段信息存放在定长数组中，当一个位置的值为空时表示该位置可以插入新字段，这样可以一定程度上减少内存碎片化的问题
 */
int get_rest_field_index(struct D_base* base, int start_index);

/**
 * 类似于 get_rest_field_index
 */
int get_rest_table_index(struct D_base* base, int start_index);

/**
 * 重内存角度判断两值是否相等
 */
int is_memory_equal(const u_int8_t *d1, const u_int8_t * d2, int length);

/**
 * 字符串是否包含
 */
int m_string_in(const u_int8_t* s1, int len1, const u_int8_t* s2, int len2);
u_int64_t b_arith_opera(struct D_field* d1_field, struct D_field* d2_field, enum R_ARI_TYPE type, u_int8_t *d1, u_int8_t *d2, u_int8_t* rlt);


/// 高级操作，作用如函数名

int create_database(char *name);
int delete_database(char *name);
int load_database(char* name, struct D_base* base);

int m_create_table(FILE *fp, struct D_base* base, char* table_name, struct D_field* fields, int field_len);
int m_delete_table(FILE *fp, struct D_base* base, char* table_name);
int m_rename_table(FILE *fp, struct D_base* base, char* old_name, char* new_name);


int m_add_field(FILE *fp, struct D_base* base, struct D_field* field);
int m_delete_field(FILE *fp, struct D_base* base, int field_id);
int m_rename_field(FILE *fp, struct D_base* base, int field_id, char* new_name);

/**
 * 定义sql的操作类型
 */
enum Q_OPERA_TYPE{
    OP_SELECT,
    OP_DELETE,
    OP_UPDATE, // 包括 insert
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

/**
 * 遍历PAGE，默认每次遍历读取整个PAGE的数据，以较少系统调用消耗的时间
 */
int m_iter_page(FILE *fp, struct D_base* base, u_int8_t is_insert, int table_id, iter_arg_func arg_func, void* arg_func_arg);

// 数据查询
int m_i_query_rows(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t* row_data, void* argv);
int m_i_query_rows2(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t* row_data, void* argv);

/**
 * 插入数据
 */
int m_i_insert_rows(struct D_base* base, int page_index, struct D_page* page_header, u_int8_t* row_data, void* argv);

// ...... 还有许多功能未实现

#endif  // MINIDB_TABLE_H