//#include <stdio.h>
//#include <string.h>



 



int main() {

    unsigned short _Vector3 v1 = {1, 2, 3};
    unsigned short _Vector3 v2 = {4, 5, 6};
    unsigned short _Vector3 v3;

    v3 = v1 + v2;
    
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
    global++;

    return v1;
}
