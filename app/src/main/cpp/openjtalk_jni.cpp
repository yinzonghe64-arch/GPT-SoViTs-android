
#include <jni.h>
#include <cstdint>
#include <string>

#include "openjtalk_native.h"

namespace {

void throwJavaException(JNIEnv* env, const char* message) {
    jclass exceptionClass =
        env->FindClass("java/lang/IllegalStateException");

    if (exceptionClass != nullptr) {
        env->ThrowNew(exceptionClass, message);
    }
}

// 将 Unicode 码点编码为标准 UTF-8
void appendUtf8(std::string& output, uint32_t codepoint) {
    if (codepoint <= 0x7F) {
        output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
        output.push_back(static_cast<char>(
            0xC0 | (codepoint >> 6)
        ));
        output.push_back(static_cast<char>(
            0x80 | (codepoint & 0x3F)
        ));
    } else if (codepoint <= 0xFFFF) {
        output.push_back(static_cast<char>(
            0xE0 | (codepoint >> 12)
        ));
        output.push_back(static_cast<char>(
            0x80 | ((codepoint >> 6) & 0x3F)
        ));
        output.push_back(static_cast<char>(
            0x80 | (codepoint & 0x3F)
        ));
    } else {
        output.push_back(static_cast<char>(
            0xF0 | (codepoint >> 18)
        ));
        output.push_back(static_cast<char>(
            0x80 | ((codepoint >> 12) & 0x3F)
        ));
        output.push_back(static_cast<char>(
            0x80 | ((codepoint >> 6) & 0x3F)
        ));
        output.push_back(static_cast<char>(
            0x80 | (codepoint & 0x3F)
        ));
    }
}

// 将 Java UTF-16 转换为标准 UTF-8
bool javaStringToUtf8(
    JNIEnv* env,
    jstring input,
    std::string& output
) {
    if (input == nullptr) {
        return false;
    }

    const jsize length = env->GetStringLength(input);
    const jchar* chars = env->GetStringChars(input, nullptr);

    if (chars == nullptr) {
        return false;
    }

    output.clear();

    for (jsize i = 0; i < length; ++i) {
        uint32_t codepoint = chars[i];

        // 高代理项
        if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
            if (i + 1 >= length) {
                env->ReleaseStringChars(input, chars);
                return false;
            }

            uint32_t low = chars[i + 1];

            if (low < 0xDC00 || low > 0xDFFF) {
                env->ReleaseStringChars(input, chars);
                return false;
            }

            codepoint =
                0x10000 +
                ((codepoint - 0xD800) << 10) +
                (low - 0xDC00);

            ++i;
        } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
            env->ReleaseStringChars(input, chars);
            return false;
        }

        appendUtf8(output, codepoint);
    }

    env->ReleaseStringChars(input, chars);
    return true;
}

void* getHandle(jlong value) {
    return reinterpret_cast<void*>(
        static_cast<uintptr_t>(value)
    );
}

std::string intArrayToString(
    const int* values,
    int count
) {
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

jstring makeJavaString(
    JNIEnv* env,
    const std::string& value
) {
    return env->NewStringUTF(value.c_str());
}

} // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_ai_gsv_mobile_OpenJTalkNative_nativeCreate(
    JNIEnv* env,
    jobject,
    jstring dictionaryPath
) {
    std::string path;

    if (!javaStringToUtf8(env, dictionaryPath, path)) {
        throwJavaException(
            env,
            "Invalid OpenJTalk dictionary path"
        );
        return 0;
    }

    void* handle = openjtalk_native_create(path.c_str());

    if (handle == nullptr) {
        throwJavaException(
            env,
            "OpenJTalk initialization failed"
        );
        return 0;
    }

    return static_cast<jlong>(
        reinterpret_cast<uintptr_t>(handle)
    );
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

    std::string text;

    if (!javaStringToUtf8(env, inputText, text)) {
        throwJavaException(env, "Invalid Japanese input text");
        return nullptr;
    }

    OpenJTalkNativeProsodyResult* result =
        openjtalk_native_phonemize_with_prosody(
            handle,
            text.c_str()
        );

    if (result == nullptr) {
        int errorCode =
            openjtalk_native_get_last_error(handle);

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
        result->phonemes != nullptr
            ? result->phonemes
            : "";

    std::string a1 = intArrayToString(
        result->prosody_a1,
        result->phoneme_count
    );

    std::string a2 = intArrayToString(
        result->prosody_a2,
        result->phoneme_count
    );

    std::string a3 = intArrayToString(
        result->prosody_a3,
        result->phoneme_count
    );

    std::string f1 = intArrayToString(
        result->prosody_f1,
        result->phoneme_count
    );

    std::string question =
        result->is_question != 0 ? "1" : "0";

    jclass stringClass =
        env->FindClass("java/lang/String");

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
    const char* version =
        openjtalk_native_get_version();

    if (version == nullptr) {
        return env->NewStringUTF("unknown");
    }

    return env->NewStringUTF(version);
}
