#ifdef _WIN32
#include "boinc_win.h"
#endif

#include "shmem.h"
#include "filesys.h"
#include "app_ipc.h"
#include "boinc_api.h"
#include "graphics2.h"

#ifdef USE_FILE_MAPPED_SHMEM
#include <sys/stat.h>
#endif

#if (defined(_WIN32) || defined(USE_FILE_MAPPED_SHMEM))
static void get_shmem_name(char* prog_name, char* shmem_name) {
    APP_INIT_DATA aid;
    boinc_get_init_data(aid);
    sprintf(shmem_name, "boinc_%s_%d", prog_name, aid.slot);
}
#else
static key_t get_shmem_name(char* prog_name) {
    char cwd[256], path[256];
    boinc_getcwd(cwd);
    sprintf(path, "%s/init_data.xml", cwd);
    return ftok(path, 2);
}
#endif

void* boinc_graphics_make_shmem(char* prog_name, int size) {
#ifdef _WIN32
    HANDLE shmem_handle;
    char shmem_name[256];
    void* p;
    get_shmem_name(prog_name, shmem_name);
    shmem_handle = create_shmem(shmem_name, size, &p, false);
    if (shmem_handle == NULL) return 0;
    return p;
#else
    void* p;
#ifdef USE_FILE_MAPPED_SHMEM
    char shmem_name[256];
    get_shmem_name(prog_name, shmem_name);
    int retval = create_shmem(shmem_name, size, &p);
    // Graphics app may be run by a different user & group than worker app
    // Although create_shmem passed 0666 to open(), it was modified by umask
    if (retval == 0) chmod(shmem_name, 0666);
#else
    key_t key = get_shmem_name(prog_name);
    int retval = create_shmem(key, size, 0, &p);
#endif
    if (retval) return 0;
    return p;
#endif
}

#ifdef _WIN32
void* boinc_graphics_get_shmem(char* prog_name) {
    HANDLE shmem_handle;
    char shmem_name[256];
    void* p;
    get_shmem_name(prog_name, shmem_name);
    shmem_handle = attach_shmem(shmem_name, &p);
    if (shmem_handle == NULL) return 0;
    return p;
}
#else
void* boinc_graphics_get_shmem(char* prog_name) {
    void* p;
    int retval;
#ifdef USE_FILE_MAPPED_SHMEM
    char shmem_name[256];
    get_shmem_name(prog_name, shmem_name);
    retval = attach_shmem(shmem_name, &p);
#else
    key_t key = get_shmem_name(prog_name);
    retval = attach_shmem(key, &p);
#endif
    if (retval) return 0;
    return p;
}
#endif
