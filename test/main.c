//#include <stdio.h>
//#include <string.h>

const char str[] = "Hello world.\\\\\\ I am' dave! " "yehaw!\n" "how are \"you\" today?";

char  test7 = 'a';
bool  test0 = true;
int   test1 = 01177;
int   test1 = 1194;
int   test2 = 119400;
int   test3 = 1094;
int   test8 = 1094ll;
float test4 = 9.871234f;
float test5 = 10e+36;
float test6 = 0x10p+36;
 



int main() {

    /* hello world */

    /*
    unsigned short _Vector3 v1 = {1, 2, 3};
    unsigned short _Vector3 v2 = {4, 5, 6};
    unsigned short _Vector3 v3;

    v3 = v1 + v2;
    */

    return 0;
    
}



hyper int global1;



struct input {
    uint64_t v1 : 8;
    uint64_t v2 : 8;
    uint64_t v3 : 8;
    uint64_t n1 : 8;    // 32
    uint64_t n2 : 8;
    uint64_t n3 : 8;
    uint64_t t1 : 8;
    uint64_t t2 : 8;    // 64
};

struct output {
    float v1;
    float v2;
    float v3;
};

struct uniforms {
    int time;
};




hyper struct output main_test(struct input i, struct uniforms u) {
    global1++;

    struct output o;

    o.v1 = (float)u.time;
    o.v2 = (float)i.v1;

    return o;
}
