package com.mivik.getdex

import android.content.Context
import android.content.ContextWrapper
import android.os.Process
import android.util.Log
import com.mivik.mxp.Const
import com.mivik.mxp.MXP
import dalvik.system.BaseDexClassLoader
import de.robv.android.xposed.IXposedHookLoadPackage
import de.robv.android.xposed.XC_MethodHook
import de.robv.android.xposed.XposedBridge
import de.robv.android.xposed.callbacks.XC_LoadPackage
import java.util.concurrent.atomic.AtomicBoolean

private const val T = "GetDex"

class MainHook : IXposedHookLoadPackage {
	private val dumped = AtomicBoolean(false)

	override fun handleLoadPackage(lpparam: XC_LoadPackage.LoadPackageParam?) {
		lpparam ?: return
		if (lpparam.packageName != MXP.database["target"]) return
		Log.i(T, "GetDex launched in the target application")
		XposedBridge.hookMethod(
			ContextWrapper::class.java.getDeclaredMethod("attachBaseContext", Context::class.java),
			object : XC_MethodHook() {
				override fun beforeHookedMethod(param: MethodHookParam?) {
					param ?: return
					if (!dumped.compareAndSet(false, true)) return
					val context = param.args[0] as Context
					GetDex.initialize(context)
					val classLoader = context.classLoader
					val parent = classLoader.parent
					if (classLoader is BaseDexClassLoader) GetDex.fixAllClasses(
						classLoader,
						object : GetDex.ClassFilter {
							override fun filter(className: String): Boolean =
								try {
									parent.loadClass(className)
									false
								} catch (t: Throwable) {
									true
								}
						})
					else Log.e(T, "ClassLoader is not BaseDexClassLoader, please fixClass manually")
				}
			})
	}
}