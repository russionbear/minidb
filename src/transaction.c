//
// Created by Administrator on 4/2/2023.
//

#include "transaction.h"
#include "utils.h"
#include <string.h>


/***************************************************************************
 * 事务 transaction
 * 调用一下函数时需要切换到数据库文件的目录
 **************************************************************************/

int get_table_id_by_name(struct D_base* base, char* name){
    int i;
    for(i=0;i<base->table_nu;i++){
        if(base->tables[i].table_id!=0&& strcmp(name, base->tables[i].name)==0){
            return i+1;
        }
    }
    return 0;
}

int get_field_id_by_pure_name(struct D_base* base, char* table_name, char* field_name){
    int i;
    int table_id = get_table_id_by_name(base, table_name);
    for(i=0;i<base->field_nu;i++){
        if(base->fields[i].table_id==table_id&& strcmp(field_name, base->fields[i].name)==0){
            return i+1;
        }
    }
    return 0;
}

void print_row_data(struct D_field* fields, int field_len, u_int8_t* row_data){
    int j =0;
    int offset = 0;
    char tmp_string[MAX_STRING_LENGTH];
    for(j=0;j<field_len;j++){
        printf("%s=", fields[j].name);
        switch (fields[j].type) {
            case BOOL:
                if((row_data+offset)[0]) printf("true"); else printf("false");
                break;
            case CHAR:
                printf("%c", (row_data+offset)[0]);
                break;
            case INT:
                printf("%d", b2int32(row_data+offset));
                break;
            case LONG:
                printf("%ld", b2int64(row_data+offset));
                break;
            case FLOAT:
                printf("%f", b2float32(row_data+offset));
                break;
            case DOUBLE:
                printf("%lf", b2float64(row_data+offset));
                break;
            case STRING:
            case BLOB:
                memcpy(tmp_string, row_data+offset, get_field_size(&fields[j]));
                tmp_string[get_field_size(&fields[j])] = 0;
                printf("%s", tmp_string);
                break;
        }

        printf(", ");
        offset += get_field_size(&fields[j]);
//        printf("\noffset=%d\n", offset);
    }
    printf("\n");
}


inline char* get_world_from_sql_s(struct SQL_STRUCTURE* sql_s, int index){
    return get_string_from_p(sql_s->full_sql+sql_s->world_offset_size[index*2], sql_s->world_offset_size[index*2+1]);
}

