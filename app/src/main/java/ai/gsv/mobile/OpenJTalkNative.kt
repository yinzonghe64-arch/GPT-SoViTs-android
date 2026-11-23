
package ai.gsv.mobile

/**
 * OpenJTalk 日语前端 JNI 封装。
 *
 * 音素韵律结果数组的顺序：
 * 0: phonemes
 * 1: prosody_a1
 * 2: prosody_a2
 * 3: prosody_a3
 * 4: prosody_f1
 * 5: is_question ("1" 或 "0")
 */
internal class OpenJTalkNative(
    dictionaryPath: String
) : AutoCloseable {

    private var nativeHandle: Long = 0L

    init {
        check(nativeHandle == 0L)
        nativeHandle = nativeCreate(dictionaryPath)
        check(nativeHandle != 0L) {
            "OpenJTalk initialization returned an invalid handle"
        }
    }

    fun phonemizeWithProsody(text: String): Array<String> {
        check(nativeHandle != 0L) {
            "OpenJTalk has already been closed"
        }

        require(text.isNotBlank()) {
            "Japanese input text must not be empty"
        }

        val result = nativePhonemizeWithProsody(
            nativeHandle,
            text
        )

        check(result.size == 6) {
            "Unexpected OpenJTalk result format"
        }

        return result
    }

    fun version(): String {
        check(nativeHandle != 0L) {
            "OpenJTalk has already been closed"
        }

        return nativeGetVersion()
    }

    override fun close() {
        val handle = nativeHandle
        if (handle != 0L) {
            nativeHandle = 0L
            nativeDestroy(handle)
        }
    }

    private external fun nativeCreate(
        dictionaryPath: String
    ): Long

    private external fun nativeDestroy(
        nativeHandle: Long
    )

    private external fun nativePhonemizeWithProsody(
        nativeHandle: Long,
        inputText: String
    ): Array<String>

    private external fun nativeGetVersion(): String

    companion object {
        init {
            System.loadLibrary("gsv_cpu_threads")
        }
    }
}
