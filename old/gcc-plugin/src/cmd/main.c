// Reference
// https://en.cppreference.com/w/c/language.html
// https://gcc.gnu.org/onlinedocs/gccint/
// https://gcc.gnu.org/onlinedocs/gccint/Plugins.html
// https://gcc.gnu.org/onlinedocs/gccint/Plugin-API.html
// https://gcc.gnu.org/wiki/plugins
// https://github.com/gcc-mirror/gcc/blob/master/gcc/plugin.h

// sudo apt install gcc-15-plugin-dev

// /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/gcc-plugin.h
// /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/plugin.h


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "wrapper/gcc-plugin.h"
#include "wrapper/plugin-version.h"




#define DEREF_TYPE(__type) typeof(*(__type)NULL)




__inline__ size_t linux_gsap_len(void);
void linux_gsap(int bmax, char buff[]);





//int gcc_build_version = GCCPLUGIN_VERSION;







int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    system("gcc -fplugin=./bin/hyper.so ./test/main.c");

    return 0;
}






// linux get self absolute path
__inline__ size_t linux_gsap_len(void) {
    char buff[PATH_MAX];
    getcwd(buff, PATH_MAX);
    return strnlen(buff, PATH_MAX);
}

void linux_gsap(int bmax, char buff[]) {
    getcwd(buff, bmax);
} 
