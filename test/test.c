/*
 * flagtest.c — a grab-bag test program for probing GCC optimization/
 * size/hardening flags.
 *
 * Build examples:
 *   gcc -Os -S -o - flagtest.c | less          # inspect generated asm
 *   gcc -Os -c flagtest.c -o flagtest.o && size flagtest.o
 *   gcc -O2 -funroll-loops --param max-unroll-times=4 -S flagtest.c
 *
 * Each function below is written to isolate ONE behavior so that diffing
 * `-S` output (or grepping for call/jmp/movl patterns) between two flag
 * sets gives you a clean signal. Comments mark what each section probes.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

/* ------------------------------------------------------------------ */
/* 1. Globals — for testing -fvisibility, BSS placement, --gc-sections */
/* ------------------------------------------------------------------ */

int global_uninit;                 /* .bss candidate */
int global_zero_explicit = 0;      /* also .bss in practice */
int global_nonzero = 42;           /* .data — actually costs bytes on disk */
static int static_uninit;          /* .bss, internal linkage */
const char *global_string = "a string literal in .rodata";

/* Never referenced anywhere — good for testing --gc-sections / -flto
 * dead-code elimination. Should vanish entirely from a culled binary. */
static int dead_global = 1234;
__attribute__((used)) static int unused_helper(int x) { return x * 3 + dead_global; }
/* `used` keeps -Wunused-function quiet while still letting --gc-sections
 * (a LINKER decision) prune it away if nothing calls it — exactly what
 * we want to test: does the linker cull it even though the compiler was
 * told not to complain about it being unreferenced. */

/* ------------------------------------------------------------------ */
/* 2. Fixed, small trip-count loops — complete peeling candidates     */
/* ------------------------------------------------------------------ */

/* 4 iterations: should fully flatten under -fpeel-loops with
 * max-completely-peel-times >= 4 */
void fill4(volatile int *arr) {
    for (int i = 0; i < 4; i++) {
        arr[i] = i;
    }
}

/* 16 iterations: flattens at default max-completely-peel-times=16,
 * but should stay a real loop if capped lower (e.g. =4) */
void fill16(volatile int *arr) {
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 2;
    }
}

/* 32 iterations: exceeds default peel/unroll thresholds, good for
 * testing the *partial* unroll path (max-unroll-times) */
void fill32(volatile int *arr) {
    for (int i = 0; i < 32; i++) {
        arr[i] = i;
    }
}

/* ------------------------------------------------------------------ */
/* 3. Unknown trip-count loop — general unroller / no-complete-peel   */
/* ------------------------------------------------------------------ */

void fill_n(volatile int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
}

/* A loop with a function call body — defeats vectorization, useful for
 * testing pure unroll/peel behavior without SIMD muddying the picture */
// extern void external_sink(int x);   /* deliberately undefined here */

__attribute__((noinline)) void external_sink(int x) { (void)x; }


void call_loop_fixed(void) {
    for (int i = 0; i < 16; i++) {
        external_sink(i);
    }
}

void call_loop_variable(int n) {
    for (int i = 0; i < n; i++) {
        external_sink(i);
    }
}

/* ------------------------------------------------------------------ */
/* 4. Nested loops — -floop-interchange candidate                     */
/* ------------------------------------------------------------------ */

#define DIM 64
void matrix_touch(volatile int mat[DIM][DIM]) {
    /* column-major access pattern on a row-major array: cache-hostile
     * unless the compiler interchanges the loop order */
    for (int col = 0; col < DIM; col++) {
        for (int row = 0; row < DIM; row++) {
            mat[row][col] = row + col;
        }
    }
}

/* ------------------------------------------------------------------ */
/* 5. Redundant computation — GCSE / tree-PRE candidates               */
/* ------------------------------------------------------------------ */

int partial_redundancy(int x, int y, int flag) {
    int result;
    if (flag) {
        result = x * y + 1;       /* x*y computed here ... */
    } else {
        result = 0;
    }
    return result + x * y;        /* ... and redundantly again here */
}

int full_redundancy(int a, int b) {
    int p = a * b;
    int q = a * b;   /* trivially redundant; -fgcse should fold this */
    return p + q;
}

/* ------------------------------------------------------------------ */
/* 6. memcmp / string-builtin inlining threshold tests                */
/* ------------------------------------------------------------------ */

int cmp2(const void *a, const void *b)  { return memcmp(a, b, 2)  == 0; }
int cmp8(const void *a, const void *b)  { return memcmp(a, b, 8)  == 0; }
int cmp16(const void *a, const void *b) { return memcmp(a, b, 16) == 0; }
int cmp32(const void *a, const void *b) { return memcmp(a, b, 32) == 0; }
int cmp64(const void *a, const void *b) { return memcmp(a, b, 64) == 0; }

/* ------------------------------------------------------------------ */
/* 7. Vectorizable loop — -fvect-cost-model / -ftree-vectorize probe   */
/* ------------------------------------------------------------------ */