int get_field_from_sql_s(struct D_field* field, struct SQL_STRUCTURE* sql_s, int world_range_s, int world_range_e){
    int k1, k2, k3;
    int h1;
    char tmp_string[MAX_STRING_LENGTH];

    k1 = k2 = k3 = 0;
    while (sql_s->world_offset_size[k1*2]<world_range_s) k1++;
    while (k2!=sql_s->world_length&&sql_s->world_offset_size[k2*2+1]+sql_s->world_offset_size[k2*2]< world_range_e) k2++;

//    if(sql_s->world_offset_size[k1*2]!=world_range_s) k1--;
//    printf("k1, k2 = %d, %d\n", k1, k2);
    if((k2-k1)<2)
        return 1;
    memcpy(field->name, sql_s->full_sql+sql_s->world_offset_size[k1*2], sql_s->world_offset_size[k1*2+1]);

    h1 = 2;
    switch (get_str_hash(sql_s->full_sql+sql_s->world_offset_size[(k1+1)*2], sql_s->world_offset_size[(k1+1)*2+1])) {
        case S_KW_BOOL:
            field->type = BOOL;
            field->filed_size = get_field_size(field);
            break;
        case S_KW_INT:
            field->type = INT;
            field->filed_size = get_field_size(field);
            break;
        case S_KW_LONG:
            field->type = LONG;
            field->filed_size = get_field_size(field);
            break;
        case S_KW_FLOAT:
            field->type = FLOAT;
            field->filed_size = get_field_size(field);
            break;
        case S_KW_DOUBLE:
            field->type = DOUBLE;
            field->filed_size = get_field_size(field);
            break;
        case S_KW_CHAR:
            field->type = CHAR;
            field->filed_size = get_field_size(field);
            break;
        case S_KW_STRING:
            if((k2-k1)<3){
                k3 = 32;
            }
            else{
                memcpy(tmp_string, sql_s->full_sql+sql_s->world_offset_size[(k1+2)*2], sql_s->world_offset_size[(k1+2)*2+1]);
                tmp_string[sql_s->world_offset_size[(k1+2)*2+1]+1] = '\0';
                k3 = (int)strtol(tmp_string, 0, 10);
            }
            field->length = k3;
            field->type = STRING;
            field->filed_size = get_field_size(field);
            h1 = 3;
            break;
        case S_KW_BLOB:
            field->type = BLOB;
            field->filed_size = get_field_size(field);
            break;
        default:
            sql_s->can_be_used = 0;
            return 2;
            break;
    }

    for(h1+=k1;h1<k2;h1++){
        switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[h1 * 2],
                             sql_s->world_offset_size[h1 * 2 + 1])) {
            case S_KW_DEFAULT:
                // type
                field->default_value = calloc(field->filed_size, sizeof(char));
                memcpy(tmp_string, sql_s->full_sql+sql_s->world_offset_size[(h1+1)*2], sql_s->world_offset_size[(h1+1)*2+1]);
                tmp_string[sql_s->world_offset_size[(h1+1)*2+1]] = 0;
//                printf("nu: %ld\n", strtol(tmp_string, 0, 10));
                switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[(k1 + 1) * 2],
                                     sql_s->world_offset_size[(k1 + 1) * 2 + 1])) {
                    case S_KW_CHAR:
                    case S_KW_STRING:
                    case S_KW_BLOB:
                        memcpy(field->default_value, sql_s->full_sql+sql_s->world_offset_size[(h1+1)*2], sql_s->world_offset_size[(h1+1)*2+1]);
                        break;
                    case S_KW_BOOL:
                        uint8tob(field->default_value, (u_int8_t) strtol(tmp_string, 0 , 10));
                        break;
                    case S_KW_INT:
                        uint32tob(field->default_value, (u_int32_t) strtol(tmp_string, 0 , 10));
                        break;
                    case S_KW_LONG:
                        uint64tob(field->default_value, (u_int64_t) strtol(tmp_string, 0 , 10));
                        break;
                    case S_KW_FLOAT:
                        float32tob(field->default_value, (float32_t) strtof(tmp_string, 0));
                        break;
                    case S_KW_DOUBLE:
                        float64tob(field->default_value, (float64_t) strtod(tmp_string, 0));
                        break;
                    default:
                        sql_s->can_be_used = 0;
                        return 2;
                        break;
                }
                h1 ++;
                break;
            case S_KW_PRIMARY:
                field->field_mask |= 0b11;
                h1 ++;
                break;
            case S_KW_UNIQUE:
                field->field_mask |= 0b10;
                break;
            case S_KW_AUTO_INCREMENT:
                field->field_mask |= 0b100;
                break;
            default:
                sql_s->can_be_used = 0;
                return 2;
                break;
        }
    }

    return 0;
}

inline int str2type2b(struct D_field * field, char* s0, int s0_len, u_int8_t* b){
    switch (field->type) {
        case BOOL: uint8tob(b, (u_int8_t)strtol(s0, 0, 10));
            break;
        case INT: uint32tob(b, strtol(s0, 0, 10)); break;
        case LONG: uint64tob(b, strtol(s0, 0, 10)); break;
        case FLOAT: float32tob(b, strtof(s0, 0)); break;
        case DOUBLE: float64tob(b, strtod(s0, 0)); break;
        case CHAR: b[0] = s0[0]; break;
        case STRING:
        case BLOB: memcpy(b, s0, s0_len); break;
        default:
            return 0;
    }
    return get_field_size(field);
}

int get_field_row_data_from_sql_s(struct D_field* fields, int field_num, struct SQL_STRUCTURE* sql_s, int world_range_s, int world_range_e, u_int8_t * row_data){
    int k1, k2;
    int rw_offset = 0;
    int i;
    char tmp_string[MAX_STRING_LENGTH];

    k1 = k2 = 0;
    while (sql_s->world_offset_size[k1*2]<world_range_s) k1++;
    while (k2!=sql_s->world_length&&sql_s->world_offset_size[k2*2]<= world_range_e) k2++;

    if((k2-k1)!=field_num)
        return 1;

    for(i=0, rw_offset=0;i<field_num;i++){
        memcpy(tmp_string, sql_s->full_sql+sql_s->world_offset_size[(k1+i)*2], sql_s->world_offset_size[(k1+i)*2+1]);
        tmp_string[sql_s->world_offset_size[(k1+i)*2+1]] = 0;
        printf("[%d, %d]  %d  %s tmp_string: %s\n", sql_s->world_offset_size[(k1+i)*2], sql_s->world_offset_size[(k1+i)*2+1],
               rw_offset, fields[i].name, tmp_string);
        str2type2b(&fields[i], tmp_string, sql_s->world_offset_size[(k1+i)*2+1], row_data+rw_offset);
        rw_offset = get_field_size(&fields[i]);
    }

//    print_row_data(fields, field_num, row_data);
    return 0;
}

