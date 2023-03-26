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
    rlt = fwrite(&db, sizeof(db), 1, fp);
    fclose(fp);

    if(rlt!=1){
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

int load_database(char* name, struct D_context * ctx){
    int rlt=0, i, j;
    FILE *fp;

    if(name==0||ctx==0)
        return 3;

    if((fp=fopen(name, "rb"))==0)
        return 1;

    if((rlt=fread(&ctx->database, sizeof(struct D_base), 1, fp))!=1){
#ifdef OPEN_LOG
        syslog(LOG_ERR, "load_database %ld, %d\n", sizeof(struct D_base), rlt);
#endif
        fclose(fp);
        return 2;
    }

    ctx->fields = calloc(ctx->database.field_nu, sizeof(struct D_field));
    ctx->tables = calloc(ctx->database.table_nu, sizeof(struct D_table));


    if(ctx->database.table_nu!=0&&((rlt=fread(ctx->tables, sizeof(struct D_table), ctx->database.table_nu, fp))!=ctx->database.table_nu)){
#ifdef OPEN_LOG
        syslog(LOG_ERR, "2222");
#endif
        fclose(fp);
        free(ctx->fields);
        free(ctx->tables);
        ctx->fields = 0;
        ctx->tables = 0;
        return 2;
    }

    fseek(fp, sizeof(struct D_base)+MAX_TABLE_NU*sizeof(struct D_table), SEEK_SET);
    if(ctx->database.field_nu!=0&&((rlt=fread(ctx->fields, sizeof(struct D_field), ctx->database.field_nu, fp))!=ctx->database.field_nu)){
#ifdef OPEN_LOG
        syslog(LOG_ERR, "11111");
#endif
        fclose(fp);
        free(ctx->fields);
        free(ctx->tables);
        ctx->fields = 0;
        ctx->tables = 0;
        return 1;
    }

    fclose(fp);

    return 0;
}


// table

int create_table(struct D_context * ctx, char* table_name, struct D_field* fields, int field_len){
    int rlt=0;
    int i, j, k;
    int table_id_arr[ctx->database.table_nu];
    FILE *fp;
    struct D_table *table = calloc(1, sizeof(struct D_table));
    struct D_table *n_tables;
    struct D_field *n_fields;

    if(ctx->database.table_nu==MAX_TABLE_NU){
        return 0x12;
    }
    if(ctx->database.field_nu+field_len>MAX_FILED_NU){
        return 0x13;
    }

    // 查重
    for(i=0;i<ctx->database.table_nu;i++)
        if(strcmp(ctx->tables[i].name, table_name)==0)
            return 5;
    for(i=0;i<field_len;i++)
        if(fields[i].name[0] == 0)
            return 7;

    // has same name
    for(i=0;i<field_len;i++){
        for(j=i+1;j<field_len;j++)
            if(strcmp(fields[i].name, fields[j].name) == 0)
                return 6;
    }

    if(ctx->database.name[0] == 0)
        return 3;

    
    // alloc table_id
    memset(table, 0, sizeof(struct D_table));
    strncpy(table->name, table_name, sizeof(char)* strlen(table_name));
    for(i=0;i<ctx->database.table_nu;i++){
        table_id_arr[i] = ctx->tables->table_id;
    }

    table->table_id = get_random(0b1111, 0b1111+MAX_TABLE_NU, table_id_arr, ctx->database.table_nu);
//    table->row_size = count_row_size(fields, field_len, 0);
    for(i=0;i<field_len;i++)
        fields[i].table_id = table->table_id;

    if((fp=fopen(ctx->database.name, "r+b"))==0)
        return 1;

    // write table
    if((rlt=fwrite(table, sizeof(struct D_table), 1, fp))!=1){
        return 2;
    }

    // write field
    fseek(fp, (long)(sizeof(ctx->database)+sizeof(struct D_table)*MAX_TABLE_NU+sizeof(struct D_field)+ctx->database.field_nu), SEEK_SET);
    if((rlt=fwrite(fields, sizeof(struct D_table), field_len, fp)) != field_len){
        return 2;
    }
    
    fclose(fp);

    // update context
    n_tables = calloc(ctx->database.table_nu+1, sizeof(struct D_table));
    n_fields = calloc(ctx->database.field_nu + field_len, sizeof(struct D_field));
    memcpy(n_tables, ctx->tables, sizeof(struct D_table)*ctx->database.table_nu);
    n_tables[ctx->database.table_nu] = *table;
    ctx->database.table_nu ++;
    free(ctx->tables);
    ctx->tables = n_tables;

    memcpy(n_fields, ctx->tables, sizeof(struct D_table) * ctx->database.table_nu);
    for(i=0;i<field_len;i++)
        n_fields[i + ctx->database.field_nu] = fields[i];
    ctx->database.field_nu += field_len;
    free(ctx->fields);
    ctx->fields = n_fields;

    return 0;
}


int delete_table(struct D_context * ctx, char* table_name){
    int rlt=0;
    int i, j, k, field_nu;
    int table_id_arr[ctx->database.table_nu];
    FILE *fp;
    struct D_table *n_tables;
    struct D_field *n_fields;

    if(table_name==0)
        return 0x11;
    if(!ctx->database.table_nu){
        return 1;
    }

    for(i=0;i<ctx->database.table_nu;i++)
        if(strcmp(ctx->tables[i].name, table_name)==0){
            i=-i;
            break;
        }

    if(i>0){

        return 2;
    }

    for(j=0, k=0; j < ctx->database.field_nu; j++)
        if(ctx->fields[j].table_id==ctx->tables[-i].table_id)
            k ++;
    field_nu = k;

    n_tables = calloc(ctx->database.table_nu-1, sizeof(struct D_table));
    n_fields = calloc(ctx->database.field_nu - k, sizeof(struct D_field));

    // alloc table_id
    memcpy(n_tables, ctx->tables, sizeof(struct D_table)*(-i));
    memcpy(n_tables+(-i), ctx->tables+(-i+1), sizeof(struct D_table)*(ctx->database.table_nu-(-i+1)));

    for(k=0, j=0; k < ctx->database.field_nu; k++){
        if(ctx->fields[j].table_id==ctx->tables[-i].table_id)
            continue;
        n_fields[k] = ctx->fields[j++];
    }

    if((fp=fopen(ctx->database.name, "r+b"))==0){
        free(n_tables);
        free(n_fields);
        return 3;
    }

    // write table
    fseek(fp, sizeof(struct D_context)+sizeof(struct D_table)*(-i), SEEK_SET);
    if((rlt=fwrite(n_tables+(-i), sizeof(struct D_table), (ctx->database.table_nu-(-i+1)), fp))!=(ctx->database.table_nu-(-i+1))){
        free(n_tables);
        free(n_fields);

        return 4;
    }

    // write field
    fseek(fp, sizeof(ctx->database)+sizeof(struct D_table)*MAX_TABLE_NU, SEEK_SET);
    if((rlt=fwrite(n_fields, sizeof(struct D_field), ctx->database.field_nu-field_nu, fp)) != ctx->database.field_nu-field_nu){
        free(n_tables);
        free(n_fields);
        return 5;
    }

    fclose(fp);

    // update context
    ctx->database.table_nu --;
    free(ctx->tables);
    ctx->tables = n_tables;

    ctx->database.field_nu -= field_nu;
    free(ctx->fields);
    ctx->fields = n_fields;

    if(ctx->database.table_nu==0)
        ctx->tables = 0;
    if(ctx->database.field_nu==0)
        ctx->fields = 0;

    return 0;
}

int rename_table(struct D_context * ctx, char* old_name, char* new_name){
    int i,j, rlt;
    FILE *fp;

    if(old_name==0||new_name==0)
        return 0x1;

    if(!ctx->database.table_nu){
        return 1;
    }

    for(i=0;i<ctx->database.table_nu;i++)
        if(strcmp(ctx->tables[i].name, old_name)==0){
            i = -i;
            break;
        }

    if(i>0){
        return 2;
    }

    memcpy(ctx->tables[-i].name, new_name, strlen(new_name)*sizeof(char));

    if((rlt=(fp= fopen(ctx->database.name, "r+b")))==0){

        return 3;
    }

    fseek(fp, sizeof(struct D_context)+(-1)*sizeof(struct D_table), SEEK_SET);
    if((rlt=fwrite(&ctx->tables[-i], sizeof(struct D_table), 1, fp))!=1){

        fclose(fp);
        memcpy(ctx->tables[-i].name, old_name, strlen(old_name)*sizeof(char));
        return 4;
    }
    fclose(fp);

    return 0;
}


// about field

int add_field(struct D_context * ctx, int table_id, struct D_field* field){
    int i=0, rlt;
    FILE *fp;

    if(ctx->database.field_nu==MAX_FILED_NU){
        return 0x12;
    }

    if(field->name[0]=='\0')
        return 0x11;

    for(i=0;i<ctx->database.field_nu;i++)
        if(ctx->fields[i].table_id==table_id&& strcmp(ctx->fields[i].name, field->name) == 0){
            return 1;
        }

    field->table_id = table_id;
    struct D_field *n_fields = calloc(ctx->database.field_nu+1, sizeof(struct D_field));
    memcpy(n_fields, ctx->fields, sizeof(struct D_field)*ctx->database.field_nu);
    n_fields[ctx->database.field_nu] = *field;

    if((rlt=(fp=fopen(ctx->database.name, "r+b")))==0){
        free(n_fields);
        return 2;
    }
    fseek(fp, (long)(sizeof(struct D_context)+MAX_TABLE_NU*sizeof(struct D_table)+sizeof(struct D_field)*ctx->database.field_nu), SEEK_SET);
    if((rlt=fwrite(field, sizeof(struct D_field), 1, fp))!=1){
        free(n_fields);
        fclose(fp);
        return 3;
    }

    fclose(fp);

    ctx->database.field_nu ++;
    free(ctx->fields);
    ctx->fields = n_fields;
    return 0;
}

int delete_field(struct D_context * ctx, int table_id, char * field_name){
    int i=0, j, rlt;
    FILE *fp;

    if(field_name==0)
        return 0x1;
    if(ctx->database.field_nu==0)
        return 1;

    for(i=0;i<ctx->database.field_nu;i++)
        if(ctx->fields[i].table_id==table_id&& strcmp(ctx->fields[i].name, field_name) == 0){
            i=-i;
            break;
        }

    if(i>0){
        return 2;
    }

    struct D_field *n_fields = calloc(ctx->database.field_nu-1, sizeof(struct D_field));
    memcpy(n_fields, ctx->fields, sizeof(struct D_field)*(-i));

    for(j=-i+1;j<ctx->database.field_nu;j++)
        memcpy(&n_fields[j-1], &ctx->fields[j], sizeof(struct D_field));

    if((rlt=(fp=fopen(ctx->database.name, "r+b")))==0){

        free(n_fields);
        return 2;
    }
    fseek(fp, sizeof(struct D_context)+MAX_TABLE_NU*sizeof(struct D_table)+sizeof(struct D_field)*(-i), SEEK_SET);
    if((rlt=fwrite(ctx->fields+(-i+1), sizeof(struct D_field), ctx->database.field_nu-1-(-i), fp))!=ctx->database.field_nu-1-(-i)){
        fclose(fp);
        free(n_fields);
        return 3;
    }

    fclose(fp);

    ctx->database.field_nu --;
    free(ctx->fields);
    ctx->fields = n_fields;

    if(ctx->database.field_nu==0)
        ctx->fields = 0;
    return 0;
}

int rename_field(struct D_context * ctx, int table_id, char* old_name, char* new_name){
    int i,j, rlt;
    FILE *fp;

    if(old_name==0||new_name==0)
        return 0x1;
    if(!ctx->database.field_nu){
        return 1;
    }


    for(i=0;i<ctx->database.field_nu;i++)
        if(ctx->fields[i].table_id==table_id&&strcmp(ctx->fields[i].name, new_name)==0){
            return 0x1;
        }

    for(i=0;i<ctx->database.field_nu;i++)
        if(ctx->fields[i].table_id==table_id&&strcmp(ctx->fields[i].name, old_name)==0){
            i = -i;
            break;
        }

    if(i>0){
        return 2;
    }

    memcpy(ctx->fields[-i].name, new_name, strlen(new_name)*sizeof(char));

    if((rlt=(fp= fopen(ctx->database.name, "r+b")))==0){
        memcpy(ctx->fields[-i].name, old_name, strlen(old_name)*sizeof(char));
        return 3;
    }

    fseek(fp, (long)(sizeof(struct D_context)+MAX_TABLE_NU*sizeof(struct D_table)+(-i)*sizeof(struct D_field)), SEEK_SET);
    if((rlt=fwrite(&ctx->fields[-i], sizeof(struct D_field), 1, fp))!=1){

        fclose(fp);
        memcpy(ctx->fields[-i].name, old_name, strlen(old_name)*sizeof(char));
        return 4;
    }
    fclose(fp);

    return 0;
}

// row

int make_row(void* values, struct D_field* fields){

}

//int insert_row(struct D_context * ctx, int table_id, int field_len, char** field_names, void* value);

int insert_rows(struct D_context * ctx, int table_id, int field_len, char** field_names, int value_len, void** values){
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

int update_rows(struct D_context * ctx, int field_opera_len, struct R_field_opera* field_opera, struct R_query * query){

    return 0;
}

int do_select(struct D_context * ctx, struct R_query* query, struct R_view* view){

    return 0;
}


// ------------------------------------

