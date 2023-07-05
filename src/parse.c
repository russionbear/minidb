//
// Created by 19545 on 28/03/2023.
// sql 语句解析相关函数
// 其实可以用正则来实现，但这里没有
//


/*
## sql语句解析示例
```

create database dbname;
drop databse dbname;
use dbname;

show databvases;
show tables;
DESC table_name;

CREATE TABLE table_name
(
column_name1 data_type(size) unique auto_increment,
column_name2 data_type(size) default '',
column_name3 data_type(size) PRIMARY KEY,
);
alter table table_name rename to new_name;
drop table table_name;

alter table table_name rename column oldname to newname
alter table table_name add/drop field_name XXXX;

select * from table where field1 = '' and field2 = '' and field3 + field4 > 6 limit orderby filed desc ;
select field1, field2 from table where field1 = '' and field2 = '' and field3 + field4 > 6;
// select count(field) as field1 as,  sum(field) as field2 as , avg(field) as 99

insert into table values (v1, v2), ();
insert into table(field1, field2) valuse(v1, v2);

update table set field1=v1, field2=field1+1, where ...;

delete from table where ...
```




 */




#include <regex.h>
#include <string.h>
#include "utils.h"
#include "transaction.h"
#include <ctype.h>

// //////////////////////////////
// 每个函数对应相应类型的sql语句解析//
// ///////////////////////////////

/**
 *
 * */
int parse_create_table_from_sql(struct SQL_STRUCTURE * sql_s, struct D_base *base){
    int bracket_num;
    int *bracket_offset=0;
    int i, j;
    bracket_num = count_appeared_nu(sql_s->full_sql, "[()]", 0, 1);
    if(bracket_num<2)
        return 1;
    bracket_offset = calloc(bracket_num, sizeof(int));
    count_appeared_nu(sql_s->full_sql, "[()]", bracket_offset, 1);

    // 排除字符串中的 字符
    for(i=0;i<bracket_num;i++)
        for(j=0;j<sql_s->world_length;j++){
            if(bracket_offset[i]<sql_s->world_offset_size[j*2])
                continue;
            else if(bracket_offset[i]<sql_s->world_offset_size[j*2+1]){
                bracket_offset[i] = -1;
                break;
            }
        }

    // 选值
    i = 0;
    while(bracket_offset[i]==-1) i++;
    sql_s->create_table_range[0] = bracket_offset[i];
    i = bracket_num-1;
    while(bracket_offset[i]==-1) i--;
    sql_s->create_table_range[1] = bracket_offset[bracket_num-1];

    bracket_num = count_appeared_nu(sql_s->full_sql, ",", 0, 1);
    free(bracket_offset);
    bracket_offset = calloc(bracket_num, sizeof(int));
    count_appeared_nu(sql_s->full_sql, ",", bracket_offset, 1);

    sql_s->create_table_field_split_len = 0;
    // 排除字符串中的 字符
    for(i=0;i<bracket_num;i++){
        if(bracket_offset[i]<sql_s->create_table_range[0]||bracket_offset[i]>sql_s->create_table_range[1]){
            bracket_offset[i] = -1;
            continue;
        }
        for(j=0;j<sql_s->world_length;j++){
            if(bracket_offset[i]<sql_s->world_offset_size[j*2])
                continue;
            else if(bracket_offset[i]<sql_s->world_offset_size[j*2+1]){
                bracket_offset[i] = -1;
                j = -j;
                break;
            }
        }
        if(j>0)
            sql_s->create_table_field_split_len ++;
    }

    // 赋值
    sql_s->create_table_field_split_offset = calloc(sql_s->create_table_field_split_len, sizeof(int));
    for(i=0, j=0;i<bracket_num;i++)
        if(bracket_offset[i]!=-1)
            sql_s->create_table_field_split_offset[j++] = bracket_offset[i];
    free(bracket_offset);

    return 0;
}

int parse_select_from_sql(struct SQL_STRUCTURE * sql_s, struct D_base *base){
    return 0;
}

