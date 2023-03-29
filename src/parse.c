//
// Created by 19545 on 28/03/2023.
//

#include <regex.h>
#include <string.h>
#include "parse.h"
#include "table.h"
#include <ctype.h>


int count_appeared_nu(const char* input_str, const char* pattern, int* match_offset_size, char is_char){
    int i=0;
    regex_t reg;
    const char* p1=0;
    const size_t match_sz = 1;
    regmatch_t p_match[1];
    int c = regcomp(&reg, pattern, REG_EXTENDED);
    if (0 != c)
    {
        return -1;
    }
    /** 起始匹配的偏移量 */
    int offset = 0;
    int match_count = 0;
    do {
        p1 = input_str + offset;
        c = regexec(&reg, p1, match_sz, p_match, 0);
        if (REG_NOMATCH == c)
            break;
        else if (0 == c)
        {
            if(match_offset_size!=0){
                if(is_char){
                    match_offset_size[i] = p_match[0].rm_so + offset;
                }else{
                    match_offset_size[i*2] = p_match[0].rm_so + offset;
                    match_offset_size[i*2+1] = p_match[0].rm_eo - p_match[0].rm_so;
                }
                i++;
            }
            ++match_count;
            offset += p_match[0].rm_eo;
            continue;
        }

    } while (1);
    regfree(&reg);
    return match_count;
}

/**
 *
 * @param s1
 * @param s1_len
 * @param s2
 * @param s2_len
 * @param flag 1 << 1: 是否忽略大小写
 * @return
 */
u_int8_t is_string_equal(const char* s1, const char* s2, int str_len, int flag){
    int i=0;
    for(;i<str_len;i++){
        if(flag&1){
            if(tolower(s1[i])!= tolower(s2[i]))
                return 0;
        }else{
            if(s1[i]!= s2[i])
                return 0;
        }
    }
    return 1;
}


int pare_sql_from_string(struct SQL_STRUCTURE * sql_s){
    int i, j, k1;

    int split_count = 0;
    int *split_index=0;
    int world_count = 0;
    int* world_offset_size=0;

    split_count = count_appeared_nu(sql_s->full_sql, "[^\\]'", 0, 1);
    if(split_count%2){
        sql_s->can_be_used = 0;
        return 1;
    }
    split_index = calloc(split_count, sizeof(int));
    count_appeared_nu(sql_s->full_sql, "[^\\]'", split_index, 1);

    world_count = count_appeared_nu(sql_s->full_sql, "\\w+", 0, 0);
    if(world_count<=1){
        free(split_index);
        return 2;
    }
    world_offset_size = calloc(world_count*2, sizeof(int));
    count_appeared_nu(sql_s->full_sql, "\\w+", world_offset_size, 0);

    // 排除字符串中的字符串
    sql_s->world_length = 0;
    for(i=0;i<world_count;i++){
        for(j=0;j<split_count/2;j++){
            if(split_index[j*2]<world_offset_size[i*2]&&split_index[j*2+1]>world_offset_size[i*2]+world_offset_size[i*2+1]){
                world_offset_size[i*2] = -1;
                world_offset_size[i*2+1] = 0;
                break;
            }
        }
        if(world_offset_size[i*2]!=-1)
            sql_s->world_length ++;
    }

    // 将 字符串 和 world 的位置写入sql_structure
    sql_s->world_length += split_count / 2;
    sql_s->world_offset_size = calloc(sql_s->world_length*2, sizeof(int));
    sql_s->str_value_count = split_count/2;
    sql_s->str_value_mark_arr = calloc(sql_s->world_length, sizeof(int));
    memset(sql_s->str_value_mark_arr, 0, sizeof(unsigned char)*sql_s->world_length);

    i=0; j=0; k1=0;
    while(i!=world_count&&j!=(split_count/2)){
        if(i==world_count||world_offset_size[i*2]>split_index[j*2]){
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
            //if(j==(split_count/2)||world_offset_size[i*2]<split_index[j*2])
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
    // free mem
    free(world_offset_size);
    world_offset_size = 0;
    free(split_index);
    split_index = 0;

    sql_s->can_be_used = 1;
    switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[0], sql_s->world_offset_size[1])) {
        case S_KW_CREATE:
            switch (get_str_hash(sql_s->full_sql + sql_s->world_offset_size[0], sql_s->world_offset_size[1])) {
                case S_KW_DATABASE:

                    break;
                case S_KW_TABLE:

                    break;
                default:
                    break;
            }
            break;
        case S_KW_DROP:
            break;
        case S_KW_USE:
            break;
        case S_KW_ALTER:
            break;
        case S_KW_SELECT:
            break;
        case S_KW_INSERT:
            break;
        case S_KW_UPDATE:
            break;
        case S_KW_DELETE:
            break;
        default:
            sql_s->can_be_used = 0;
    }
    return -1;
}

void free_sql_structure(struct SQL_STRUCTURE * sql_s){

}


/**
 * python code
import re

def _hash(v: str):
    rlt = 0
    for i1, i in enumerate(v.upper()):
        rlt += (i1+1) * ord(i)
    return rlt


l1 = re.findall(r'\w+', s0)
l2 = [_hash(i[5:]) for i in l1]
s0 = ''
for i, j in zip(l1, l2):
    s0 += f"{i}={hex(j)},\n"
print(s0)
print(len(set(l2))==len(l2) and 42 not in l2)
 *
 *
 * @param s1
 * @param str_len
 * @return
 */