/* 没有校验sql语句*/

int tx_create_database(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    FILE *fp;
    char* db_name=get_world_from_sql_s(sql_s, 2);
    int rlt;
    struct D_base db;
    if((fp=fopen(db_name, "r+b"))==0){
        free(db_name);
        return 1;
    }

    memset(&db, 0, sizeof(db));
    strncpy(db.name, db_name, MAX_NAME_LENGTH);
    rlt = m_base(fp, 0, &db);

    free(db_name);
    fclose(fp);
    return rlt;
}
int tx_drop_database(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    char* db_name=get_world_from_sql_s(sql_s, 2);
    if(remove(db_name)==0){
        free(db_name);
        return 0;
    }
    free(db_name);
    return 1;
}
int tx_use_database(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    int rlt=0;
    FILE *fp;
    char* db_name=get_world_from_sql_s(sql_s, 1);

    if((fp=fopen(db_name, "rb"))==0){
        free(db_name);
        return 1;
    }

    rlt = m_base(fp, 1, &tx_manager->base);

    fclose(fp);

    free(db_name);
    if(rlt!=0){
        return 4;
    }

    return 0;
}

//int tx_show_tables(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, struct tx_result_tables* result){
//    int i, j;
//    result->num = 0;
//    for(i=0, j=0;j<tx_manager->base.table_nu;i++)
//        if(tx_manager->base.tables[i].table_id!=0){
//            j++;
//        }
//}


int tx_create_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    int i, j, k;
    int j1;
    FILE *fp;
    int rlt;
    struct D_field fields[sql_s->create_table_field_split_len+1];
    char table_name[MAX_NAME_LENGTH];

    k = 0;
    memset(fields, 0, sizeof(struct D_field)*(sql_s->create_table_field_split_len+1));
    i = sql_s->create_table_range[0];
    if(sql_s->create_table_field_split_len) j = sql_s->create_table_field_split_offset[0];
    else j = sql_s->create_table_range[1];

    do{
        rlt = get_field_from_sql_s(&fields[k], sql_s,i, j);
        if(rlt)
            return rlt;

        if(j==sql_s->create_table_range[1])
            break;

        k++;
        i = j;
        if(sql_s->create_table_field_split_offset[sql_s->create_table_field_split_len-1]==j)
            j = sql_s->create_table_range[1];
        else{
            j1 = 0;
            while(sql_s->create_table_field_split_offset[j1]!=j)j1++;
            j = sql_s->create_table_field_split_offset[j1+1];
        }
    }
    while (i!=sql_s->create_table_range[1]);

    if((fp= fopen(tx_manager->base.name, "r+b"))==0)
        return 1;

    memcpy(table_name, sql_s->full_sql+sql_s->world_offset_size[2*2], sql_s->world_offset_size[2*2+1]);
    rlt = m_create_table(fp, &tx_manager->base, table_name, fields, sql_s->create_table_field_split_len+1);
    fclose(fp);

    if(rlt)
        return rlt;

    return 0;
}
int tx_drop_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    char* table_name = get_world_from_sql_s(sql_s, 2);
    FILE *fp;
    int rlt;

    if((fp= fopen(tx_manager->base.name, "r+b"))==0){
        free(table_name);
        return 1;
    }
    rlt = m_delete_table(fp, &tx_manager->base, table_name);
    free(table_name);
    fclose(fp);
    return rlt;
}
int tx_alter_rename_table(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    char* new_table_name = get_world_from_sql_s(sql_s, 5);
    char* table_name = get_world_from_sql_s(sql_s, 2);
    FILE *fp;
    int rlt;

    if((fp= fopen(tx_manager->base.name, "r+b"))==0){
        free(table_name);
        free(new_table_name);
        return 1;
    }
    rlt = m_rename_table(fp, &tx_manager->base, table_name, new_table_name);
    free(table_name);
    free(new_table_name);
    fclose(fp);
    return rlt;
}

