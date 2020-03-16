package com.mivik.getdex

import android.content.Context
import android.content.ContextWrapper
import android.util.Log
import com.mivik.mxp.Const
import dalvik.system.BaseDexClassLoader
import de.robv.android.xposed.IXposedHookLoadPackage
import de.robv.android.xposed.XC_MethodHook
import de.robv.android.xposed.XposedBridge
import de.robv.android.xposed.callbacks.XC_LoadPackage

private const val T = "GetDex"

class MainHook : IXposedHookLoadPackage {

	override fun handleLoadPackage(lpparam: XC_LoadPackage.LoadPackageParam?) {
		lpparam ?: return
		if (lpparam.packageName != Const.TARGET_PACKAGE_NAME) return
		Log.i(T, "GetDex launched in the target application")
		XposedBridge.hookMethod(
			ContextWrapper::class.java.getDeclaredMethod("attachBaseContext", Context::class.java),
			object : XC_MethodHook() {
				override fun beforeHookedMethod(param: MethodHookParam?) {
					param ?: return
					val context = param.args[0] as Context
					GetDex.initialize(context)
					val classLoader = context.classLoader
					if (classLoader is BaseDexClassLoader) GetDex.fixAllClasses(
						classLoader,
						object : GetDex.ClassFilter {
							override fun filter(className: String): Boolean {
								return className.startsWith("com.")
							}
						})
					else Log.e(T, "ClassLoader is not BaseDexClassLoader, please fixClass manually")
				}
			})
	}
}