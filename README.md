## code struction


## error
```
struct D_context{
    struct D_base database;
    struct D_table* tables;
    struct D_field* fields;
};
struct D_context ctx; // error info:  Segmentation fault (core dumped)
struct D_context * ctx; // right
```

```
    char * names[ctx->database.table_nu];
    // 等价于两个*
```

```
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

## io structure

- work dispatcher: 底层任务分配器 + event 
  - 任务队列、等待读写队列、进行中队列
  - 过程：无阻塞->出队->if(读){有写->放入等待队列 && return; 读;读完->回调} else (写){有读->}
  - 过程：无阻塞 -> 出队 -> 获取page锁 -> if(读){有写->放入等待队列 && return; 读;读完->回调} else (写){有读->}
- work spool: 最底层的io任务

## sql parse
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
alter table table_name add/drop XXXX field_name;

select * from table where field1 = '' and field2 = '' and field3 + field4 > 6 limit orderby filed desc ;
select field1, field2 from table where field1 = '' and field2 = '' and field3 + field4 > 6;
// select count(field) as field1 as,  sum(field) as field2 as , avg(field) as 99

insert into table values(v1, v2);
insert into table(field1, field2) valuse(v1, v2);

update table set field1=v1, field2=field1+1, where ...;

delete from table where ...
```