int tx_alter_table_add_column(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    FILE *fp;
    int rlt;
    struct D_field field;
    char* table_name;

    rlt = get_field_from_sql_s(&field, sql_s, sql_s->world_offset_size[4*2], sql_s->full_sql_length);
    if(rlt)
        return rlt;

    if((fp= fopen(tx_manager->base.name, "r+b"))==0){
        return 1;
    }
    table_name = get_world_from_sql_s(sql_s, 2);
    field.table_id = get_table_id_by_name(&tx_manager->base, table_name);

    rlt = m_add_field(fp, &tx_manager->base, &field);
    fclose(fp);

    free(table_name);
    if(!rlt)
        return rlt;
    return 0;
}
int tx_alter_table_drop_column(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    FILE *fp;
    int rlt;
    int i, j;
    char* table_name=0;
    char* field_name=0;

    if((fp= fopen(tx_manager->base.name, "r+b"))==0){
        return 1;
    }
    table_name = get_world_from_sql_s(sql_s, 2);
    field_name = get_world_from_sql_s(sql_s, 4);
    rlt = m_delete_field(fp, &tx_manager->base, get_field_id_by_pure_name(&tx_manager->base, table_name, field_name));
    fclose(fp);
    free(table_name);
    free(field_name);
    if(!rlt)
        return rlt;
    return 0;
}
int tx_alter_table_rename_column(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    FILE *fp;
    int rlt;
    int i, j;
    char* table_name=0;
    char* field_name=0;
    char* new_field_name=0;

    if((fp= fopen(tx_manager->base.name, "r+b"))==0){
        return 1;
    }

    table_name = get_world_from_sql_s(sql_s, 2);
    field_name = get_world_from_sql_s(sql_s, 5);
    new_field_name = get_world_from_sql_s(sql_s, 7);
    rlt = m_rename_field(fp, &tx_manager->base, get_field_id_by_pure_name(&tx_manager->base, table_name, field_name), new_field_name);
    fclose(fp);
    free(table_name);
    free(field_name);
    free(new_field_name);
    if(!rlt)
        return rlt;
    return 0;
}

//唯一一个支持并发的事务
int tx_select(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, struct tx_result_select* result){
    /** 1. 选出常量与变量
     *  2. 通过 AND/OR 分割出逻辑表达式
     *  3. 通过括号对 逻辑表达式一次合并
     *  4. 二次合并所有逻辑表达式
     *  5. 分割逻辑出表达式两边的运算表达式
     *  6. 通过括号对 运算表达式一次分割
     *  7. 通过
     */

    FILE *fp;
    char* table_name;
    int table_id;
    int i=0;
    struct Q_select_rows *query;
    struct D_field *fields;

    if((fp= fopen(tx_manager->base.name, "r+b"))==0){
        return 1;
    }
    table_name = get_world_from_sql_s(sql_s, 3);
    table_id = get_table_id_by_name(&tx_manager->base, table_name);

    query = calloc(1, sizeof(struct Q_select_rows)+sizeof(u_int8_t*)*MAX_SELECT_LIMIT);
    memset(query, 0, sizeof(struct Q_select_rows)+sizeof(u_int8_t*)*MAX_SELECT_LIMIT);
    query->result_length = 1024;
    query->result_offset = 0;
    query->opera_type = OP_SELECT;
    query->prim_field_len = get_table_field_num(&tx_manager->base, table_id);
    query->prim_field_offset_size = calloc(query->prim_field_len*2, sizeof(int));
    fields = get_table_fields(&tx_manager->base, table_id);
    count_field_offset_size(&tx_manager->base, table_id, fields, query->prim_field_len, query->prim_field_offset_size);
    query->prim_field_row_size = count_row_size(fields, query->prim_field_len, table_id);

    query->need_field_len = query->prim_field_len;
    query->need_field_row_size = query->prim_field_row_size;
    query->need_field_offset_size = query->prim_field_offset_size;

    m_iter_page(fp, &tx_manager->base, 0, table_id, m_i_query_rows2, query);
    if(result!=0){
        result->num = query->result_offset;
        result->data = calloc(query->need_field_row_size* query->result_offset, sizeof(u_int8_t));
        for(i=0;i<result->num;i++)
            memcpy(result->data+query->need_field_row_size*i, query->result_data[i], query->need_field_row_size);
    }
    return 0;
}
int tx_insert(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    FILE *fp;
    int i, j, j1, j2, k, field_num;
    int table_id;
    int kw_value_index;
    int rlt;
    char* table_name;
    struct D_field* fields;
    struct Q_insert_rows* query;

    table_name = get_world_from_sql_s(sql_s, 2);
    table_id = get_table_id_by_name(&tx_manager->base, table_name);

    for(i=3, kw_value_index = 3;i<sql_s->world_length;i++){
        if(get_str_hash(sql_s->full_sql+sql_s->world_offset_size[2*i], sql_s->world_offset_size[2*i+1])==VALUE
           &&sql_s->full_sql[sql_s->world_offset_size[2*i-1]]!='\\') {
            kw_value_index = i;
            break;
        }
    }
    // 所有字段
    if(kw_value_index==3){
        field_num = get_table_field_num(&tx_manager->base, table_id);
        fields = get_table_fields(&tx_manager->base, table_id);
    }
    else{ // 暂不考虑排序问题
        field_num = kw_value_index - 3 + 1;
        fields = calloc(field_num, sizeof(struct D_field));
        k = 0;
        for(j=0, j1=0;j1<tx_manager->base.field_nu;j++){
            if(tx_manager->base.fields[j].table_id==0)
                continue;
            j1++;
            for(i=3;i<=kw_value_index;i++){
                if(tx_manager->base.fields[j].table_id==table_id&&
                   is_string_equal(tx_manager->base.fields[j].name,
                                   sql_s->full_sql+sql_s->world_offset_size[i*2],
                                   sql_s->world_offset_size[i*2+1], 0)
                        ){
                    memcpy(&fields[k++], &tx_manager->base.fields[j], sizeof(struct D_field));
                }
            }
        }
    }

    // 先不校验数据
    query = calloc(1, sizeof(struct Q_insert_rows)+sizeof(u_int8_t*)*sql_s->insert_value_length);
    query->insert_data_length = sql_s->insert_value_length;
    query->current_offset = 0;
    query->row_size = count_row_size(fields, field_num, table_id);
    query->max_page_rows = (int)count_page_max_rows(query->row_size);

    if((fp= fopen(tx_manager->base.name, "r+b"))==0)
        return 1;
    query->write_fp = fp;

    for(i=0, k=0;i<sql_s->insert_value_length;i++){
        query->insert_data[k] = calloc(1, query->row_size);
        rlt = get_field_row_data_from_sql_s(fields, field_num, sql_s, sql_s->insert_value_ranges[i*2],
                                            sql_s->insert_value_ranges[i*2+1],
                                            query->insert_data[k]);
        if(rlt){
            free(table_name);
            free(fields);
            free(query);
            fclose(fp);
            return rlt;
        }
        k++;
    }

    rlt = m_iter_page(fp, &tx_manager->base, 1, table_id, m_i_insert_rows, query);

    free(table_name);
    free(fields);
    free(query);
    fclose(fp);
    return rlt;
}
int tx_delete(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    FILE *fp;
    char *table_name = get_world_from_sql_s(sql_s, 2);
    int table_id = get_table_id_by_name(&tx_manager->base, table_name);
    int i=0, j, k=0;
    int rlt;

    if((fp=fopen(tx_manager->base.name, "r+b"))){
        free(table_name);
        return 1;
    }
    for(i=0, j=0;j<tx_manager->base.page_nu;i++){
        if(tx_manager->base.page_ids[i]==table_id){
            tx_manager->base.page_ids[i] = 0;
            k++;
        }
    }
    tx_manager->base.page_nu-=k;

    rlt = m_base(fp, 0, &tx_manager->base);
    free(table_name);
    return rlt;
}
int tx_update(struct sql_transaction_manager* tx_manager, struct SQL_STRUCTURE* sql_s, void* result){
    return 1;
}

