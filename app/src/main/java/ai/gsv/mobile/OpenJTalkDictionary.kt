
package ai.gsv.mobile

import android.content.Context
import java.io.File
import java.io.FileOutputStream
import java.util.zip.ZipInputStream

/**
 * OpenJTalk 日语词典管理器。
 *
 * 首次使用时，将 assets/openjtalk-dictionary.zip
 * 解压到应用内部存储，并返回 dictionary 目录路径。
 */
internal object OpenJTalkDictionary {

    private const val ASSET_NAME = "openjtalk-dictionary.zip"
    private const val DIRECTORY_NAME = "openjtalk"

    @Synchronized
    fun getDictionaryPath(context: Context): String {
        val appContext = context.applicationContext
        val root = File(appContext.filesDir, DIRECTORY_NAME)
        val dictionary = File(root, "dictionary")
        val pending = File(root, "dictionary.pending")

        // 已经解压完成，直接使用。
        if (isValidDictionary(dictionary)) {
            return dictionary.absolutePath
        }

        root.mkdirs()

        // 清理上一次可能中断的解压。
        pending.deleteRecursively()

        try {
            ZipInputStream(
                appContext.assets.open(ASSET_NAME).buffered()
            ).use { zip ->

                val extractionRoot = pending.canonicalFile
                var entry = zip.nextEntry

                while (entry != null) {
                    val output = File(pending, entry.name).canonicalFile

                    // 防止 ZIP 路径穿越。
                    require(
                        output.path.startsWith(
                            extractionRoot.path + File.separator
                        ) || output == extractionRoot
                    ) {
                        "Invalid dictionary ZIP entry: ${entry.name}"
                    }

                    if (entry.isDirectory) {
                        output.mkdirs()
                    } else {
                        output.parentFile?.mkdirs()

                        FileOutputStream(output).buffered().use { stream ->
                            zip.copyTo(stream)
                        }
                    }

                    zip.closeEntry()
                    entry = zip.nextEntry
                }
            }

            val extractedDictionary = File(pending, "dictionary")

            check(isValidDictionary(extractedDictionary)) {
                "OpenJTalk dictionary extraction failed: required files are missing"
            }

            // 替换旧词典。
            if (dictionary.exists()) {
                check(dictionary.deleteRecursively()) {
                    "Could not remove the old dictionary"
                }
            }

            check(extractedDictionary.renameTo(dictionary)) {
                "Could not install the extracted dictionary"
            }

            pending.deleteRecursively()

            return dictionary.absolutePath

        } catch (error: Throwable) {
            pending.deleteRecursively()
            throw IllegalStateException(
                "Failed to install OpenJTalk dictionary",
                error
            )
        }
    }

    private fun isValidDictionary(directory: File): Boolean {
        return directory.isDirectory &&
            File(directory, "sys.dic").isFile &&
            File(directory, "matrix.bin").isFile &&
            File(directory, "char.bin").isFile &&
            File(directory, "unk.dic").isFile
    }
}