void vec_add(float * restrict dst, const float * restrict a,
             const float * restrict b, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i];
    }
}

/* Predictive commoning candidate: reuses arr[i-1] across iterations */
void diff_prev(volatile int *out, const int *in, int n) {
    for (int i = 1; i < n; i++) {
        out[i] = in[i] - in[i - 1];
    }
}

/* ------------------------------------------------------------------ */
/* 8. Stack buffer — -fstack-protector / -D_FORTIFY_SOURCE probe      */
/* ------------------------------------------------------------------ */

void stack_buffer_user(const char *input) {
    char buf[64];
    /* snprintf with a runtime-unknown length is exactly what FORTIFY's
     * _chk variants and the stack protector care about */
    snprintf(buf, sizeof(buf), "value: %s", input);
    printf("%s\n", buf);
}

void large_alloca_user(int n) {
    /* triggers -fstack-clash-protection probing if n is large/unknown */
    char *buf = (char *)alloca((size_t)n);
    if (n > 0) {
        buf[0] = 'x';
        buf[n - 1] = 'y';
    }
}

/* ------------------------------------------------------------------ */
/* 9. Function pointer / indirect call — -fcf-protection (CET) probe  */
/* ------------------------------------------------------------------ */

typedef int (*op_fn)(int, int);

static int op_add(int a, int b) { return a + b; }
static int op_sub(int a, int b) { return a - b; }

int dispatch(op_fn fn, int a, int b) {
    return fn(a, b);   /* indirect call site: ENDBR64 / CET-relevant */
}

int pick_and_run(int which, int a, int b) {
    op_fn table[2] = { op_add, op_sub };
    return dispatch(table[which & 1], a, b);
}

/* ------------------------------------------------------------------ */
/* 10. Cloning / constant-propagation candidate — -fipa-cp-clone probe */
/* ------------------------------------------------------------------ */

static int scale(int value, int factor) {
    /* called with both constant and non-constant `factor` below;
     * a prime candidate for -fipa-cp-clone to create a specialized
     * clone for the constant call sites */
    return value * factor + (factor > 0 ? 1 : -1);
}

int scale_by_two(int value)        { return scale(value, 2); }
int scale_by_ten(int value)        { return scale(value, 10); }
int scale_by_runtime(int value, int f) { return scale(value, f); }

/* ------------------------------------------------------------------ */
/* 11. Loop-invariant branch — -funswitch-loops probe                 */
/* ------------------------------------------------------------------ */

void invariant_branch_loop(volatile int *arr, int n, int mode) {
    for (int i = 0; i < n; i++) {
        if (mode) {           /* mode is loop-invariant: unswitching  */
            arr[i] = i * 2;
        } else {
            arr[i] = i + 1;
        }
    }
}

/* ------------------------------------------------------------------ */
/* 12. PIC / GOT-indirection probe — global accessed from a function  */
/* ------------------------------------------------------------------ */

int touch_global(void) {
    global_nonzero += 1;
    return global_nonzero;
}

/* ------------------------------------------------------------------ */
/* main — exercises everything so nothing gets DCE'd away entirely    */
/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    volatile int buf32[32];
    volatile int buf16[16];
    volatile int buf4[4];
    volatile int mat[DIM][DIM];

    fill4((int *)buf4);
    fill16((int *)buf16);
    fill32((int *)buf32);
    fill_n((int *)buf32, argc + 32);

    matrix_touch(mat);

    int r1 = partial_redundancy(argc, 7, argc % 2);
    int r2 = full_redundancy(argc, 11);

    char a[64] = "hello world this is a test buffer of bytes!!!!";
    char b[64] = "hello world this is a test buffer of bytes!!!!";
    int c2  = cmp2(a, b);
    int c8  = cmp8(a, b);
    int c16 = cmp16(a, b);
    int c32 = cmp32(a, b);
    int c64 = cmp64(a, b);

    float fa[64], fb[64], fd[64];
    for (int i = 0; i < 64; i++) { fa[i] = (float)i; fb[i] = (float)(i * 2); }
    vec_add(fd, fa, fb, 64);

    int diffbuf[32];
    diff_prev((int *)diffbuf, (int *)buf32, 32);

    if (argc > 1) {
        stack_buffer_user(argv[1]);
        large_alloca_user((int)strlen(argv[1]) + 16);
    }

    int dispatched = pick_and_run(argc, 3, 4);

    int s2  = scale_by_two(argc);
    int s10 = scale_by_ten(argc);
    int srt = scale_by_runtime(argc, argc + 1);

    invariant_branch_loop((int *)buf32, 32, argc % 2);

    int g = touch_global();

    long total = r1 + r2 + c2 + c8 + c16 + c32 + c64
               + (long)fd[0] + diffbuf[1] + dispatched
               + s2 + s10 + srt + g + global_uninit + static_uninit;

    printf("total=%ld string=%s\n", total, global_string);
    return (int)(total & 0xff);
}
