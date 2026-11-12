// Символы с клавиатуры: на Android raylib не заполняет очередь GetCharPressed(),
// поэтому GameActivity передаёт их сюда из UI-потока, а игра читает в потоке android_main.
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <jni.h>
#include <deque>
#include <mutex>

extern "C" struct android_app *GetAndroidApp(void);

static std::mutex g_mutex;
static std::deque<int> g_chars;

int eador_android_next_char() {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_chars.empty()) return 0;
    int c = g_chars.front();
    g_chars.pop_front();
    return c;
}

// Экранная клавиатура для UI_Input. Вызывается из потока android_main:
// NativeActivity сама передаёт запрос в UI-поток.
void eador_android_show_keyboard(bool show) {
    android_app *app = GetAndroidApp();
    if (!app || !app->activity) return;
    if (show)
        ANativeActivity_showSoftInput(app->activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED);
    else
        ANativeActivity_hideSoftInput(app->activity, 0);
}

extern "C" JNIEXPORT void JNICALL
Java_com_eador_reforged_GameActivity_nativeOnChar(JNIEnv *, jclass, jint code_point) {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (g_chars.size() < 256) g_chars.push_back((int)code_point);
}
