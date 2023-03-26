#include "table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OPEN_LOG

#ifdef OPEN_LOG
#include <syslog.h>
#endif



// tool

int get_random(int min, int max, int *not_in_arr, int not_in_arr_len){
    int a;
    int i;
    struct timespec timestamp;

    while(1){
        clock_gettime(CLOCK_REALTIME, &timestamp);
        srand((unsigned)timestamp.tv_nsec);
        a = rand() % (max-min) + min;
        if(not_in_arr==0)
            return a;

        for(i=0;i<not_in_arr_len;i++){
            if(not_in_arr[i]==a)
                break;
        }

        if(i==not_in_arr_len)
            return a;
    }
}


// memory operation

int m_base(FILE *fp, u_int8_t is_read, struct D_base* base){
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp, 0, SEEK_SET);
    if(is_read){
        if(fread(base, (u_int32_t)sizeof(struct D_base), 1, fp)!=1)
            return 2;
    }else{
        if(fwrite(base, (u_int32_t)sizeof(struct D_base), 1, fp)!=1)
            return 2;
    }

    return 0;
}
int m_base_info(FILE *fp, u_int8_t is_read, struct D_base* base){
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp, 0, SEEK_SET);
    if(is_read){
        if(fread(base, sizeof(base->name)+sizeof(int)*2, 1, fp)!=1)
            return 2;
    }else{
        if(fwrite(base, sizeof(base->name)+sizeof(int)*2, 1, fp)!=1)
            return 2;
    }

    return 0;
}
int m_base_page_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int start_index, int length){
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp, (int32_t)(sizeof(struct D_base)-sizeof(u_int8_t)*MAX_PAGE_NU+sizeof(u_int8_t)*start_index), SEEK_SET);
    if(is_read){
        if(fread(base->page_ids+start_index, sizeof(u_int8_t), length, fp)!=length)
            return 2;
    }else{
        if(fwrite(base->page_ids+start_index, sizeof(u_int8_t), length, fp)!=length)
            return 2;
    }

    return 0;
}

int m_table(FILE *fp, u_int8_t is_read, struct D_base* base, int table_index){
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp, (u_int32_t)(sizeof(base->name)+sizeof(int)*2+table_index*sizeof(struct D_table)), SEEK_SET);
    if(is_read){
        if(fread(&base->tables[table_index], sizeof(struct D_table), 1, fp)!=1)
            return 2;
    }else{
        if(fwrite(&base->tables[table_index], sizeof(struct D_table), 1, fp)!=1)
            return 2;
    }

    return 0;
}
int m_field(FILE *fp, u_int8_t is_read, struct D_base* base, int field_index){
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp,
          (u_int32_t)(sizeof(base->name)+sizeof(int)*2+MAX_TABLE_NU*sizeof(struct D_table)+field_index*sizeof(struct D_field)),
                  SEEK_SET);
    if(is_read){
        if(fread(&base->fields[field_index], sizeof(struct D_field), 1, fp)!=1)
            return 2;
    }else{
        if(fwrite(&base->fields[field_index], sizeof(struct D_field), 1, fp)!=1)
            return 2;
    }

    return 0;
}

int m_page(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, struct D_page *page_header){
    int table_id=0;
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp, (u_int32_t)(sizeof(struct D_base)+PAGE_SIZE*page_index),
          SEEK_SET);

    if(base->page_ids[page_index]==0)
        return 3;
    if((table_id=base->tables[base->page_ids[page_index]].table_id) == 0)
        return 3;

    if(is_read){
        if(fread(page_header, sizeof(struct D_page)+ PAGE_SIZE / base->tables[table_id - 1].row_size * sizeof(u_int8_t), 1, fp) != 1)
            return 2;
    }else{
        if(fwrite(page_header, sizeof(struct D_page)+ PAGE_SIZE / base->tables[table_id - 1].row_size * sizeof(u_int8_t), 1, fp) != 1)
            return 2;
    }

    return 0;
}
int m_page_row_mask(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, struct D_page *page_header){
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    fseek(fp, (u_int32_t)(sizeof(struct D_base)+PAGE_SIZE*page_index+sizeof(u_int8_t)*row_index),
          SEEK_SET);

    if(is_read){
        if(fread(page_header->m_row_mask+row_index, sizeof(u_int8_t), 1, fp) != 1)
            return 2;
    }else{
        if(fwrite(page_header->m_row_mask+row_index, sizeof(u_int8_t), 1, fp) != 1)
            return 2;
    }

    return 0;
}