int run_sql(char* input_str, struct sql_transaction_manager* tx_manager, void* result){
    int rlt;
    struct D_base* base = &tx_manager->base;
    struct SQL_STRUCTURE *sql_s = calloc(1, sizeof(struct SQL_STRUCTURE));
    sql_s->full_sql = input_str;
    sql_s->full_sql_length = (int)strlen(input_str);

    rlt = pare_sql_from_string(sql_s, &tx_manager->base);
    if(rlt||!sql_s->can_be_used)
        return 1;

    print_re_worlds(sql_s->world_length, sql_s->world_offset_size, sql_s->full_sql);

    switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[2 * 0 + 0], sql_s->world_offset_size[2 * 0 + 1])) {
        case S_KW_CREATE:
            switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[1*2], sql_s->world_offset_size[1*2+1])) {
                case S_KW_DATABASE:
                    rlt = tx_create_database(tx_manager, sql_s, 0);
                    break;
                case S_KW_TABLE:
                    rlt = tx_create_table(tx_manager, sql_s, 0);
                    if(rlt)
                        return rlt;
                    break;
                default:
                    sql_s->can_be_used = 0;
                    break;
            }
            break;
        case S_KW_DROP:
            switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[1 * 2],
                                 sql_s->world_offset_size[1 * 2 + 1])) {
                case S_KW_DATABASE:
                    rlt = tx_drop_database(tx_manager, sql_s, 0);
                    break;
                case S_KW_TABLE:
                    rlt = tx_drop_table(tx_manager, sql_s, 0);
                    break;
                default:
                    sql_s->can_be_used = 0;
                    break;
            }
            break;
        case S_KW_USE:
            rlt = tx_use_database(tx_manager, sql_s, 0);
            break;
        case S_KW_ALTER:
            switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[3 * 2],
                                 sql_s->world_offset_size[3 * 2 + 1])) {
                case S_KW_RENAME:
                    if(get_str_hash(sql_s->full_sql+sql_s->world_offset_size[4*2], sql_s->world_offset_size[4*2+1])==S_KW_TO){
                        rlt = tx_alter_rename_table(tx_manager, sql_s, 0);
                    }else{
                        rlt = tx_alter_table_rename_column(tx_manager, sql_s, 0);
                    }
                    break;
                case S_KW_ADD:
                    rlt = tx_alter_table_add_column(tx_manager, sql_s, 0);
                    break;
                case S_KW_DROP:
                    rlt = tx_alter_table_drop_column(tx_manager, sql_s, 0);
                    break;
                default:
                    sql_s->can_be_used = 0;
            }
            break;
        case S_KW_SELECT:
            rlt = tx_select(tx_manager, sql_s, result);
            break;
        case S_KW_INSERT:
            rlt = tx_insert(tx_manager, sql_s, 0);
            break;
        case S_KW_UPDATE:
            rlt = tx_update(tx_manager, sql_s, 0);
            break;
        case S_KW_DELETE:
            rlt = tx_delete(tx_manager, sql_s, 0);
            break;
        default:
            sql_s->can_be_used = 0;
            break;
    }

    if(!sql_s->can_be_used)
        rlt = 90;

    return rlt;
}

