//
//  main.c
//  csim
//  name: WU YUEXIN
//  Student ID: 1900012946
//
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
 #include "cachelab.h"

typedef int Bool;
#define TRUE 1
#define FALSE 0

int s, E, b;
unsigned long S;
FILE * fp;
// If there is '-v' in cmd, it will print the detail
Bool detailed = TRUE; 
int hit_count = 0, miss_count = 0, evict_count = 0;
unsigned long no, tag, offset, size;
Bool flag_miss, flag_hit, flag_evict;
unsigned long visit_times = 0;

struct cacheline{
    Bool valid; // indicate whether the cacheline is used
    unsigned long tag;
    unsigned long seq; // indicate the latest usage
} *MyCache;

// turn string into address in the format of unsigned long
// and get size at the same time
// the function will return the value of address
unsigned long str2addr(char * str){
    unsigned long len = strlen(str);
    int i = 0;
    for(; i < len; i++)
        if(str[i] == ',')
            break;
    size = 0;
    for(int j = i + 1; j < len; j++){
        if(str[j] >= '0' && str[j] <= '9')
            size = size * 10 + (str[j] - '0');
    }
    unsigned long addr = 0;
    for(int j = 3; j < i; j++){
        if('0' <= str[j] && str[j] <= '9') addr = addr * 16 +(str[j] - '0');
        else if('A' <= str[j] && str[j] <= 'Z') addr = addr * 16 + (str[j] - 'A') + 10;
        else if('a' <= str[j] && str[j] <= 'z') addr = addr * 16 + (str[j] - 'a') + 10;
    }
    return addr;
}

// ravel the address into tag and setnumber and offset
// the values are saved in global variables
void addr2ele(unsigned long addr){
    offset = addr - (addr >> b << b);
    addr = addr >> b;
    no = addr - (addr >> s << s);
    addr = addr >> s;
    tag = addr;
}

// update_cache due to tag, set number to get merely one byte
void update_cache(){
    visit_times++;
    unsigned long temp = no * E;
    // hit
    for(unsigned long i = temp; i < temp + E; i++){
        if(MyCache[i].valid == TRUE && MyCache[i].tag == tag){
            flag_hit = TRUE;
            MyCache[i].seq = visit_times;
            return;
        }
    }
    // miss
    for(unsigned long i = temp; i < temp + E; i++){
        if(MyCache[i].valid == FALSE){
            MyCache[i].valid = TRUE;
            MyCache[i].tag = tag;
            MyCache[i].seq = visit_times;
            flag_miss = TRUE;
            miss_count++;
            return;
        }
    }
    // miss and evict
    unsigned long min_seq = MyCache[temp].seq;
    unsigned long pos = temp;
    for(unsigned long i = temp + 1; i < temp + E; i++){
        if(MyCache[i].seq < min_seq){
            min_seq = MyCache[i].seq;
            pos = i;
        }
    }
    MyCache[pos].tag = tag;
    MyCache[pos].seq = visit_times;
    flag_evict = TRUE;
    flag_miss = TRUE;
    evict_count++;
    miss_count++;
}

int main(int argc, char * argv[]){
    char t[30], str[30];
    int ch;
    while((ch = getopt(argc, argv, "vs:E:b:t:")) != -1){
        switch(ch){
            case 'v': detailed = TRUE; break;
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': strcpy(t, optarg); break;
        }
    }
    fp = fopen(t, "r");
    if(!fp) {
        printf("Fail to open\n");
        exit(0);
    }
    // the numebr of sets
    S = 1 << s;
    // allocate memory
    MyCache = (struct cacheline *) malloc(sizeof(struct cacheline) * S * E);
    for(unsigned long i = 0; i < S * E; i++){
        MyCache[i].seq = 0;
        MyCache[i].tag = 0;
        MyCache[i].valid = FALSE;
    }
    // read in one line at one time
    while(fgets(str, 30, fp)){
        if(str[0] == 'I') continue;
        unsigned long addr = str2addr(str);
        flag_hit = flag_miss = flag_evict = FALSE;
        for(int i = 0; i < size; i++){
            addr2ele(addr);
            update_cache();
            addr++;
        }
        addr -= size;
        if(detailed){
            printf("%c %lx,%ld", str[1], addr, size);
            if(flag_miss) printf(" miss");
            if(flag_evict) printf(" eviction");
            if(!(flag_evict || flag_miss) && flag_hit) printf(" hit");
            if(str[1] == 'M') printf(" hit");
            printf(" \n");
        }
        if(!(flag_evict || flag_miss) && flag_hit) hit_count++;
        if(str[1] == 'M') hit_count++;
            
    }
    printSummary(hit_count, miss_count, evict_count);

    free(MyCache);
    // close file
    Bool f_closed = fclose(fp);
    if(!f_closed) {
        printf("Warning: fail to close the file!\n");
        exit(0);
    }
    return 0;
}


