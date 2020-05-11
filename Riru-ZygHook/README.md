# Riru - Zyghook

![License GPL-3.0](https://img.shields.io/badge/license-GPLv3.0-green.svg)


Zyghook的目标:
1. 尝试动态拉起/data/lcoal/tmp/sohook/packagename.so
2. 方便快速进行HOOk调试


日志：

2020-5-11 11:27:28
1. 三星S10能读写SD卡
2. 1加7T不能读写SD卡

2020-5-11 11:30:16
尝试进行dlopenhook：

方案1：
```c++
__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv
//DLSym并不适用,原因是并没有被导出
system/bin/linker64 //apex/com.android.runtime/bin/linker64 1加7T
const char* linker_name = sizeof(void*) == 4 ? "/system/bin/linker": "system/bin/linker64";
void* hLinker = WDynamicLibOpen(linker_name);
if (!hLinker){
    LOGE("Failed to WDynamicLibOpen(%s), hook_linker aborted", linker_name);
    return;
}

void* fun = WDynamicLibSymbol(hLinker, "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");        
if(fun != NULL){
    WInlineHookFunction(fun, (void*)dlopen_for_all, (void**)&old_dlopen_for_all);
    LOGI("inline_hook %s: %p %p %p", "dlopen_for_all", fun, (void*)dlopen_for_all, old_dlopen_for_all);
} else {
    LOGE("__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv not found");
}
```



方案2：
```c++
NEW_FUNC_DEF(void*, __loader_dlopen, const char* name, int flags, /*const void* extinfo,*/ const void* caller_addr)；
XHOOK_REGISTER("libdl.so", __loader_dlopen);
```




Thanks：

https://github.com/kotori2/riru_unity_example.git