int print_view_data(struct D_field* fields, int field_len, struct tx_result_select* view_data){
    int i=0, j;
    int offset=0;
    char tmp_string[MAX_STRING_LENGTH];
    for(;i<view_data->num;i++){
        for(j=0;j<field_len;j++){
            printf("%s=", fields[j].name);
            switch (fields[j].type) {
                case BOOL:
                    if((view_data->data+offset)[0]) printf("true"); else printf("false");
                    break;
                case CHAR:
                    printf("%c", (view_data->data+offset)[0]);
                    break;
                case INT:
                    printf("%d", b2int32(view_data->data+offset));
                    break;
                case LONG:
                    printf("%ld", b2int64(view_data->data+offset));
                    break;
                case FLOAT:
                    printf("%f", b2float32(view_data->data+offset));
                    break;
                case DOUBLE:
                    printf("%lf", b2float64(view_data->data+offset));
                    break;
                case STRING:
                case BLOB:
                    memcpy(tmp_string, view_data->data+offset, get_field_size(&fields[j]));
                    tmp_string[get_field_size(&fields[j])] = 0;
                    printf("%s", tmp_string);
                    break;
            }

            printf(", ");
            offset += get_field_size(&fields[j]);
//            printf("\noffset=%d\n", offset);
        }
        printf("\n");
    }
}

int test_equation(){
    int i, j, k;

    int world_len;
    int *world_offset_size;
    int nu_num;
    int *nu_index;
    int symbol_num;
    int *symbol_index;
    int *symbol_order;

    char test_string[32] = "3*3/5%78 + 12";
    char tmp_string[MAX_STRING_LENGTH];
    int range[2];


    range[0] = 0;
    range[1] = strlen(test_string);

    world_len = count_appeared_nu(test_string, "(\\w+)|([*+-/%=!><]+)",0, 0);
    world_offset_size = calloc(world_len*2, sizeof(int));
    count_appeared_nu(test_string, "(\\w+)|([*+-/%=!><]+)",world_offset_size, 0);

    for(i=0, nu_num=0, symbol_num=0;i<world_len;i++){
        memcpy(tmp_string, test_string + world_offset_size[i*2], world_offset_size[i*2+1]);
        tmp_string[world_offset_size[i*2+1]] = 0;
//        printf("%d, %d: %s\n", world_offset_size[i*2], world_offset_size[i*2+1], tmp_string);
        switch (tmp_string[0]) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
                symbol_num ++;
                break;
            default:
                nu_num ++;
        }
    }

    nu_index = calloc(nu_num, sizeof(int));
    symbol_index = calloc(symbol_num, sizeof(int));
    symbol_order = calloc(symbol_num, sizeof(int));

    for(i=0, j=0, k=0;i<world_len;i++){
        memcpy(tmp_string, test_string + world_offset_size[i*2], world_offset_size[i*2+1]);
        tmp_string[world_offset_size[i*2+1]] = 0;
//        printf("%d, %d: %s\n", world_offset_size[i*2], world_offset_size[i*2+1], tmp_string);
        switch (tmp_string[0]) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
                symbol_index[j++] = i;
                break;
            default:
                nu_index[k++] = i;
        }
    }

    // 排序
    memset(symbol_order, 0, sizeof(int)*symbol_num);
    for(i=0;i<symbol_num;i++){
        switch (test_string[symbol_index[i]]) {
            case '*':
            case '/':
            case '%':
                symbol_order[0] = symbol_order[i];
                i = -1;
        }
        if(i==-1)
            break;
    }
}