int parse_insert_from_sql(struct SQL_STRUCTURE * sql_s, struct D_base *base){
    int bracket_num;
    int *bracket_offset=0;
    int i, j, k;
    int kw_value_index;
    bracket_num = count_appeared_nu(sql_s->full_sql, "[()]", 0, 1);
    if(bracket_num<2)
        return 1;
    bracket_offset = calloc(bracket_num, sizeof(int));
    count_appeared_nu(sql_s->full_sql, "[()]", bracket_offset, 1);

    for(i=3, kw_value_index = 3;i<sql_s->world_length;i++){
        if(get_str_hash(sql_s->full_sql+sql_s->world_offset_size[2*i], sql_s->world_offset_size[2*i+1])==VALUE
           &&sql_s->full_sql[sql_s->world_offset_size[2*i-1]]!='\\') {
            kw_value_index = i;
            break;
        }
    }
    sql_s->use_custom_fields = kw_value_index != 3;

    // 排除字符串中的 字符
    for(i=0, k = bracket_num;i<bracket_num;i++)
        for(j=0;j<sql_s->world_length;j++){
            if(bracket_offset[i]<sql_s->world_offset_size[j*2])
                continue;
            else if(bracket_offset[i]<sql_s->world_offset_size[j*2+1]){
                bracket_offset[i] = -1;
                k--;
                break;
            }
        }
    if(k<2){
        free(bracket_offset);
        return 1;
    }

    // 选值
    i = 0;
    if(sql_s->use_custom_fields){
        while(bracket_offset[i]==-1) i++;
        sql_s->insert_field_range[0] = bracket_offset[i];
        i++;
        while(bracket_offset[i]==-1) i++;
        sql_s->insert_field_range[1] = bracket_offset[i];
        i++;

        sql_s->insert_value_length = (k-2)/2;
    }
    else{
        sql_s->insert_value_length = k/2;
    }

    sql_s->insert_value_ranges = calloc(sql_s->insert_value_length*2, sizeof(int));
    for(j=0;j<sql_s->insert_value_length*2;j++){
        while(bracket_offset[i]==-1) i++;
        sql_s->insert_value_ranges[j] = bracket_offset[i++];
    }

    free(bracket_offset);
    return 0;
}

int parse_where_from_sql(struct SQL_STRUCTURE * sql_s, struct D_base *base){
    return 0;
}

/**
 * 初次从字符串中解析sql语句，并调用上面的函数进行进一步解析
 */
