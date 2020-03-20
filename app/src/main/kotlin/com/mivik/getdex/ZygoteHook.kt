package com.mivik.getdex

import android.annotation.SuppressLint
import android.annotation.TargetApi
import android.app.AppComponentFactory
import android.content.pm.ApplicationInfo
import android.os.Build
import android.os.Process
import android.util.Log
import com.mivik.mxp.Const
import com.mivik.mxp.MXP
import de.robv.android.xposed.IXposedHookZygoteInit
import de.robv.android.xposed.XC_MethodHook
import de.robv.android.xposed.XposedBridge
import java.lang.reflect.WildcardType

private const val T = "GetDex-Zygote"

class ZygoteHook : IXposedHookZygoteInit {
	companion object {
		init {
			MXP.unsealHiddenApi()
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
						if (info.packageName == MXP.database["target"]) {
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