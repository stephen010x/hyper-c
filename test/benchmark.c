// =========================
// Paste your memcmp2 here:
// bool memcmp2(const char *restrict str1,
//              const char *restrict str2,
//              int len);
// =========================

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#define ITERATIONS 10000000


#define __inline__ static inline __attribute__((always_inline))

__inline__ bool _bcmp(const char *restrict str1, const char *restrict str2, int len) {
    switch (len) {
        case 1:  return str1[0] == str2[0];
        case 2:  return *(int16_t*)&str1[0] == *(int16_t*)&str2[0];
        case 4:  return *(int32_t*)&str1[0] == *(int32_t*)&str2[0];
        case 8:  return *(int64_t*)&str1[0] == *(int64_t*)&str2[0];
        default: exit(1);
    }
}

__inline__ bool memcmp2(const char *restrict str1, const char *restrict str2, int len) {
    switch (len) {
        case 0:  return true;
        case 1:  return _bcmp(str1, str2, 1);
        case 2:  return _bcmp(str1, str2, 2);
        case 3:  return _bcmp(str1, str2, 2) && _bcmp(str1+2, str2+2, 1);
        case 4:  return _bcmp(str1, str2, 4);
        case 5:  return _bcmp(str1, str2, 4) && _bcmp(str1+4, str2+4, 1);
        case 6:  return _bcmp(str1, str2, 4) && _bcmp(str1+4, str2+4, 2);
        case 7:  return _bcmp(str1, str2, 4) &&  memcmp2(str1+4, str2+4, 3);
        case 8:  return _bcmp(str1, str2, 8);
        case 9:  return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 1);
        case 10: return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 2);
        case 11: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 3);
        case 12: return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 4);
        case 13: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 5);
        case 14: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 6);
        case 15: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 7);
        case 16: return _bcmp(str1, str2, 8) && _bcmp(str1+8, str2+8, 8);
        case 17: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 9);
        case 18: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 10);
        case 19: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 11);
        case 20: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 12);
        case 21: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 13);
        case 22: return _bcmp(str1, str2, 8) &&  memcmp2(str1+8, str2+8, 14);
        default: return !memcmp(str1, str2, len);
    }
}

__inline__ double now_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

char a[1<<20];
char b[1<<20];




__inline__ int test(char *a, char *b, int i) {
    double start, t1, t2;

    // memcpy(b, a, 32);

    start = now_sec();
    int local = 0;

    for (int x = 0; x < ITERATIONS; x++) {
        // b[x % i] ^= (char)(x * 31);
        local += !!memcmp(a+(unsigned short)x, b+(unsigned short)x, i) == 0;
        // __asm__ volatile("" : : "r"(local) : "memory");
    }

    t1 = now_sec() - start;
    // memcpy(b, a, 32);

    start = now_sec();
    for (int x = 0; x < ITERATIONS; x++) {
        // b[x % i] ^= (char)(x * 31);
        local += !!memcmp2(a+(unsigned short)x, b+(unsigned short)x, i) == 0;
        // __asm__ volatile("" : : "r"(local) : "memory");
    }

    t2 = now_sec() - start;

    printf("Size %d bytes\n", i);
    printf("  memcmp:  %.6f sec\n", t1);
    printf("  memcmp2: %.6f sec\n", t2);
    printf("  ratio:   %.3fx\n\n", t2 / t1);

    return local;
}




int main() {
    int local = 0;

    for (int i = 0; i < sizeof(a); i++) {
        a[i] = (char)(i * 2654435761ULL) >> 16;
        b[i] = (char)(i * 2654435761ULL) >> 16 + (i%50 ? 0 : 10);
    }

    local += test(a, b, 1);
    local += test(a, b, 2);
    local += test(a, b, 3);
    local += test(a, b, 4);
    local += test(a, b, 5);
    local += test(a, b, 6);
    local += test(a, b, 7);
    local += test(a, b, 8);
    local += test(a, b, 9);
    local += test(a, b, 10);
    local += test(a, b, 11);
    local += test(a, b, 12);
    local += test(a, b, 13);
    local += test(a, b, 14);
    local += test(a, b, 15);
    local += test(a, b, 16);
    local += test(a, b, 17);
    local += test(a, b, 18);
    local += test(a, b, 19);
    local += test(a, b, 20);
    local += test(a, b, 21);
    local += test(a, b, 22);
    local += test(a, b, 23);

    return local;
}