int pare_sql_from_string(struct SQL_STRUCTURE * sql_s, struct D_base *base){
    int i, j, k1;

    int split_count = 0;
    int *split_index=0;
    int world_count = 0;
    int* world_offset_size=0;
    int rlt=0;
    char* tmp_string1=0, *tmp_string2=0, *tmp_string3;
    FILE *fp;

    // 提取字符串
    split_count = count_appeared_nu(sql_s->full_sql, "[^\\]'", 0, 1);
    if(split_count%2){
        sql_s->can_be_used = 0;
        return 1;
    }
    split_index = calloc(split_count, sizeof(int));
    count_appeared_nu(sql_s->full_sql, "[^\\]'", split_index, 1);

    // 提取单词 和 符号
    world_count = count_appeared_nu(sql_s->full_sql, "(\\w+)|([*+-/%=!><]+)", 0, 0);
    if(world_count<=1){
        free(split_index);
        return 2;
    }
    world_offset_size = calloc(world_count*2, sizeof(int));
    count_appeared_nu(sql_s->full_sql, "(\\w+)|([*+-/%=!><]+)", world_offset_size, 0);

    // 排除字符串中的字符串
    sql_s->world_length = 0;
    for(i=0;i<world_count;i++){
        for(j=0;j<split_count/2;j++){
            if(split_index[j*2]<world_offset_size[i*2]&&split_index[j*2+1]>=world_offset_size[i*2]+world_offset_size[i*2+1]){
                world_offset_size[i*2] = -1;
                world_offset_size[i*2+1] = 0;
                break;
            }else if(world_offset_size[i*2+1]==1&&sql_s->full_sql[world_offset_size[i*2]]==','){
                world_offset_size[i*2] = -1;
                world_offset_size[i*2+1] = 0;
                break;
            }
        }
        if(world_offset_size[i*2]!=-1)
            sql_s->world_length ++;
    }

    // 将 字符串 和 world 的位置写入sql_structure
    sql_s->world_offset_size = calloc(sql_s->world_length*2, sizeof(int));
    sql_s->str_value_mark_arr = calloc(sql_s->world_length, sizeof(int));
    memset(sql_s->str_value_mark_arr, 0, sizeof(unsigned char)*sql_s->world_length);
    i=0; j=0; k1=0;
    if(split_count==0){ // 没有字符串
        for(;i<world_count;i++){
            if(world_offset_size[i*2]==-1)
                continue;
            if(world_offset_size[i*2+1]==1&&sql_s->full_sql[world_offset_size[i*2]]==','){
                continue;
            }
            sql_s->world_offset_size[j*2] = world_offset_size[i*2];
            sql_s->world_offset_size[j*2+1] =  world_offset_size[i*2+1];
//            if()
            j++;
        }
        sql_s->world_length = j;
    }else{
        while((i!=world_count||j<(split_count/2))){
            if(j<(split_count/2)&&(i==world_count||world_offset_size[i*2]>split_index[j*2])){
                // 排除空字符串
                if(split_index[j*2+1]-split_index[j*2]-1==0){
                    j++;
                    continue;
                }

                sql_s->world_offset_size[k1*2] = split_index[j*2]+1;
                sql_s->world_offset_size[k1*2+1] = split_index[j*2+1]-split_index[j*2]-1;

                sql_s->str_value_mark_arr[k1] = 1;

                j++;
                k1++;
            }
            else
            {
                if(world_offset_size[i*2]==-1){
                    i++;
                    continue;
                }

                sql_s->world_offset_size[k1*2] = world_offset_size[i*2];
                sql_s->world_offset_size[k1*2+1] =  world_offset_size[i*2+1];
                i++;
                k1++;
            }
        }

        sql_s->world_length = k1;
    }
    // free mem
    free(world_offset_size);
    world_offset_size = 0;
    free(split_index);
    split_index = 0;

    sql_s->can_be_used = 1;
    switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[0], sql_s->world_offset_size[1])) {
        case S_KW_CREATE:
            switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[1*2], sql_s->world_offset_size[1*2+1])) {
                case S_KW_DATABASE:
                    if(sql_s->world_length<3)
                        return 1;

                    tmp_string1 = calloc(sql_s->world_offset_size[2 * 2 + 1] + 1, sizeof(char));
                    memcpy(tmp_string1, sql_s->full_sql + sql_s->world_offset_size[2 * 2], sql_s->world_offset_size[2 * 2 + 1]);
                    tmp_string1[sql_s->world_offset_size[2 * 2 + 1]] = '\0';
                    create_database(tmp_string1);
                    free(tmp_string1);
                    break;
                case S_KW_TABLE:
                    rlt = parse_create_table_from_sql(sql_s, base);
                    if(rlt)
                        return rlt;
                    break;
                default:
                    break;
            }
            break;
        case S_KW_DROP:
        case S_KW_USE:
            break;
        case S_KW_ALTER:
            if(sql_s->world_length < 4||base->name[0]==0)
                return 1;
            switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[3*2], sql_s->world_offset_size[3*2+1])) {
                case S_KW_RENAME:
                    // rename table name
//                    if(sql_s->world_length==6){
//
//                        // table name
//                        tmp_string1 = calloc(sql_s->world_offset_size[2 * 2 + 1] + 1, sizeof(char));
//                        memcpy(tmp_string1, sql_s->full_sql + sql_s->world_offset_size[2 * 2], sql_s->world_offset_size[2 * 2 + 1]);
//                        tmp_string1[sql_s->world_offset_size[2 * 2 + 1]] = '\0';
//
//                        // new table name
//                        tmp_string2 = calloc(sql_s->world_offset_size[2 * 5 + 1] + 1, sizeof(char));
//                        memcpy(tmp_string2, sql_s->full_sql + sql_s->world_offset_size[2 * 5], sql_s->world_offset_size[2 * 5 + 1]);
//                        tmp_string2[sql_s->world_offset_size[2 * 5 + 1]] = '\0';
//
//                        if((fp= fopen(base->name, "r+b"))==0){
//                            free(tmp_string1);
//                            free(tmp_string2);
//                            tmp_string1 =  0; tmp_string2 = 0;
//                            return 1;
//                        }
//                        rlt = m_rename_table(fp, base, tmp_string1, tmp_string2);
//                        free(tmp_string1);
//                        free(tmp_string2);
//                        tmp_string1 =  0; tmp_string2 = 0;
//                        fclose(fp);
//                        if(rlt)
//                            return rlt;
//                    }
//                    else if(sql_s->world_length==8){
//                        // table name
//                        tmp_string1 = calloc(sql_s->world_offset_size[2 * 2 + 1] + 1, sizeof(char));
//                        memcpy(tmp_string1, sql_s->full_sql + sql_s->world_offset_size[2 * 2], sql_s->world_offset_size[2 * 2 + 1]);
//                        tmp_string1[sql_s->world_offset_size[2 * 2 + 1]] = '\0';
//
//                        // field name
//                        tmp_string2 = calloc(sql_s->world_offset_size[2 * 5 + 1] + 1, sizeof(char));
//                        memcpy(tmp_string2, sql_s->full_sql + sql_s->world_offset_size[2 * 5], sql_s->world_offset_size[2 * 5 + 1]);
//                        tmp_string2[sql_s->world_offset_size[2 * 5 + 1]] = '\0';
//
//                        // new field name
//                        tmp_string2 = calloc(sql_s->world_offset_size[2 * 7 + 1] + 1, sizeof(char));
//                        memcpy(tmp_string2, sql_s->full_sql + sql_s->world_offset_size[2 * 7], sql_s->world_offset_size[2 * 7 + 1]);
//                        tmp_string2[sql_s->world_offset_size[2 * 7 + 1]] = '\0';
//
//                        if((fp= fopen(base->name, "r+b"))==0){
//                            free(tmp_string1);
//                            free(tmp_string2);
//                            free(tmp_string3);
//                            tmp_string1 =  0; tmp_string2 = 0; tmp_string3 = 0;
//                            return 1;
//                        }
//                        rlt = m_rename_field(fp, base,
//                                             get_field_id_by_pure_name(base, tmp_string1, tmp_string2),
//                                             tmp_string3);
//                        free(tmp_string1);
//                        free(tmp_string2);
//                        free(tmp_string3);
//                        tmp_string1 =  0; tmp_string2 = 0; tmp_string3 = 0;
//                        fclose(fp);
//                        if(rlt)
//                            return rlt;
//                    }
                    break;
                case S_KW_ADD:
                    break;
                case S_KW_DROP:

                    break;
                default:
                    sql_s->can_be_used = 0;
            }
            break;
        case S_KW_SELECT:
            rlt = parse_select_from_sql(sql_s, base);
            break;
        case S_KW_INSERT:
            rlt = parse_insert_from_sql(sql_s, base);
            break;
        case S_KW_UPDATE:
            break;
        case S_KW_DELETE:
            break;
        default:
            sql_s->can_be_used = 0;
    }
    return rlt;
}

void free_sql_structure(struct SQL_STRUCTURE * sql_s){

}

// 测试正则
int learn_regex(){

    printf("%d\n", get_str_hash("*", 1));
    /** 待匹配字符串 */
    char inputstr[128] = "'hello,'111\\'wel(come) '**222'to my party";
    /** regex 错误输出缓冲区 */
    char regerrbuf[256];
    regex_t reg;
    /** 正则表达式 */
    const char* pattern = "(\\w+)|([*+-/%=!><]+)"; //  [^\]'
    int match_offset_size[200];
    int match_length = 0;
    int i=0;
    match_length = count_appeared_nu(inputstr, pattern, match_offset_size, 0);

    printf("match_length: %d\n", match_length);
    for(;i<match_length;i++)
        printf("%d:%d\n", match_offset_size[i*2], match_offset_size[i*2+1]);

    return 0;

}
