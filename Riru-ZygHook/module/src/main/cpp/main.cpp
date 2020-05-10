#include <jni.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include "include/logging.h"
#include "include/hook.h"
#include <fstream>
#include <iostream>

static std::string soname;
static char package_name[256];
static int uid;
static int enable_hook;
static std::vector<std::string> globalPkgBlackList = {"com.oneplus.camera", "com.oneplus.gallery"};

// inline bool so_exists (const std::string& name) {
//   struct stat buffer;   
//   return (stat (name.c_str(), &buffer) == 0); 
// }

inline bool so_exists (const std::string& name) {
    if (access(name.c_str(), R_OK) == 0) {
        return true;
    } else {
        PLOGE("access %s", name.c_str());        
    }
    return false;
}

bool isAppNeedHook(JNIEnv *env, jstring jAppDataDir, jstring jPackageName) {
    if (jPackageName) {
        const char *packageName = env->GetStringUTFChars(jPackageName, nullptr);
        sprintf(package_name, "%s", packageName);
        env->ReleaseStringUTFChars(jPackageName, packageName);
    } else if (jAppDataDir) {
        const char *appDataDir = env->GetStringUTFChars(jAppDataDir, nullptr);
        int user = 0;
        while (true) {
            // /data/user/<user_id>/<package>
            if (sscanf(appDataDir, "/data/%*[^/]/%d/%s", &user, package_name) == 2)
                break;

            // /mnt/expand/<id>/user/<user_id>/<package>
            if (sscanf(appDataDir, "/mnt/expand/%*[^/]/%*[^/]/%d/%s", &user, package_name) == 2)
                break;

            // /data/data/<package>
            if (sscanf(appDataDir, "/data/%*[^/]/%s", package_name) == 1)
                break;

            package_name[0] = '\0';
            return false;
        }
        env->ReleaseStringUTFChars(jAppDataDir, appDataDir);
    } else {
        return false;
    }

    const char *p1 = strstr(package_name, "com.lingdong.tv");
    if(p1 != NULL)
    {
        soname = "/data/emulated/0/sohook/"; soname += package_name; soname += ".so";
        LOGI("Will Check %s", soname.c_str());
        if(so_exists(soname))
        {
            LOGI("Will Load uid=%d soname=%s", uid, soname.c_str());
            return true;
        }
        else
        {
            //LOGI("Not Load %s", soname.c_str());
            soname = "/data/local/tmp/"; soname += package_name; soname += ".so";
            if(so_exists(soname))
            {
                LOGI("Will Load uid=%d soname=%s", uid, soname.c_str());
                return true;
            }
        }
    }


    // std::string pkgName = package_name;
    // for (auto &s : globalPkgBlackList) {
    //     if (pkgName.find(s) != std::string::npos) {
    //         return false;
    //     }
    // }

    return false;
}

void injectBuild(JNIEnv *env) {
    if (env == nullptr) {
        LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
        return;
    }
    LOGI("inject android.os.Build for %s ", package_name);

    jclass build_class = env->FindClass("android/os/Build");
    if (build_class == nullptr) {
        LOGW("failed to inject android.os.Build for %s due to build is null", package_name);
        return;
    }

    jstring brand = env->NewStringUTF("google");
    jstring manufacturer = env->NewStringUTF("Google");
    jstring product = env->NewStringUTF("coral");
    jstring model = env->NewStringUTF("Pixel 4 XL");

    jfieldID brand_id = env->GetStaticFieldID(build_class, "BRAND", "Ljava/lang/String;");
    if (brand_id != nullptr) {
        env->SetStaticObjectField(build_class, brand_id, brand);
    }

    jfieldID manufacturer_id = env->GetStaticFieldID(build_class, "MANUFACTURER", "Ljava/lang/String;");
    if (manufacturer_id != nullptr) {
        env->SetStaticObjectField(build_class, manufacturer_id, manufacturer);
    }

    jfieldID product_id = env->GetStaticFieldID(build_class, "PRODUCT", "Ljava/lang/String;");
    if (product_id != nullptr) {
        env->SetStaticObjectField(build_class, product_id, product);
    }

    jfieldID device_id = env->GetStaticFieldID(build_class, "DEVICE", "Ljava/lang/String;");
    if (device_id != nullptr) {
        env->SetStaticObjectField(build_class, device_id, product);
    }

    jfieldID model_id = env->GetStaticFieldID(build_class, "MODEL", "Ljava/lang/String;");
    if (model_id != nullptr) {
        env->SetStaticObjectField(build_class, model_id, model);
    }

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(brand);
    env->DeleteLocalRef(manufacturer);
    env->DeleteLocalRef(product);
    env->DeleteLocalRef(model);
}

static void appProcessPre(JNIEnv *env, jint _uid, jstring appDataDir, jstring packageName) {
    uid = _uid;
    enable_hook = isAppNeedHook(env, appDataDir, packageName);    
    //LOGI("%s", package_name);
}


static void appProcessPost(JNIEnv *env) {
    if (enable_hook) {
        //injectBuild(env);
        install_hook(package_name, uid % 100000, soname);
        std::ofstream outfile;
        outfile.open("/sdcard/sohook/zyghookxx.txt");
        outfile << package_name << std::endl;
        outfile.close();

        FILE* fp = fopen("/sdcard/zyghook.txt", "w");
        if(fp)
        {
            LOGE("---------SUCCEED---------%s---------", package_name);
            fputs(package_name, fp);
            fclose(fp);
        }
        else
        {
            PLOGE("---------FAILED---------%s---------", package_name);            
        }
    }
}

extern "C" {
#define EXPORT __attribute__((visibility("default"))) 

EXPORT void nativeForkAndSpecializePre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jintArray *fdsToClose, jintArray *fdsToIgnore, jboolean *is_child_zygote,
        jstring *instructionSet, jstring *appDataDir, jstring *packageName,
        jobjectArray *packagesForUID, jstring *sandboxId) {
    // packageName, packagesForUID, sandboxId are added from Android Q beta 2, removed from beta 5
    appProcessPre(env, *_uid, *appDataDir, *packageName);    
}

EXPORT int nativeForkAndSpecializePost(JNIEnv *env, jclass clazz, jint res) {
    if (res != 0) return 0;
    appProcessPost(env);
    return 0;
}

EXPORT void specializeAppProcessPre(
        JNIEnv *env, jclass clazz, jint *_uid, jint *gid, jintArray *gids, jint *runtimeFlags,
        jobjectArray *rlimits, jint *mountExternal, jstring *seInfo, jstring *niceName,
        jboolean *startChildZygote, jstring *instructionSet, jstring *appDataDir,
        jstring *packageName, jobjectArray *packagesForUID, jstring *sandboxId) {
    // this is added from Android Q beta, but seems Google disabled this in following updates

    // packageName, packagesForUID, sandboxId are added from Android Q beta 2, removed from beta 5
    appProcessPost(env);
}

EXPORT int specializeAppProcessPost(
        JNIEnv *env, jclass clazz) {
    // this is added from Android Q beta, but seems Google disabled this in following updates
    appProcessPost(env);
    return 0;
}

EXPORT void onModuleLoaded() {
    // called when the shared library of Riru core is loaded
    LOGI("XHOOk LOADED");
}

}