#include <jni.h>

#include <memory>
#include <new>
#include <mutex>
#include <string>
#include <vector>
#include <optional>
#include "bitboard.h"
#include "engine.h"
#include "position.h"
#include "search.h"
#include "tune.h"

namespace {

using namespace Stockfish;

std::once_flag initFlag;

void initialize_core() {
    std::call_once(initFlag, []() {
        Bitboards::init();
        Position::init();
    });
}

class JniEngine {
   public:
    JniEngine() {
        initialize_core();
        engine = std::make_unique<Engine>(std::nullopt);
        Tune::init(engine->get_options());
        engine->set_on_verify_networks([](std::string_view) {});
    }

    std::unique_ptr<Engine> engine;
};

void throw_java_runtime(JNIEnv* env, const char* message) {
    jclass exClass = env->FindClass("java/lang/RuntimeException");
    if (exClass)
        env->ThrowNew(exClass, message);
}

JniEngine* from_handle(JNIEnv* env, jlong handle) {
    if (handle == 0)
    {
        throw_java_runtime(env, "Invalid engine handle");
        return nullptr;
    }

    return reinterpret_cast<JniEngine*>(handle);
}

std::string to_std_string(JNIEnv* env, jstring str) {
    if (!str)
        return {};

    const char* raw = env->GetStringUTFChars(str, nullptr);
    if (!raw)
        return {};

    std::string out(raw);
    env->ReleaseStringUTFChars(str, raw);
    return out;
}

std::vector<std::string> to_string_vector(JNIEnv* env, jobjectArray arr) {
    std::vector<std::string> out;
    if (!arr)
        return out;

    const jsize len = env->GetArrayLength(arr);
    out.reserve(static_cast<size_t>(len));

    for (jsize i = 0; i < len; ++i)
    {
        auto* elem = reinterpret_cast<jstring>(env->GetObjectArrayElement(arr, i));
        out.push_back(to_std_string(env, elem));
        env->DeleteLocalRef(elem);
    }

    return out;
}

}  // namespace

extern "C" {

JNIEXPORT jlong JNICALL Java_org_stockfish_StockfishJNI_nativeCreate(JNIEnv* env, jobject) {
    auto* engine = new (std::nothrow) JniEngine();
    if (!engine)
    {
        throw_java_runtime(env, "Failed to allocate engine");
        return 0;
    }

    return reinterpret_cast<jlong>(engine);
}

JNIEXPORT void JNICALL Java_org_stockfish_StockfishJNI_nativeDestroy(JNIEnv*, jobject, jlong handle) {
    delete reinterpret_cast<JniEngine*>(handle);
}

JNIEXPORT void JNICALL Java_org_stockfish_StockfishJNI_nativeSetPosition(
  JNIEnv* env, jobject, jlong handle, jstring fen, jobjectArray moves) {
    auto* e = from_handle(env, handle);
    if (!e)
        return;

    e->engine->set_position(to_std_string(env, fen), to_string_vector(env, moves));
}

JNIEXPORT jboolean JNICALL Java_org_stockfish_StockfishJNI_nativeMakeMove(
  JNIEnv* env, jobject, jlong handle, jstring move) {
    auto* e = from_handle(env, handle);
    if (!e)
        return JNI_FALSE;

    return e->engine->make_move(to_std_string(env, move));
}

JNIEXPORT jint JNICALL Java_org_stockfish_StockfishJNI_nativeStaticEvalCp(JNIEnv* env,
                                                                           jobject,
                                                                           jlong handle) {
    auto* e = from_handle(env, handle);
    if (!e)
        return 0;

    return e->engine->static_eval();
}

JNIEXPORT jstring JNICALL Java_org_stockfish_StockfishJNI_nativeBestMoveDepth(JNIEnv* env,
                                                                               jobject,
                                                                               jlong   handle,
                                                                               jint    depth) {
    auto* e = from_handle(env, handle);
    if (!e)
        return env->NewStringUTF("");

    auto limits = Search::LimitsType{};
    limits.depth = depth;

    auto [bestMove, ponderMove] = e->engine->search_bestmove(limits);
    (void) ponderMove;
    return env->NewStringUTF(bestMove.c_str());
}

}  // extern "C"
