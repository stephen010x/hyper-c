// Reference
// https://en.cppreference.com/w/c/language.html
// https://gcc.gnu.org/onlinedocs/gccint/
// https://gcc.gnu.org/onlinedocs/gccint/Plugins.html
// https://gcc.gnu.org/onlinedocs/gccint/Plugin-API.html
// https://gcc.gnu.org/wiki/plugins
// https://gcc.gnu.org/wiki/GCC_PluginAPI
// https://github.com/gcc-mirror/gcc/blob/master/gcc/plugin.h

// sudo apt install gcc-15-plugin-dev

// /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/gcc-plugin.h
// /usr/lib/gcc/x86_64-linux-gnu/15/plugin/include/plugin.h


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "wrapper/gcc-plugin.h"
#include "wrapper/plugin-version.h"
#include "hyperc.h"





#define __export__ __attribute__((visibility("default")))
#define DEREF_TYPE(__type) typeof(*(__type)NULL)




__inline__ size_t linux_gsap_len(void);
void linux_gsap(int bmax, char buff[]);





DEREF_TYPE(plugin_init_func) plugin_init;
int plugin_is_GPL_compatible __attribute__((used, visibility("default")));
int gcc_build_version = GCCPLUGIN_VERSION;









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