int m_row(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, char* data){
    int table_id=0, row_size, row_begin_offset;
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    if(base->page_ids[page_index]==0)
        return 3;
    if((table_id=base->tables[base->page_ids[page_index]].table_id) == 0)
        return 3;
    row_size = base->tables[table_id-1].row_size;
    row_begin_offset = (int32_t)(sizeof(struct D_base)+PAGE_SIZE*page_index+PAGE_SIZE/row_size*sizeof(u_int8_t)+sizeof(struct D_page))+row_size*row_index;

    fseek(fp, row_begin_offset, SEEK_SET);

    if(is_read){
        if(fread(data, row_size, 1, fp) != 1)
            return 2;
    }else{
        if(fwrite(data, row_size, 1, fp) != 1)
            return 2;
    }

    return 0;
}
int m_row_field(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_index, int *field_offset_size, int field_len, char* data){
    int table_id=0, row_size, row_begin_offset;
    int i;
    char* tmp_data = data;
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    if(base->page_ids[page_index]==0)
        return 3;
    if((table_id=base->tables[base->page_ids[page_index]].table_id) == 0)
        return 3;
    row_size = base->tables[table_id-1].row_size;
    row_begin_offset = (int32_t)(sizeof(struct D_base)+PAGE_SIZE*page_index+PAGE_SIZE/row_size*sizeof(u_int8_t)+sizeof(struct D_page))+row_size*row_index;

    fseek(fp, row_begin_offset, SEEK_SET);

    if(is_read){
        for(i=0;i<field_len;i++){
            fseek(fp, field_offset_size[i*2], SEEK_CUR);
            if(fread(tmp_data, field_offset_size[i*2+1], 1, fp) != 1)
                return 2;
            tmp_data += field_offset_size[i*2+1];
        }
    }else{
        for(i=0;i<field_len;i++){
            fseek(fp, field_offset_size[i*2], SEEK_CUR);
            if(fwrite(tmp_data, field_offset_size[i*2+1], 1, fp) != 1)
                return 2;
            tmp_data += field_offset_size[i*2+1];
        }
    }

    return 0;
}
int m_rows(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, char* data){
    int table_id=0, row_size, row_begin_offset;
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    if(base->page_ids[page_index]==0)
        return 3;
    if((table_id=base->tables[base->page_ids[page_index]].table_id) == 0)
        return 3;
    row_size = base->tables[table_id-1].row_size;
    row_begin_offset = (int32_t)(sizeof(struct D_base)+PAGE_SIZE*page_index+PAGE_SIZE/row_size*sizeof(u_int8_t)+sizeof(struct D_page))+row_size*row_offset;

    fseek(fp, row_begin_offset, SEEK_SET);

    if(is_read){
        if(fread(data, row_size*row_length, 1, fp) != 1)
            return 2;
    }else{
        if(fwrite(data, row_size*row_length, 1, fp) != 1)
            return 2;
    }

    return 0;
}
int m_rows_fields(FILE *fp, u_int8_t is_read, struct D_base* base, int page_index, int row_offset, int row_length, int *field_offset_size, int field_len, char* data){
    int table_id=0, row_size, row_begin_offset;
    int i, j;
    char* tmp_data = data;
    if(fp==0||base==0||base->name[0]==0)
        return 1;

    if(base->page_ids[page_index]==0)
        return 3;
    if((table_id=base->tables[base->page_ids[page_index]].table_id) == 0)
        return 3;
    row_size = base->tables[table_id-1].row_size;
    row_begin_offset = (int32_t)(sizeof(struct D_base)+PAGE_SIZE*page_index+PAGE_SIZE/row_size*sizeof(u_int8_t)+sizeof(struct D_page))+row_size*row_offset;

    fseek(fp, row_begin_offset, SEEK_SET);

    if(is_read){
        for(j=0;j<row_length;j++){
            fseek(fp, row_begin_offset+j*row_size,
                  SEEK_SET);
            for(i=0;i<field_len;i++){
                fseek(fp, field_offset_size[i*2], SEEK_CUR);
                if(fread(tmp_data, field_offset_size[i*2+1], 1, fp) != 1)
                    return 2;
                tmp_data += field_offset_size[i*2+1];
            }
        }
    }else{
        for(j=0;j<row_length;j++){
            fseek(fp, row_begin_offset+j*row_size,
                  SEEK_SET);
            for(i=0;i<field_len;i++){
                fseek(fp, field_offset_size[i*2], SEEK_CUR);
                if(fwrite(tmp_data, field_offset_size[i*2+1], 1, fp) != 1)
                    return 2;
                tmp_data += field_offset_size[i*2+1];
            }
        }
    }

    return 0;
}


