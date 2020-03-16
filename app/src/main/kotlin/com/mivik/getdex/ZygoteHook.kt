package com.mivik.getdex

import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.AppComponentFactory
import android.content.pm.ApplicationInfo
import android.os.Build
import android.util.Log
import com.mivik.mxp.Const
import de.robv.android.xposed.IXposedHookZygoteInit
import de.robv.android.xposed.XC_MethodHook
import de.robv.android.xposed.XposedBridge

private const val T = "GetDex-Zygote"

class ZygoteHook : IXposedHookZygoteInit {
	companion object {
		init {
			if (Build.VERSION.SDK_INT >= 28) {
				try {
					val forName =
						Class::class.java.getDeclaredMethod("forName", String::class.java)
					val vmRuntimeClass =
						forName.invoke(null, "dalvik.system.VMRuntime") as Class<*>
					val getRuntime = vmRuntimeClass.getDeclaredMethod("getRuntime")
					val setHiddenApiExemptions = vmRuntimeClass.getDeclaredMethod(
						"setHiddenApiExemptions",
						Array<String>::class.java
					)
					val sVmRuntime = getRuntime.invoke(null)
					setHiddenApiExemptions.invoke(sVmRuntime, arrayOf("L"))
					Log.i(T, "Unsealed Hidden API")
				} catch (t: Throwable) {
					Log.e(T, "Failed to unseal Hidden API", t)
				}
			}
		}
	}

	@SuppressLint("PrivateApi", "SoonBlockedPrivateApi")
	override fun initZygote(startupParam: IXposedHookZygoteInit.StartupParam?) {
		startupParam ?: return
		if (Build.VERSION.SDK_INT < 28) return
		try {
			val classLoadedApk = Class.forName("android.app.LoadedApk")
			XposedBridge.hookMethod(
				classLoadedApk.getDeclaredMethod(
					"createAppFactory",
					ApplicationInfo::class.java,
					ClassLoader::class.java
				), @TargetApi(Build.VERSION_CODES.P)
				object : XC_MethodHook() {
					val DEFAULT = AppComponentFactory::class.java.getDeclaredField("DEFAULT").get(null)

					override fun beforeHookedMethod(param: MethodHookParam?) {
						param ?: return
						val info = param.args[0] as ApplicationInfo? ?: return
						if (info.packageName == Const.TARGET_PACKAGE_NAME) {
							param.result = DEFAULT
							Log.i(T, "Intercepted ijiami from loading native libraries")
						}
					}
				})
		} catch (t: Throwable) {
			Log.e(T, "Failed to hook", t)
		}
	}
}