
#include <jni.h>
#include <cstdint>
#include <string>
#include <stdexcept>

#include "openjtalk_native.h"

namespace {

void throwJavaException(JNIEnv* env, const char* message) {
    jclass exceptionClass = env->FindClass("java/lang/IllegalStateException");
    if (exceptionClass != nullptr) {
        env->ThrowNew(exceptionClass, message);
    }
}

void* getHandle(jlong value) {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(value));
}

std::string intArrayToString(const int* values, int count) {
    std::string output;

    if (values == nullptr || count <= 0) {
        return output;
    }

    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            output += ",";
        }
        output += std::to_string(values[i]);
    }

    return output;
}

jstring makeJavaString(JNIEnv* env, const std::string& value) {
    return env->NewStringUTF(value.c_str());
}

} // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_ai_gsv_mobile_OpenJTalkNative_nativeCreate(
    JNIEnv* env,
    jobject,
    jstring dictionaryPath
) {
    if (dictionaryPath == nullptr) {
        throwJavaException(env, "OpenJTalk dictionary path is null");
        return 0;
    }

    const char* path = env->GetStringUTFChars(dictionaryPath, nullptr);
    if (path == nullptr) {
        return 0;
    }

    void* handle = openjtalk_native_create(path);

    env->ReleaseStringUTFChars(dictionaryPath, path);

    if (handle == nullptr) {
        throwJavaException(env, "OpenJTalk initialization failed");
        return 0;
    }

    return static_cast<jlong>(reinterpret_cast<uintptr_t>(handle));
}

extern "C" JNIEXPORT void JNICALL
Java_ai_gsv_mobile_OpenJTalkNative_nativeDestroy(
    JNIEnv*,
    jobject,
    jlong nativeHandle
) {
    void* handle = getHandle(nativeHandle);

    if (handle != nullptr) {
        openjtalk_native_destroy(handle);
    }
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_ai_gsv_mobile_OpenJTalkNative_nativePhonemizeWithProsody(
    JNIEnv* env,
    jobject,
    jlong nativeHandle,
    jstring inputText
) {
    void* handle = getHandle(nativeHandle);

    if (handle == nullptr) {
        throwJavaException(env, "Invalid OpenJTalk handle");
        return nullptr;
    }

    if (inputText == nullptr) {
        throwJavaException(env, "Japanese input text is null");
        return nullptr;
    }

    const char* text = env->GetStringUTFChars(inputText, nullptr);
    if (text == nullptr) {
        return nullptr;
    }

    OpenJTalkNativeProsodyResult* result =
        openjtalk_native_phonemize_with_prosody(handle, text);

    env->ReleaseStringUTFChars(inputText, text);

    if (result == nullptr) {
        int errorCode = openjtalk_native_get_last_error(handle);
        const char* errorMessage =
            openjtalk_native_get_error_string(errorCode);

        throwJavaException(
            env,
            errorMessage != nullptr
                ? errorMessage
                : "OpenJTalk phonemization failed"
        );
        return nullptr;
    }

    const char* phonemes =
        result->phonemes != nullptr ? result->phonemes : "";

    std::string a1 = intArrayToString(
        result->prosody_a1, result->phoneme_count
    );

    std::string a2 = intArrayToString(
        result->prosody_a2, result->phoneme_count
    );

    std::string a3 = intArrayToString(
        result->prosody_a3, result->phoneme_count
    );

    std::string f1 = intArrayToString(
        result->prosody_f1, result->phoneme_count
    );

    std::string question =
        result->is_question != 0 ? "1" : "0";

    jclass stringClass = env->FindClass("java/lang/String");

    if (stringClass == nullptr) {
        openjtalk_native_free_prosody_result(result);
        return nullptr;
    }

    jobjectArray output = env->NewObjectArray(
        6,
        stringClass,
        nullptr
    );

    if (output == nullptr) {
        openjtalk_native_free_prosody_result(result);
        return nullptr;
    }

    env->SetObjectArrayElement(
        output, 0, makeJavaString(env, phonemes)
    );

    env->SetObjectArrayElement(
        output, 1, makeJavaString(env, a1)
    );

    env->SetObjectArrayElement(
        output, 2, makeJavaString(env, a2)
    );

    env->SetObjectArrayElement(
        output, 3, makeJavaString(env, a3)
    );

    env->SetObjectArrayElement(
        output, 4, makeJavaString(env, f1)
    );

    env->SetObjectArrayElement(
        output, 5, makeJavaString(env, question)
    );

    openjtalk_native_free_prosody_result(result);

    return output;
}

extern "C" JNIEXPORT jstring JNICALL
Java_ai_gsv_mobile_OpenJTalkNative_nativeGetVersion(
    JNIEnv* env,
    jobject
) {
    const char* version = openjtalk_native_get_version();

    if (version == nullptr) {
        return env->NewStringUTF("unknown");
    }

    return env->NewStringUTF(version);
}