inline int get_field_size(struct D_field* field){
    switch (field->type) {
        case CHAR:
        case BOOL: return 1;

        case INT: return 4;

        case LONG:
        case FLOAT:
        case DOUBLE:
        case BLOB: return 8;

        case STRING: return field->length;
        default:
            return 0;
    }
}

int count_row_size(struct D_field* fields, int field_len, int table_id){
    // 计算一条记录需要的内存
    int i, k = 0;
    for(i=0;i<field_len;i++){
        if(fields[i].table_id==table_id||table_id==0)
            switch (fields[i].type) {
                case BOOL: k += 1; break;
                case INT: k += 4; break;
                case LONG: k += 8; break;
                case FLOAT: k += 4; break;
                case DOUBLE: k += 8; break;
                case CHAR: k += 1; break;
                case STRING: k += fields[i].length; break;
                case BLOB: k += 8; break;
                default:
                    break;
            }
    }
    return k;
}

int get_rest_table_index(struct D_base* base, int start_index){
    int i;
    for(i=start_index;;i++)
        if(base->tables[i].table_id==0)
            return i;
}
int get_rest_field_index(struct D_base* base, int start_index){
    int i;
    for(i=start_index;;i++)
        if(base->fields[i].table_id==0)
            return i;
}

int create_database(char *name){
    u_int64_t rlt=0;
    FILE *fp;
    if(name == 0)
        return 3;

    if((fp=fopen(name, "wb"))==0)
        return 1;
    
    struct D_base db;
    memset(&db, 0, sizeof(db));
    strncpy(db.name, name, sizeof(db.name));
    rlt = m_base(fp, 0, &db);
    fclose(fp);

    if(rlt!=0){
#ifdef OPEN_LOG
        syslog(LOG_ERR, "create_db %ld, %ld\n", sizeof(struct D_base), rlt);
#endif
        return 2;
    }

    return 0;
}

int delete_database(char *name){
    if(remove(name)==0)
        return 0;
    return 1;
}

int load_database(char* name, struct D_base* base){
    int rlt=0;
    FILE *fp;

    if(name==0||base==0)
        return 3;

    if((fp=fopen(name, "rb"))==0)
        return 1;
    rlt = m_base(fp, 1, base);

    fclose(fp);

    if(rlt!=0){
        return 4;
    }

    return 0;
}


