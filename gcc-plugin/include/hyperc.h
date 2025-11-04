

#define __inline__ __attribute__((always_inline))
#define lenof(__n) (sizeof(__n)/(sizeof((__n)[0])))
#define segfault() (((void (*)(void))NULL)())




