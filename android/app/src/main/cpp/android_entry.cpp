// Точка входа на Android: android_main из raylib (rcore_android.c) вызывает main().
#include <android/asset_manager.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>

extern "C" struct android_app *GetAndroidApp(void);
int EadorMain(int argc, char **argv);

// Без -Wl,--wrap=fopen (снят в CMakeLists.txt) __wrap_fopen из raylib не
// используется, но ссылается на __real_fopen; --no-undefined требует символ.
extern "C" FILE *__real_fopen(const char *path, const char *mode) {
    return fopen(path, mode);
}

#define LOG_TAG "Eador"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// eador.cfg из assets, если в каталоге данных его ещё нет.
static void ensure_config(AAssetManager *mgr) {
    if (access("eador.cfg", F_OK) == 0 || !mgr) return;
    AAsset *asset = AAssetManager_open(mgr, "eador.cfg", AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("assets/eador.cfg not found");
        return;
    }
    FILE *out = fopen("eador.cfg", "wb");
    if (out) {
        fwrite(AAsset_getBuffer(asset), 1, (size_t)AAsset_getLength(asset), out);
        fclose(out);
    }
    AAsset_close(asset);
}

int main(int argc, char **argv) {
    android_app *app = GetAndroidApp();
    const char *dir = app->activity->externalDataPath;
    if (!dir) dir = app->activity->internalDataPath;
    if (!dir || chdir(dir) != 0) {
        LOGE("cannot chdir to data dir '%s'", dir ? dir : "(null)");
        return 1;
    }
    LOGI("data dir: %s", dir);
    mkdir("save", 0755);
    mkdir("screens", 0755);
    ensure_config(app->activity->assetManager);
    return EadorMain(argc, argv);
}