/** join select*/

//// 0 ok, 1, end, 2 exit, 3 empty
//int m_yield_table_rows(
//        FILE *fp, struct D_base* base, int table_id, int *field_offset_size, int field_len, struct Q_yield_func_args* yield_args
//){
//    int i=yield_args->y_page_b_offset, j=yield_args->y_row_b_offset, k=0;
//    u_int64_t rlt;
//
//    if(yield_args->y_page_b_offset==-1)
//        return 1;
//
//    for(;base->page_ids[i]!=0;i++)
//        if(base->page_ids[i]==table_id)
//            break;
//
//    if(base->page_ids[i]==0){
//        yield_args->y_page_b_offset =-1;
//        return 1;
//    }
//    yield_args->y_page_b_offset = i;
//
//
//    if(yield_args->y_page==0){
//        yield_args->y_page = calloc(1, yield_args->y_page_header_size);
//        m_page(fp, 1, base, i, yield_args->y_page);
//    }
//
//    for(;j<yield_args->y_page_max_rows;j++)
//        if(yield_args->y_page->m_row_mask[j]!=0)
//            break;
//
//    yield_args->y_row_b_offset = j;
//
//    if(j>=yield_args->y_page_max_rows)
//    {
//        free(yield_args->y_page);
//        yield_args->y_page = 0;
//        yield_args->y_row_b_offset = 0 ;
//        yield_args->y_page_b_offset ++;
//        return 3;
//    }
//
//    rlt = m_row_field(fp, 1, base, i, j, field_offset_size, field_len, yield_args->y_row_data);
//
//    if(rlt!=0)
//        return 2;
//
//    yield_args->y_row_b_offset ++;
//    return 0;
//}


