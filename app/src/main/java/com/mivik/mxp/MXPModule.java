package com.mivik.mxp;

import android.util.Log;
import dalvik.system.PathClassLoader;
import de.robv.android.xposed.IXposedHookInitPackageResources;
import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.callbacks.XC_InitPackageResources;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

import java.io.*;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

class MXPModule {
	private static MXPModule INSTANCE;

	private File apkFile;
	private long lastModifiedTime, lastSize;
	private final ArrayList<IXposedHookLoadPackage> HookLoadPackages = new ArrayList<>();
	private final ArrayList<IXposedHookInitPackageResources> HookInitPackageResources = new ArrayList<>();

	private MXPModule(File apkFile) {
		this.apkFile = apkFile;
		load();
	}

	public static synchronized MXPModule getInstance(File file) {
		if (file == null) return null;
		if (INSTANCE == null) INSTANCE = new MXPModule(file);
		return INSTANCE;
	}

	public boolean hasChanged() {
		synchronized (this) {
			if (lastSize != apkFile.length()) return true;
			return lastModifiedTime != apkFile.lastModified();
		}
	}

	public void ensureLoaded() {
		if (hasChanged()) {
			Log.i(Const.T, "Apk file changed, reload");
			load();
			return;
		}
		Log.v(Const.T, "Apk file not changed");
	}

	private void sync() {
		synchronized (this) {
			lastModifiedTime = apkFile.lastModified();
			lastSize = apkFile.length();
		}
	}

	public void handleLoadPackage(XC_LoadPackage.LoadPackageParam param) {
		for (IXposedHookLoadPackage one : HookLoadPackages) {
			try {
				one.handleLoadPackage(param);
			} catch (Throwable t) {
				Log.e(Const.T, "Module " + one.getClass().getName(), t);
			}
		}
	}

	public void handleInitPackageResources(XC_InitPackageResources.InitPackageResourcesParam param) {
		for (IXposedHookInitPackageResources one : HookInitPackageResources) {
			try {
				one.handleInitPackageResources(param);
			} catch (Throwable t) {
				Log.e(Const.T, "Module " + one.getClass().getName(), t);
			}
		}
	}

	public boolean load() {
		HookLoadPackages.clear();
		HookInitPackageResources.clear();
		InputStream in = null;
		ZipFile file = null;
		ZipEntry entry;
		try {
			file = new ZipFile(apkFile);
			entry = file.getEntry("assets/xposed_init_real");
			if (entry == null) {
				Log.e(Const.T, "assets/xposed_init is not found in " + apkFile);
				return false;
			}
			in = file.getInputStream(entry);
		} catch (IOException e) {
			Log.e(Const.T, "Failed to load module from " + apkFile, e);
			try {
				if (file != null) file.close();
			} catch (IOException ee) {
				Log.e(Const.T, "Failed to close", ee);
			}
			return false;
		}
		final ClassLoader loader = new PathClassLoader(apkFile.getAbsolutePath(), XposedBridge.BOOTCLASSLOADER);
		BufferedReader reader = null;
		try {
			reader = new BufferedReader(new InputStreamReader(in));
			String line;
			while ((line = reader.readLine()) != null) {
				line = line.trim();
				if (line.isEmpty() || line.startsWith("#")) continue;
				try {
					Log.i(Const.T, "Loading class " + line);
					Class<?> moduleClass = loader.loadClass(line);
					Object module = moduleClass.newInstance();
					if (module instanceof IXposedHookLoadPackage) HookLoadPackages.add((IXposedHookLoadPackage) module);
					else if (module instanceof IXposedHookInitPackageResources)
						HookInitPackageResources.add((IXposedHookInitPackageResources) module);
					else
						Log.e(Const.T, "Unsupported class: " + moduleClass);
				} catch (Throwable t) {
					Log.e(Const.T, "Failed to load class " + line, t);
				}
			}
		} catch (IOException e) {
			Log.e(Const.T, "Failed to read xposed_init file");
			return false;
		} finally {
			try {
				if (reader != null) reader.close();
				in.close();
				file.close();
			} catch (IOException e) {
				Log.e(Const.T, "Failed to close file");
			}
		}
		sync();
		return true;
	}
}