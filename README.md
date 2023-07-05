## 起因
最近在学习linux应用开发方面的东西， 突然冒个念头做个微型数据库，刚好可以利用这个项目来学一些东西和测试一下自己的开发能力，然后这个项目就诞生了，
考虑到一个完整的数据库需要耗费大量的精力，因此只能做个阉割版的

## 期望的功能
### 数据库管理系统方面
1. 利用单文件来存储数据库
2. 能够 创建/删除/重名名 表和字段
3. 支持增删改查 (部分实现)
4. 支持复杂的操作支持where，嵌套sql语句，联表查询 (未实现)
5. 解析sql语句 (部分实现)
6. 支持事务回滚  (未实现)

### 网络方面
1. 高并发

## 项目结构
- 表结构的设计，主要文件`table.h`
- 表的操作，主要文件`transaction.c`
- 高并发，`reactor.c`
- 测试文件`test.c`

## 使用示例参见`test.c` 测试文件

## 开发过程中的建议
- 尽量将变量放到开头位置，这样有助于防止内存泄露


## 未开发的功能
由于精力不太允许，同时又有点懒，一些复杂的功能需要算法基础、其他知识(如：编译原理)以及深厚的c项目功底，就不勉强自己地放弃了许多功能
- 放弃 跟记录状态有关的字段，不支持null，但可以在建表时设计一个字段来标记该记录的null状态
- 没有索引，我用的是定长+变长的方式(对于blob类型的数据，存放到另一个文件中，表中只储存blob在该文件中的地址)储存数据
- 没有表锁、page锁，只有数据库锁，又，库锁太简单了，就没有加上去
- 放弃where，只有完整版的insert 阉割版select，delete，没有update
- 不检查sql语法
- 没有制定统一错误码，开发阶段还统一不出来
- 没有设计缓存，但设计起来比较简单
- 没有事务回滚
- 网络方面没有优化数据读取发送的过程



## 遇到的error
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

```
struct msg{
  int len;
  char  data[0];
}

strlen(msg.data) //不知道为啥会引发错误，测试高并发服务时 gdb模式直接推出
```

## 未修复的问题
- varchar length maybe zero
