
package ai.gsv.mobile

import android.content.Context

object GsvRuntime {
    val engine = TtsEngine(listOf(QnnHtpBackend(), CpuBackend()))
    val engineLock = Any()

    @Volatile
    private var appContext: Context? = null

    private var activityInstances = 0
    private var serviceInstances = 0

    /**
     * 保存 Application Context，供词典解压和原生前端使用。
     * 不保存 Activity Context，避免造成 Activity 内存泄漏。
     */
    fun initialize(context: Context) = synchronized(engineLock) {
        appContext = context.applicationContext
    }

    fun requireApplicationContext(): Context {
        return checkNotNull(appContext) {
            "GsvRuntime has not been initialized with an application context"
        }
    }

    @Volatile
    var apiEndpoint: String? = null
        internal set

    @Volatile
    var apiError: String? = null
        internal set

    fun retainActivity() = synchronized(engineLock) {
        activityInstances++
    }

    fun releaseActivity(finishing: Boolean) = synchronized(engineLock) {
        activityInstances = (activityInstances - 1).coerceAtLeast(0)
        if (finishing) closeEngineIfUnused()
    }

    fun retainService() = synchronized(engineLock) {
        serviceInstances++
    }

    fun releaseService() = synchronized(engineLock) {
        serviceInstances = (serviceInstances - 1).coerceAtLeast(0)
        closeEngineIfUnused()
    }

    private fun closeEngineIfUnused() {
        if (activityInstances == 0 && serviceInstances == 0) {
            engine.close()
        }
    }
}
