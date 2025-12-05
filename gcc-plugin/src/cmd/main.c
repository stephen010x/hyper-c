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
//#include <sys/wait.h>
#include <limits.h>

#include "wrapper/gcc-plugin.h"
#include "wrapper/plugin-version.h"
#include "hyperc.h"




#define __export__ __attribute__((visibility("default")))
#define DEREF_TYPE(__type) typeof(*(__type)NULL)




__inline__ size_t linux_gsap_len(void);
void linux_gsap(int bmax, char buff[]);





DEREF_TYPE(plugin_init_func) plugin_init;
int plugin_is_GPL_compatible;
int gcc_build_version = GCCPLUGIN_VERSION;







int main(int argc, char *argv[]) {
//     (void)argc, (void)argv;
// 
//     // forks the process here
//     pid_t pid = fork();
// 
//     if (pid < 0) {
//         perror("failed to fork process");
//         return -1;
//     }
// 
//     if(pid) {
//         // parent fork
//         int status;
//         wait(&status);  // wait for child to terminate
//         (void)status;
// 
//         return 0;
//         
//     } else {
//         // child fork
// 
//         // copy argv arguments to a new buffer
//         char *cargv[argc+3];
//         cargv[0] = "gcc";
//         memcpy(cargv, argv+1, argc-1);
//         
//         // add my own arguments
//         cargv[argc+0] = "-fplugin=/path/to/name.ext";
//         cargv[argc+1] = "-fplugin-arg-name-key1[=value1]";
//         cargv[argc+2] = NULL;
// 
//         // execute gcc as our main process
//         //execvp(cargv[0], cargv);
// 
//         //execvp("echo", (char *[]){"echo", "hello from child", NULL});
//         
//         execvp("echo", cargv); 
//         
//         // the previous should exit the process normally. If not, then we 
//         // exit with an error here
//         _exit(127); // 127 - command not found
//     }

    //char ppath[linux_gsap_len()];
    //linux_gsap(sizeof(ppath), ppath);

    system("gcc -fplugin=./bin/hyper ./test/main.c");

    return 0;
}




__export__ int plugin_init(struct plugin_name_args *plugin_info, 
                           struct plugin_gcc_version *version   ) {

    plugin_info->version = "0.1";
    plugin_info->help = "placeholder text";

    printf("gcc basever:       %s\n", version->basever);
    printf("gcc datestamp:     %s\n", version->datestamp);
    printf("gcc devphase:      %s\n", version->devphase);
    printf("gcc revision:      %s\n", version->revision);
    printf("gcc config arg:    %s\n", version->configuration_arguments);

    printf("plugin base name:  %s\n", plugin_info->base_name);
    printf("plugin full name:  %s\n", plugin_info->full_name);
    printf("plugin version:    %s\n", plugin_info->version);

    //plugin_default_version_check(...)

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