// table
// 内部没有回滚
int m_create_table(FILE *fp, struct D_base* base, char* table_name, struct D_field* fields, int field_len){
    int row_size, table_index, field_index, i, j;
    int64_t rlt;
    struct D_table * table;
    if(fp==0||base==0||table_name==0)
        return 1;

    if((rlt= (int64_t) (fp = fopen(base->name, "r+b")))==0){
        return 2;
    }

    // table_name 查重
    if(base->table_nu!=0){
        for(i=0, j=0;j<base->table_nu;i++){
            if(base->tables[i].table_id==0)
                continue;
            else{
                if(strcmp(base->tables[i].name, table_name)==0){
                    i = -i;
                    break;
                }
                j++;
            }
        }

        if(i<=0)
            return 3;
    }

    row_size = count_row_size(fields, field_len, 0);
    table_index = get_rest_table_index(base, 0);
    table = &base->tables[table_index];

    memset(table, 0, sizeof(struct D_table));
    strncpy(table->name, table_name, sizeof(char)* strlen(table_name));
    table->name[sizeof(char)* strlen(table_name)] = '\0';
    table->row_size = row_size;
    table->table_id = table_index+1;
    rlt = m_table(fp, 0, base, table->table_id-1);

    if(rlt!=0){
        memset(table, 0, sizeof(struct D_table));
        return 3;
    }

    // 预处理field
    for(i=0;i<field_len;i++){
        if(fields[i].name[0]==0){
            return 7;
        }
        for(j=i+1;j<field_len;j++)
            if(strcmp(fields[i].name, fields[j].name)==0)
                return 3;

        fields[i].table_id = table->table_id;
        fields[i].filed_size = get_field_size(&fields[i]);
    }

    field_index = 0;
    for(i=0;i<field_len;i++){
        field_index = get_rest_field_index(base, field_index);
        fields[i].field_id = field_index + 1;
        memcpy(base->fields+field_index, fields+i, sizeof(struct D_field));
        rlt = m_field(fp, 0, base, field_index);
        if(rlt!=0){
            return 8;
        }
    }

    //更新 base_info
    base->table_nu ++;
    base->field_nu += field_len;
    if((rlt= m_base_info(fp, 0, base))!=0){
        base->table_nu -- ;
        base->field_nu -= field_len;
        return 3;
    }

    return 0;
}

int m_delete_table(FILE *fp, struct D_base* base, char* table_name){
    int row_size, table_index, field_len, i, j;
    int64_t rlt;
    struct D_table * table;
    if(fp==0||base==0||table_name==0)
        return 1;

    if((rlt= (int64_t) (fp = fopen(base->name, "r+b")))==0){
        return 2;
    }

    // table_name 是否存在
    for(i=0;j<base->table_nu;i++){
        if(base->tables[i].table_id==0)
            continue;
        else{
            if(strcmp(base->tables[i].name, table_name)==0){
                i = -i;
                break;
            }
            j++;
        }
    }
    if(i>0)
        return 2;

    table_index = -i;

    memset(base->tables+table_index, 0, sizeof(struct D_table));
    rlt = m_table(fp, 0, base, table_index);

    if(rlt!=0){
        return 3;
    }

    field_len = 0;
    for(i=0;j<base->field_nu;i++){
        if(base->fields[i].field_id==0)
            continue;
        else{
            if(base->fields[i].table_id==table_index+1){
                m_field(fp, 0, base, i);
                field_len ++;
            }
            j++;
        }
    }

    //更新 base_info
    base->table_nu --;
    base->field_nu -= field_len;
    if((rlt= m_base_info(fp, 0, base))!=0){
        return 3;
    }

    return 0;
}

int m_rename_table(FILE *fp, struct D_base* base, char* old_name, char* new_name){
    int i,j;
    int64_t rlt;

    if(fp==0||base==0||old_name==0||new_name==0)
        return 1;
    for(i=0;j<base->table_nu;i++){
        if(base->tables[0].table_id==0)
            continue;
        else{
            if(strcmp(base->tables[i].name, old_name)==0){
                i = -i;
                break;
            }
            j++;
        }
    }
    if(i>0)
        return 2;

    memcpy(base->tables[i].name, new_name, sizeof(char)* strlen(new_name));
    if((rlt=m_table(fp, 0, base, i))!=0)
        return 3;
    return 0;
}


// about field

int m_add_field(FILE *fp, struct D_base* base, struct D_field* field){
    int field_index, table_id;
    int64_t rlt;

    if(fp==0||base==0||field==0)
        return 1;

    table_id = field->table_id;
    if(base->tables[table_id-1].table_id!=table_id)
        return 1;

    field_index = get_rest_field_index(base, 0);
    memcpy(base->fields+field_index, field, sizeof(struct D_field));
    rlt = m_field(fp, 0, base, field_index);
    if(rlt!=0)
        return 4;


    base->field_nu --;
    return m_base_info(fp, 0, base);
}