int get_str_hash(const char* s1, int str_len){
    int i=0;
    int rlt = 0;
    for(;i<str_len;i++){
        rlt += (i+1) * toupper(s1[i]);
        printf("%d\n", toupper(s1[i]));
    }
    return rlt;
}

int learn_regex(){

    printf("%d\n", get_str_hash("*", 1));
/** 待匹配字符串 */
    char inputstr[128] = "'hello,'111\\'welcome '222'to my party";
//    printf("%d", inputstr[0]);
//    scanf("%s", inputstr);
    /** regex 错误输出缓冲区 */
    char regerrbuf[256];
    regex_t reg;
    /** 正则表达式 */
    const char* pattern = "\\w+"; //  [^\]'
    int match_offset_size[200];
    int match_length = 0;
    int i=0;
    match_length = count_appeared_nu(inputstr, pattern, match_offset_size, 0);

    printf("%d\n", match_length);
    for(;i<match_length;i++)
        printf("%d:%d\n", match_offset_size[i*2], match_offset_size[i*2+1]);

    return 0;
    printf("==GNU Regex Test==\n");
    printf("Pattern     :%s\n", pattern);
    printf("Input String:%s\n", inputstr);
    /************************************************************************/
    /* 编译正则表达式,编译成功的 regex_t 对象才可以被后续的 regexec 使用    */
    /************************************************************************/
    int c = regcomp(&reg, pattern, REG_EXTENDED);
    if (0 != c)
    {
        /************************************************************************/
        /*  正则表达式编译出错输出错误信息                                      */
        /*  调用 regerror 将错误信息输出到 regerrbuf 中                         */
        /*  regerrbuf 末尾置0,确保上面调用regerror 导致 regerrbuf 溢出的情况下, */
        /*  字符串仍有有结尾0                                                   */
        /*  然后 printf 输出                                                    */
        /************************************************************************/
        regerror(c, &reg, regerrbuf, sizeof(regerrbuf));
        regerrbuf[sizeof(regerrbuf) - 1] = '\0';
        printf("%s\n", regerrbuf);
        return -1;
    }
    /************************************************************************/
    /* 记录匹配位置的 regmatch_t 数组                                       */
    /* 上面的正则表达有 2 个捕获组,加上默认组(group 0),                     */
    /* 为了记录所有捕获组位置,所以这里需要长度为 3 的 regmatch_t 数组       */
    /************************************************************************/
    const size_t matchsz = 3;
    regmatch_t pmatch[3];
    /** 起始匹配的偏移量 */
    size_t offset = 0;
    /** 捕获计数         */
    int matchcount = 0;
    /************************************************************************/
    /* regexec 不能通过一次调用找到字符串中所有满足匹配条件的字符串位置,    */
    /* 所以需要通过步进偏移的方式循环查找字符串中所有匹配的字符串,          */
    /* 每一次匹配的起始偏移是上一次匹配到的字符串结束偏移                   */
    /************************************************************************/
    do {
        printf("Search start %d\n",(int)offset);
        /** 正则表达式匹配的起始地址 */
        const char* p = inputstr + offset;
        /************************************************************************/
        /* regmatch_t 用于记录正则表达匹配的结果,每一个 regmatch_t 记录一个捕获 */
        /* 组(catch group)的在字符串中的起始位置。                              */
        /* 如果调用 regexec 时如果不提供 regmatch_t(nmatch为0,pmatch为NULL),    */
        /* 或者提供的 regmatch_t 数组长小于正则表达式中全部捕获组的数量,        */
        /* regexec 也能正常匹配,只是无法记录匹配的位置                          */
        /* 或不能完全记录所有的匹配结果                                         */
        /************************************************************************/
        c = regexec(&reg, p, matchsz, pmatch, 0);
        if (REG_NOMATCH == c)
        {
            /** 没有找到匹配结束循环 */
            printf("MATCH FINISHED\n");
            break;
        }
        else if (0 == c)
        {
            /** 找到匹配,则输出匹配到的所有捕获组(catch group) */
            printf("%d MATCH (%d-%d)\n", ++matchcount, pmatch[0].rm_so, pmatch[0].rm_eo);
            for (int i = 0; i < matchsz; ++i)
            {
                printf("group %d :<<", i);
//                print_str(p, pmatch[i].rm_so, pmatch[i].rm_eo);
                printf(">>\n");
            }
            /************************************************************************/
            /* 使用整体匹配捕获组0(group 0)的结束位置的更新偏移量,                  */
            /* 下一次匹配从当前匹配的结束位置开始                                   */
            /************************************************************************/
            offset += pmatch[0].rm_eo;
            continue;
        }
        else
        {
            /************************************************************************/
            /** regexec 调用出错输出错误信息,结束循环                               */
            /************************************************************************/
            regerror(c, &reg, regerrbuf, sizeof(regerrbuf));
            regerrbuf[sizeof(regerrbuf) - 1] = '\0';
            printf("%s\n", regerrbuf);
            break;
        }
    } while (1);
    printf("%d MATCH FOUND\n", matchcount);
    regfree(&reg);
    return 0;

}
