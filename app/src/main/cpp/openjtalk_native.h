
#ifndef OPENJTALK_NATIVE_H
#define OPENJTALK_NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPENJTALK_NATIVE_SUCCESS = 0,
    OPENJTALK_NATIVE_ERROR_INVALID_HANDLE = -1,
    OPENJTALK_NATIVE_ERROR_INVALID_INPUT = -2,
    OPENJTALK_NATIVE_ERROR_MEMORY_ALLOCATION = -3,
    OPENJTALK_NATIVE_ERROR_DICTIONARY_NOT_FOUND = -4,
    OPENJTALK_NATIVE_ERROR_INITIALIZATION_FAILED = -5,
    OPENJTALK_NATIVE_ERROR_PHONEMIZATION_FAILED = -6,
    OPENJTALK_NATIVE_ERROR_PROCESSING = -7,
    OPENJTALK_NATIVE_ERROR_INVALID_OPTION = -8,
    OPENJTALK_NATIVE_ERROR_INVALID_DICTIONARY = -9,
    OPENJTALK_NATIVE_ERROR_INVALID_UTF8 = -10
} OpenJTalkNativeError;

typedef struct {
    char* phonemes;
    int* phoneme_ids;
    int phoneme_count;
    float* durations;
    float total_duration;
} OpenJTalkNativePhonemeResult;

typedef struct {
    char* phonemes;
    int* prosody_a1;
    int* prosody_a2;
    int* prosody_a3;
    int* prosody_f1;
    int phoneme_count;
    int is_question;
} OpenJTalkNativeProsodyResult;

const char* openjtalk_native_get_version(void);

void* openjtalk_native_create(const char* dict_path);
void openjtalk_native_destroy(void* handle);

OpenJTalkNativePhonemeResult* openjtalk_native_phonemize(
    void* handle,
    const char* text
);

void openjtalk_native_free_result(
    OpenJTalkNativePhonemeResult* result
);

OpenJTalkNativeProsodyResult* openjtalk_native_phonemize_with_prosody(
    void* handle,
    const char* text
);

void openjtalk_native_free_prosody_result(
    OpenJTalkNativeProsodyResult* result
);

int openjtalk_native_get_last_error(void* handle);
const char* openjtalk_native_get_error_string(int error_code);

int openjtalk_native_set_option(
    void* handle,
    const char* key,
    const char* value
);

const char* openjtalk_native_get_option(
    void* handle,
    const char* key
);

void* openjtalk_native_initialize_utf8(
    const unsigned char* dict_path_utf8,
    int path_length
);

char* openjtalk_native_analyze_utf8(
    void* handle,
    const unsigned char* text_utf8,
    int text_length
);

void* openjtalk_native_initialize(const char* dict_path);
char* openjtalk_native_analyze(void* handle, const char* text);
void openjtalk_native_finalize(void* handle);
void openjtalk_native_free_string(char* result);

#ifdef __cplusplus
}
#endif

#endif
