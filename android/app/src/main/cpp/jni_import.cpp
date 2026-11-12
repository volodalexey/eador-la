// Импорт ресурсов GOG для LauncherActivity — порт cmake/import-gog.sh.
// Копирует dat/, text/, var/, шрифты и иконку; save/, screens/, eador.cfg не трогает.
#include <jni.h>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>

#include "innounp.h"

namespace fs = std::filesystem;

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return s;
}

// Первый элемент корня game с именем name без учёта регистра.
static fs::path find_ci(const fs::path &game, const std::string &name, bool want_dir) {
    const std::string needle = lower(name);
    for (const auto &e : fs::directory_iterator(game)) {
        if (want_dir ? !e.is_directory() : !e.is_regular_file()) continue;
        if (lower(e.path().filename().string()) == needle) return e.path();
    }
    return {};
}

static fs::path find_ico(const fs::path &game) {
    for (const auto &e : fs::directory_iterator(game)) {
        if (!e.is_regular_file()) continue;
        std::string n = lower(e.path().filename().string());
        if (n.rfind("goggame-", 0) == 0 && n.size() > 4 && n.substr(n.size() - 4) == ".ico")
            return e.path();
    }
    return {};
}

static void import_game(const fs::path &game, const fs::path &runtime) {
    fs::create_directories(runtime);
    for (const char *dir : {"dat", "text", "var"}) {
        fs::path real = find_ci(game, dir, true);
        if (real.empty())
            throw std::runtime_error(std::string("no ") + dir + "/ directory, not Eador: Genesis");
        fs::remove_all(runtime / dir);
        fs::copy(real, runtime / dir,
                 fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    }
    if (!fs::exists(runtime / "dat" / "Interface.dat"))
        throw std::runtime_error("no dat/Interface.dat, not Eador: Genesis");

    for (const char *f : {"font01.pcx", "font01.txt", "font02.pcx", "font02.txt"}) {
        fs::path real = find_ci(game, f, false);
        if (real.empty()) throw std::runtime_error(std::string("no ") + f);
        fs::copy_file(real, runtime / f, fs::copy_options::overwrite_existing);
    }

    fs::path ico = find_ico(game);
    if (!ico.empty())
        fs::copy_file(ico, runtime / "eador.ico", fs::copy_options::overwrite_existing);

    fs::create_directories(runtime / "save");
    fs::create_directories(runtime / "screens");
}

static std::string to_string(JNIEnv *env, jstring s) {
    const char *c = env->GetStringUTFChars(s, nullptr);
    std::string r(c ? c : "");
    if (c) env->ReleaseStringUTFChars(s, c);
    return r;
}

// nullptr — успех, иначе текст ошибки.
extern "C" JNIEXPORT jstring JNICALL
Java_com_eador_reforged_LauncherActivity_nativeImportExe(JNIEnv *env, jobject thiz,
                                                         jstring exe_path, jstring data_dir) {
    const fs::path runtime = to_string(env, data_dir);
    const fs::path tmp = runtime / ".gog_extract";
    jclass cls = env->GetObjectClass(thiz);
    jmethodID on_progress = env->GetMethodID(cls, "onProgress", "(JJ)V");
    try {
        fs::remove_all(tmp);
        fs::create_directories(tmp);
        uint64_t last_percent = 101;
        innounp::extract(to_string(env, exe_path), tmp.string(),
                         [&](uint64_t done, uint64_t total) {
                             uint64_t p = total ? done * 100 / total : 0;
                             if (p == last_percent || !on_progress) return;
                             last_percent = p;
                             env->CallVoidMethod(thiz, on_progress, (jlong)done, (jlong)total);
                         });
        import_game(tmp, runtime);
        fs::remove_all(tmp);
        return nullptr;
    } catch (const std::exception &e) {
        std::error_code ec;
        fs::remove_all(tmp, ec);
        return env->NewStringUTF(e.what());
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_eador_reforged_LauncherActivity_nativeImportFolder(JNIEnv *env, jobject,
                                                            jstring source_dir, jstring data_dir) {
    try {
        import_game(to_string(env, source_dir), to_string(env, data_dir));
        return nullptr;
    } catch (const std::exception &e) {
        return env->NewStringUTF(e.what());
    }
}
