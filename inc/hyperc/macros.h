#ifndef HYPERC_MACROS_H
#define HYPERC_MACROS_H



#define __inline__ __attribute__((always_inline)) static inline
#define lenof(__n) (sizeof(__n)/(sizeof((__n)[0])))
#define segfault() (((void (*)(void))NULL)())



#endif /* #ifndef HYPERC_MACROS_H */
