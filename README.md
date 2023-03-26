## code struction


## error
```c
struct D_context{
    struct D_base database;
    struct D_table* tables;
    struct D_field* fields;
};
struct D_context ctx; // error info:  Segmentation fault (core dumped)
struct D_context * ctx; // right
```

```c
    char * names[ctx->database.table_nu];
    // 等价于两个*
```

```c
    char * names[len];
    i=0;
    while(i++<field_len){
    // error code , var field_len will be modify after =
    // for(i=0;i<field_len;i++){
        // printf("%d,", field_len);
        names[i] = fileds[i].name;
    }
```

## wait for fix
- varchar length maybe zero