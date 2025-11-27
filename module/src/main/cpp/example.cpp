#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>

#include "third/utils/utils.h"
#include "third/utils/log.h"
#include "third/utils/linux_helper.h"
#include "zygisk.hpp"

using namespace std;
using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;


class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        this->process = process;
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void postAppSpecialize(const AppSpecializeArgs *args) {
        allowPkg = ReadFile("/data/pkg");
        if (allowPkg.empty()) {
            loge("pkg empty %d", errno);
            return;
        }
        allowPkg = replace_all(allowPkg, "\n", "");
        allowPkg = replace_all(allowPkg, "\r", "");
        logi("pkg list: %s", allowPkg.c_str());
        if (process.find(allowPkg) == -1) {
            return;
        }
        auto handle = dlopen("/data/libanalyse.so", RTLD_NOW);
        if (handle != nullptr) {
            logi("inject to %s", allowPkg.c_str());
        } else {
            loge("inject to %s error %d", allowPkg.c_str(), errno);
            return;
        }
        void *inject_entry = dlsym(handle, "inject_entry");
        if (!inject_entry) {
            loge("inject_entry error %d", errno);
            return;
        }
        ((void (*)(JNIEnv *)) inject_entry)(env);
    }

private:
    Api *api;
    JNIEnv *env;
    string process;
    string allowPkg;
};

static void companion_handler(int i) {
}

// Register our module class and the companion handler function
REGISTER_ZYGISK_MODULE(MyModule)

REGISTER_ZYGISK_COMPANION(companion_handler)