int m_delete_field(FILE *fp, struct D_base* base, int field_id){
    int64_t rlt;
    if(fp==0||base==0||base->fields[field_id-1].field_id!=field_id)
        return 1;

    memset(base->fields+field_id-1, 0, sizeof(struct D_field));
    rlt = m_field(fp, 0, base, field_id-1);
    if(rlt!=0)
        return 4;
    base->field_nu --;
    return m_base_info(fp, 0, base);
}

int m_rename_field(FILE *fp, struct D_base* base, int field_id, char* new_name){
    if(fp==0||base==0||new_name==0||base->fields[field_id-1].field_id!=field_id|| strcmp(new_name, base->fields[field_id-1].name)==0)
        return 1;
    memcpy(base->fields[field_id-1].name, new_name, sizeof (char)* strlen(new_name));
    return m_field(fp, 0, base, field_id-1);
}

// row
int m_check_rows_unique_field(FILE *fp, struct D_base* base, int table_id, int *unique_field_offset_size, int field_len, int row_length, char* data){
    int row_size, row_begin_offset, page_size, tmp_field_size;
    int i, j, k, h, h1;
    char* tmp_data = data;
    char* tmp_data2, *tmp_data3;
    struct D_page *page;
    if(fp==0||base==0||data==0){
        return 1;
    }

    row_size = base->tables[table_id-1].row_size;
    page_size = (int32_t)(sizeof(struct D_page) + PAGE_SIZE/row_size * sizeof(u_int8_t));
    tmp_field_size = 0;
    for(i=0;i<field_len;i++)
        tmp_field_size += unique_field_offset_size[i*2+1];

    for(i=0;base->page_ids[i]!=0;i++){
        if(base->page_ids[i]!=table_id)
            continue;
        page = calloc(1, page_size);
        for(j=0;page->m_row_mask[j]!=0;j++){
            if(page->m_row_mask[j]==0)
                continue;
            tmp_data2 = calloc(tmp_field_size, sizeof(char));
            m_row_field(fp, 1, base, i, j, unique_field_offset_size, field_len, tmp_data2);

            tmp_data = data;
            for(k=0;k<row_length;k++, tmp_data=tmp_field_size*i+data){
                tmp_data3 = tmp_data2;
                for(h=0;h<field_len;h++){
                    tmp_data+=unique_field_offset_size[h*2];
                    for(h1=0;h1<unique_field_offset_size[h*2+1];h1++){
                        if(*(tmp_data+h1)==*(tmp_data3+h1))
                            continue;
                        h1=-h1;
                        break;
                    }
                    if(h1>0){
                        free(tmp_data2);
                        free(page);
                        return 1;
                    }
                    tmp_data3 += (int)(unique_field_offset_size[h*2+1]);
                }
            }
            free(tmp_data2);
        }
        free(page);
    }

    return 0;
}



int make_row(void* values, struct D_field* fields){

}

//int insert_row(struct D_context * ctx, int table_id, int field_len, char** field_names, void* value);

