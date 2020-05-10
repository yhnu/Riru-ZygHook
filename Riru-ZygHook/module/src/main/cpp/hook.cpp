//
// Created by acris on 2020/4/12.
//
#include <cstdio>
#include <cstring>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <dlfcn.h>
#include <cstdlib>
#include <string>
#include <sys/system_properties.h>

#include "external/libxhook/xhook.h"
#include "include/riru.h"
#include "include/logging.h"

#define XHOOK_REGISTER(LIB_PATH_REGX, NAME) \
    if (xhook_register(LIB_PATH_REGX, #NAME, (void*) new_##NAME, (void **) &old_##NAME) == 0) { \
        LOGI("succeed to register hook " #NAME "."); \
        if (riru_get_version() >= 8) { \
            void *f = riru_get_func(#NAME); \
            if (f != nullptr) \
                memcpy(&old_##NAME, &f, sizeof(void *)); \
            riru_set_func(#NAME, (void *) new_##NAME); \
        } \
    } else { \
        LOGE("failed to register hook " #NAME "."); \
    }

#define NEW_FUNC_DEF(ret, func, ...) \
    static ret (*old_##func)(__VA_ARGS__); \
    static ret new_##func(__VA_ARGS__)

NEW_FUNC_DEF(void*, __dl__Z9do_dlopenPKciPK17android_dlextinfoPKv, const char *name, int flags, const void *extinfo, void *caller_addr){
    LOGI("__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
    return old___dl__Z9do_dlopenPKciPK17android_dlextinfoPKv(name, flags, extinfo, caller_addr);
}

typedef void (*T_dlopen_posthook)(const char* name);
static T_dlopen_posthook dlopen_posthook_handler = NULL;

NEW_FUNC_DEF(void*, __loader_dlopen, const char* name, int flags, /*const void* extinfo,*/ const void* caller_addr){    
    void* handle = old___loader_dlopen(name, flags, /*extinfo,*/ caller_addr);
    if(dlopen_posthook_handler) {
        LOGI("will post dlopen: %s", name);
        dlopen_posthook_handler(name);
    }
    return handle;
}

void install_hook(const char *package_name, int user, const std::string& so_name) {
    LOGI("install hook for %d:%s", user, package_name);
    // xhook_enable_debug(true);
    // XHOOK_REGISTER(".*", __system_property_get);
    XHOOK_REGISTER("libdl.so", __loader_dlopen);

    void *dl = dlopen(so_name.c_str(), RTLD_LAZY);
    if( !dl) {
        LOGE("dlopen %s failed!!!", so_name.c_str());
    }
    dlopen_posthook_handler = (T_dlopen_posthook)dlsym(dl, "dlopen_posthook");    

    // XHOOK_REGISTER(".*\\.so$", my_system_log_print);
    //char sdk[PROP_VALUE_MAX + 1];
    //if (__system_property_get("ro.build.version.sdk", sdk) > 0 && atoi(sdk) >= 28) {
        //XHOOK_REGISTER(_ZN7android4base11GetPropertyERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_);
    //}
    //XHOOK_REGISTER("linker", __dl__Z9do_dlopenPKciPK17android_dlextinfoPKv);

    if (xhook_refresh(0) == 0)
        xhook_clear();
    else
        LOGE("failed to refresh hook");
}