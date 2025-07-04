#include <jni.h>
#include <string>
#include <vector>
#include <fstream>
#include <thread>
#include "third/utils/linux_helper.h"
#include "third/utils/jni_helper.hpp"
#include "third/utils/log.h"
#include "third/dobby/include/dobby.h"
#include "base/when_hook.h"
#include "base/hook.h"
#include "global/global.h"

using namespace std;

void *get_start(const vector<MapsInfo> &maps, const string &name) {
    void *start = (void *) -1;
    for (const auto &item: maps) {
        if (item.path.find(name) == string::npos) {
            continue;
        }
        if ((uint64_t) start > (uint64_t) item.region_start) {
            start = item.region_start;
        }
    }
    return start;
}

void *get_start(const string &name) {
    MapsHelper maps;
    if (maps.refresh(name) == 0) {
        LOGI("open maps error!");
        return nullptr;
    }
    return get_start(maps.mapsInfo, name);
}

bool dump_so(const string &libName, const string &save_path_dir) {
    LOGI("start dump_so %s to %s", libName.c_str(), save_path_dir.c_str());
    MapsHelper maps;
    if (maps.refresh(libName) == 0) {
        LOGI("dump_so open maps error!");
        return false;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%d_%p_%s",
             save_path_dir.c_str(),
             getpid(),
             maps.get_module_base(libName),
             libName.c_str());

    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        LOGI("dump_so open write file error!");
        return false;
    }

    for (const auto &item: maps.mapsInfo) {
        LOGI("dump_so write offset: %p, start: %p, end: %p, perm: %s, path: %s", item.region_offset,
             item.region_start, item.region_end, item.permissions.c_str(), item.path.c_str());

        fseek(fp, (long) item.region_offset, SEEK_SET);
        fwrite((char *) item.region_start, 1,
               (size_t)((uint64_t) item.region_end - (uint64_t) item.region_start), fp);
    }

    fclose(fp);

    return true;
}


static bool hadDump = false;

jint (*org_RegisterNatives)(JNIEnv *env, jclass java_class, const JNINativeMethod *methods,
                            jint method_count);

static jint
RegisterNatives(JNIEnv *env, jclass java_class, const JNINativeMethod *methods, jint method_count) {
    if (!hadDump) {
        for (int i = 0; i < method_count; ++i) {
            if (strstr(methods[i].name, "IiIiiIiIiI") == nullptr) {
                auto *thd = new thread([]() {
                    sleep(6);
                    dump_so("libcompatible.so",
                            "/data/data/com.com2usholdings.heiroflight2.android.google.global.normal");
                });
                hadDump = true;
                break;
            }
        }
    }
    return org_RegisterNatives(env, java_class, methods, method_count);
}

bool dump_so_when_register_natives() {
    auto target = DobbySymbolResolver("libart.so",
                                      "_ZN3art3JNI15RegisterNativesEP7_JNIEnvP7_jclassPK15JNINativeMethodi");
    DobbyHook(target, (dobby_dummy_func_t) &RegisterNatives,
              (dobby_dummy_func_t *) &org_RegisterNatives);
    return true;
}

bool dump_so_when_init(const string &targetLibName) {
    return WhenSoInitHook(targetLibName,
                          [=](const string &path, void *addr, const string &funcType) {
                              dump_so(targetLibName, "/data/data/" + getPkgName());
                          });
}

bool dump_so_delay_after_so_load(const string &targetLibName, int sleepTime) {
    return WhenSoInitHook(targetLibName,
                          [=](const string &path, void *addr, const string &funcType) {
                              logi("dump_so_delay on callback");
                              auto *thd = new thread([&]() {
                                  sleep(sleepTime);
                                  dump_so(targetLibName,
                                          "/data/data/" + getPkgName());
                              });
                              thd->join();
                          });
}

bool dump_so_delay(const string &targetLibName, int sleepTime) {
    logi("dump_so_delay on callback");
    (new thread([=]() {
        sleep(sleepTime);
        dump_so(targetLibName,
                "/data/data/" + getPkgName());
    }))->detach();
    return true;
}

static string gDumpSoName;

DefineHookStub(JNI_OnLoad, jint, JavaVM * vm, void *reserved) {
    auto ret = pHook_JNI_OnLoad(vm, reserved);
    dump_so(gDumpSoName, "/data/data/" + getPkgName());
    return ret;
}

bool dump_so_when_after_jni_load(const string &targetLibName) {
    return WhenSoInitHook(targetLibName,
                          [&](const string &path, void *addr, const string &funcType) {

                          });
}