int m_insert_rows(struct D_context * ctx, int table_id, int field_len, char** field_names, int value_len, void** values){
    int i, j, k, i1, i2, rest_value_len=value_len, rlt;
    int offset_page_begin;
    struct D_page *tmp_page;
    struct D_field tmp_field;
    unsigned long row_size, page_size, total_field_len;
    FILE *fp;
    int *filed_mask_arr = 0; // 17: 1未提供 1-16 字段所在下标

    if(field_names==0||values==0||ctx==0)
        return 1;
    // 检查字段是否重复
    for(i=0;i<field_len;i++)
        if(field_names[i]==0)
            return 2;
        else
            for(j=0;j<field_len;j++)
                if(strcmp(field_names[i], field_names[j])==0)
                    return 3;

    // 检查字段是否存在
    for(i=0;i<field_len;i++){
        for(j=0;j<ctx->database.field_nu;j++)
            if(ctx->fields[j].table_id==table_id&&strcmp(ctx->fields[j].name, field_names[j])==0){
                j = -j;
                break;
            }
        if(j>0)
            return 4;
    }

    // 计算该表的字段数量
    for(j=0;j<ctx->database.field_nu;j++) if(ctx->fields[j].table_id==table_id) k++;
    total_field_len = k;
    filed_mask_arr = calloc(total_field_len, sizeof(int));

    //   检查必须提供的字段，跟新field_mask
    for(j=0, k=0;j<ctx->database.field_nu;j++){
        if(ctx->fields[j].table_id==table_id&&
           (
                   (!(ctx->fields[j].field_mask&0b101)&&(ctx->fields[j].field_mask&0b10))||
                   (!(ctx->fields[j].field_mask&0b100))&&(ctx->fields[j].field_mask&0b11)
           )
                ){
            for(i=0;i<field_len;i++)
                if(strcmp(field_names[i], ctx->fields[j].name)==0){
                    i=-i;
                    break;
                }
            if(i>0)
                return 5;

            filed_mask_arr[k] = j;
                    k++;
        }else if(ctx->fields[j].table_id){
            filed_mask_arr[k] = j + (1<<16);
            k++;
        }
    }


    offset_page_begin = sizeof(struct D_base)+sizeof(struct D_table)*MAX_TABLE_NU + sizeof(struct D_field) * MAX_FILED_NU;
    row_size = count_row_size(ctx->fields, ctx->database.field_nu, table_id);
    page_size = sizeof(struct D_page)+PAGE_SIZE/row_size*sizeof(unsigned int);

    if((rlt=(fp=fopen(ctx->database.name, "r+b")))==0){
        return 0x11;
    }

    for(i=0; i < MAX_PAGE_NU;i++){
        // 创建新page
        if(ctx->database.page_ids[i]==0){
            fseek(fp, offset_page_begin+PAGE_SIZE*i, SEEK_SET);

            tmp_page = calloc(1, page_size);
            tmp_page->table_id = table_id;
            memset(tmp_page->m_row_mask, 0, PAGE_SIZE / row_size * sizeof(unsigned int));
            if((rlt=fwrite(tmp_page, sizeof(page_size), 1, fp))!=1){
                fclose(fp);
                free(tmp_page);
                return 0x13;
            }
            ctx->database.page_ids[i] = table_id;

        }
        else if(ctx->database.page_ids[i]!=table_id)
            continue;
        else{
            // 读取page
            tmp_page = calloc(1, page_size);
            fseek(fp, offset_page_begin+PAGE_SIZE*i, SEEK_SET);
            if((rlt= fread(tmp_page, page_size, 1, fp))!=1){
                free(tmp_page);
                fclose(fp);
                return 0x12;
            }
        }

        for(j=0, i2=0;j<PAGE_SIZE/row_size; j++){
            if(tmp_page->m_row_mask[j] != 0)
                continue;
            // unique 属性 查重, waiting for update

            // one by one， 不知道什么是性能
            fseek(fp, offset_page_begin+PAGE_SIZE*i+row_size*j, SEEK_SET);
            for(k=0, i1=0;k<total_field_len;k++){
                tmp_field = ctx->fields[filed_mask_arr[k]&(131071)];
                if(filed_mask_arr[k]>>16){
                    if(tmp_field.field_mask&0b10000){
                        tmp_field.current_auto_id ++;
                        if((rlt= fwrite(&(tmp_field.current_auto_id), sizeof(int), 1, fp))!=1){
                            free(tmp_page);
                            free(filed_mask_arr);
                            fclose(fp);
                            return 0x14;
                        }
                    }
                }else{
                    rlt = 0;
                    switch (tmp_field.type) {
                        case BOOL: rlt = fwrite(((char*)values[i1])[i2], sizeof(char)*1, 1, fp); break;
                        case INT: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        case LONG: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        case FLOAT: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        case DOUBLE: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        case CHAR: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        case STRING: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        case BLOB: rlt = fwrite(((char*)values[i1])[i2], sizeof(char), 1, fp); break;
                        default:
                            break;
                    }
                    i1++;
                }
            }

            i2++;
        }
        free(tmp_page);
    }

    return 0;
}

int delete_rows(struct D_context * ctx, int * table_ids, struct R_query * query){

    return 0;
}

int m_update_rows(struct D_context * ctx, int field_opera_len, struct R_field_opera* field_opera, struct R_query * query){

    return 0;
}

int m_do_select(struct D_context * ctx, struct R_query* query, struct R_view* view){

    return 0;
}


// ------------------------------------

