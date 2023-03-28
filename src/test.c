#include "table.h"
#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

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

int test_memory_opera(){

    return 0;
}


int test_table(){

    int rlt = 0;
    char* name = "hello";
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
    strcpy(fields[2].name, "name3");

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



int main(int argc, char const *argv[])
{

//    printf("%d,", sizeof(&d1[1]));
//    printf("%d", sizeof(d1));
//    return test_table();
    return test_byte_convert();
}

