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
        allowPkg = ReadFile("/data/pkg");
        allowPkg = allowPkg.replace("\n", "");
        allowPkg = allowPkg.replace("\r", "");
        logi("inject pkg list: %s", pkg.c_str());
    }

    void postAppSpecialize(const AppSpecializeArgs *args) {
        if (process.find(allowPkg) == -1) {
            return;
        }
        logi("inject to %s", allowPkg.c_str());
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