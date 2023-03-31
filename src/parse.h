//
// Created by 19545 on 28/03/2023.
//

#ifndef MINIDB_PARSE_H
#define MINIDB_PARSE_H

#include "table.h"


enum SQL_PREFIX{
    PREFIX_CREATE,
    PREFIX_DROP,
    PREFIX_USE,
    PREFIX_ALTER,
    PREFIX_SELECT,
    PREFIX_INSERT,
    PREFIX_UPDATE,
    PREFIX_DELETE
};


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

    enum SQL_PREFIX type;
    int can_be_used;

    // 切割成片，多数语句用到这里就ok了,一下数据 //采用world_offset_size中的index
    int world_length;
    int* world_offset_size;
    unsigned char * str_value_mark_arr;
    int str_value_count;

    // create table 相关
    int create_table_range[2];
    int create_table_field_len;
    int* create_table_field_split_offset; // 逗号的位置
    struct D_field* pared_fields;

    // select 相关
    int select_range[2];

    // about where
    int where_range[2];

    int limit_index;

    struct R_arith_nodes* condition;
};

int get_str_hash(const char* s1, int str_len);

void free_sql_structure(struct SQL_STRUCTURE * sql_s);

int pare_sql_from_string(struct SQL_STRUCTURE * sql_s);


int learn_regex();

#endif //MINIDB_PARSE_H
