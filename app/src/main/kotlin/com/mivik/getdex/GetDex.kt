package com.mivik.getdex

import android.annotation.SuppressLint
import android.content.Context
import android.util.Log
import com.mivik.mxp.Const
import dalvik.system.BaseDexClassLoader
import dalvik.system.DexFile
import java.io.File
import java.lang.reflect.Array

private const val T = "GetDex"

class GetDex {
	companion object {
		private val LOCK = Object()
		private var initialized = false

		fun initialize(context: Context) {
			initialize(
				File(context.filesDir, "getdex"),
				context.packageManager.getApplicationInfo(Const.SELF_PACKAGE_NAME, 0).nativeLibraryDir
			)
		}

		@SuppressLint("UnsafeDynamicallyLoadedCode")
		fun initialize(outputDir: File, nativeLibDir: String) {
			synchronized(LOCK) {
				if (initialized) return
				initialized = true
			}
			File(nativeLibDir, "libsubstrate.so").absolutePath.run {
				Log.e(T, "Loading Substrate library, path: $this")
				System.load(this)
			}
			File(nativeLibDir, "libgetdex.so").absolutePath.run {
				Log.e(T, "Loading GetDex library, path: $this")
				System.load(this)
			}
			if (!outputDir.exists()) outputDir.mkdirs()
			setOutputPath(outputDir.absolutePath)
		}

		@JvmOverloads
		fun fixAllClasses(classLoader: BaseDexClassLoader, classFilter: ClassFilter? = null) {
			Log.e(T, "Fixing all classes")
			val pathList = BaseDexClassLoader::class.java.getDeclaredField("pathList").run {
				isAccessible = true
				get(classLoader)
			}
			val dexElements = pathList.javaClass.getDeclaredField("dexElements").run {
				isAccessible = true
				get(pathList)
			}
			if (dexElements == null || Array.getLength(dexElements) == 0) return
			val fieldDexFile =
				Array.get(dexElements, 0).javaClass.getDeclaredField("dexFile").apply { isAccessible = true }
			for (i in 0 until Array.getLength(dexElements)) {
				val dexFile = fieldDexFile.get(Array.get(dexElements, i)) as DexFile
				for (className in dexFile.entries()) {
					if (classFilter != null && (!classFilter.filter(className))) continue
					try {
						fixClass(classLoader.loadClass(className))
					} catch (e: ClassNotFoundException) {
						Log.e(T, "Class not found", e)
					}
				}
			}
			Log.e(T, "Finished")
		}

		/**
		 * set the output directory of native GetDex
		 */
		@JvmStatic
		external fun setOutputPath(path: String)

		/**
		 * Fix specific class
		 */
		@JvmStatic
		external fun fixClass(clz: Class<*>)
	}

	interface ClassFilter {
		fun filter(className: String): Boolean
	}
}