//
//// 0: ok, 1: full, 2: error
//int m_f_join_select_rows(struct Q_select_rows* select_where, int* field_offset_size, int field_len, u_int8_t* row_data){
//    int h=0, h1, tmp_data_offset, row_data_offset;
//
//    struct R_arith_node* tmp_arith_node=0;
//
//    if(select_where->result_offset==select_where->result_length) // 一遍能被yield调用
//        return 1;
//
//    /// refresh cache data
//    // refresh field _data
//    for(h=0, h1=select_where->condition.field_value_offset_len[0], tmp_data_offset=0, row_data_offset=0;h<field_len;h++){
//        if(field_offset_size[h*3]==select_where->condition.nodes[h1].field_id){
//            memcpy(select_where->condition.nodes[h1].value+tmp_data_offset, row_data+row_data_offset, field_offset_size[h*3+1]);
//            h1++;
//            tmp_data_offset += field_offset_size[h*3+2];
//        }
//        row_data_offset += field_offset_size[h*3+2];
//    }
//    // refresh middle data
//    for(h=select_where->condition.mid_value_offset_len[0];h<select_where->condition.mid_value_offset_len[1];h++){
//        tmp_arith_node = &select_where->condition.nodes[h];
//        b_arith_opera(
//                &select_where->condition.nodes[tmp_arith_node->param_node_ids[0]].custom_field,
//                &select_where->condition.nodes[tmp_arith_node->param_node_ids[1]].custom_field,
//                tmp_arith_node->type,
//                select_where->condition.nodes[tmp_arith_node->param_node_ids[0]].value,
//                select_where->condition.nodes[tmp_arith_node->param_node_ids[1]].value,
//                tmp_arith_node->value
//        );
//    }
//    // refresh logical data
//    if(tmp_arith_node==0)
//        return 2;
//    if(tmp_arith_node->value[0]){
//        // 提取需要的字段
//        row_data_offset = 0;
//        tmp_data_offset = 0;
//        select_where->result_data[select_where->result_offset] = calloc(1, select_where->need_field_row_size);
//        for(h=0;h<field_len;h++){
//            row_data_offset += field_offset_size[h*3+1];
//            if(field_offset_size[h*3]!=select_where->need_field_offset_size[h1*3])
//                continue;
//            memcpy(
//                    select_where->result_data[select_where->result_offset]+tmp_data_offset,
//                    row_data+row_data_offset,
//                    field_offset_size[h*3+2]);
//            tmp_data_offset += field_offset_size[h*3+2];
//            h1++;
//            if(h1==select_where->need_field_len)
//                break;
//        }
//        select_where->result_offset ++;
//
//        if(select_where->result_offset==select_where->result_length)
//            return 1;
//    }
//    return 0;
//}
//
//
//int m_join_select(struct D_base* base, struct Q_join_select* join_select){
//    int i, j, k;
//    u_int64_t rlt;
//    u_int8_t* query_row_data;
//    int n_for_mark[join_select->joined_table_nu];
//    //初始化yield
//    struct Q_yield_func_args yield_args[join_select->joined_table_nu];
//    for(i=0, k=0;i<join_select->joined_table_nu;i++){
//        yield_args[i].y_page_max_rows = (int)count_page_max_rows(base->tables[join_select->table_ids[i]].row_size);
//        yield_args[i].y_page_header_size = (int)count_page_header_size(base->tables[join_select->table_ids[i]].row_size);
//        yield_args[i].y_query_row_size = 0;
//        for(j=0;j<join_select->table_fields[i].need_field_len;j++)
//            yield_args[i].y_query_row_size += join_select->table_fields[i].need_field_offset_size[j*3+2];
//        k += yield_args[i].y_query_row_size;
//        yield_args[i].y_page_b_offset = 0;
//        yield_args[i].y_row_b_offset = 0;
//        yield_args[i].y_page = 0;
//    }
//
//    query_row_data = calloc(1, k);
//    for(i=0, k=0;i<join_select->joined_table_nu;i++) {
//        yield_args[i].y_row_data = query_row_data+k;
//        k += yield_args[i].y_query_row_size;
//    }
//
//    memset(query_row_data, 0, sizeof(int)*join_select->joined_table_nu);
//    rlt=0;
////    while(rlt==0){
//////        rlt =
////    }
//}
//
//
//int m_join2table_select(struct D_base* base, struct Q_join_select* join_select){
//    int i, j, k;
//    int rlt;
////    u_int8_t has1, has2;
//    u_int8_t* query_row_data;
//    //初始化yield
//    struct Q_yield_func_args yield_args[join_select->joined_table_nu];
//    for(i=0, k=0;i<join_select->joined_table_nu;i++){
//        yield_args[i].y_page_max_rows = (int)count_page_max_rows(base->tables[join_select->table_ids[i]].row_size);
//        yield_args[i].y_page_header_size = (int)count_page_header_size(base->tables[join_select->table_ids[i]].row_size);
//        yield_args[i].y_query_row_size = 0;
//        for(j=0;j<join_select->table_fields[i].need_field_len;j++)
//            yield_args[i].y_query_row_size += join_select->table_fields[i].need_field_offset_size[j*3+2];
//        k += yield_args[i].y_query_row_size;
//        yield_args[i].y_page_b_offset = 0;
//        yield_args[i].y_row_b_offset = 0;
//        yield_args[i].y_page = 0;
//    }
//
//    query_row_data = calloc(1, k);
//    memset(query_row_data, 0, sizeof(int)*join_select->joined_table_nu);
//    for(i=0, k=0;i<join_select->joined_table_nu;i++) {
//        yield_args[i].y_row_data = query_row_data+k;
//        k += yield_args[i].y_query_row_size;
//    }
//
//    rlt = 0;
////    has1 = has2 = 0;
//
//    do{
//        rlt = m_yield_table_rows(
//                join_select->fps[1], base, join_select->table_ids[1],
//                join_select->table_fields[1].need_field_offset_size,
//                join_select->table_fields[1].need_field_len, &yield_args[1]);
//    } while (rlt==3);
//    if(rlt==1||rlt==2){
//        free(query_row_data);
//        return 0;
//    }
//
//    while(rlt==0){
//        do{
//            rlt = m_yield_table_rows(
//                    join_select->fps[0], base, join_select->table_ids[0],
//                    join_select->table_fields[0].need_field_offset_size,
//                    join_select->table_fields[0].need_field_len, &yield_args[0]);
//        } while (rlt==3);
//        if(rlt==1){
//            if(yield_args[0].y_page!=0){
//                free(yield_args[0].y_page);
//                yield_args[0].y_page = 0;
//            }
//            yield_args[0].y_page_b_offset = 0;
//            yield_args[0].y_row_b_offset = 0;
//
//
//            do{
//                rlt = m_yield_table_rows(
//                        join_select->fps[1], base, join_select->table_ids[1],
//                        join_select->table_fields[1].need_field_offset_size,
//                        join_select->table_fields[1].need_field_len, &yield_args[1]);
//            } while (rlt==3);
//            if(rlt==1||rlt==2){
//                return 0;
//            }
//            continue;
//
//        }
//        else if(rlt==0){
////            rlt = m_f_join_select_rows(join_select->result_row_field, )
//// stop develop join select
//            continue;
//        }else{
//            break;
//        }
//
//    }
//    free(query_row_data);
//    return 0;
//}


// ------------------------------------


