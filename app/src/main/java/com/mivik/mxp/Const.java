package com.mivik.mxp;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;

public final class Const {
	public static String T = "MXP";
	public static final String SELF_PACKAGE_NAME = "com.mivik.getdex";
	public static final String TARGET_PACKAGE_NAME = "com.xxx.xxx";

	private static File APK_FILE = null;

	private Const() {
	}

	public static File getApkFile() {
		if (APK_FILE == null || (!APK_FILE.exists())) APK_FILE = getApkFile(SELF_PACKAGE_NAME);
		return APK_FILE;
	}

	/**
	 * Get the path of apk file by package name
	 * Notice that invoking from some system process will return null due to having no execution access to "pm"
	 */
	private static File getApkFile(String packageName) {
		try {
			Process pro = Runtime.getRuntime().exec(new String[]{"pm", "list", "package", "-f"});
			BufferedReader reader = new BufferedReader(new InputStreamReader(pro.getInputStream()));
			String line;
			while ((line = reader.readLine()) != null) {
				if (line.length() == 0) continue;
				final int ind = line.lastIndexOf('=');
				String cur = line.substring(ind + 1);
				if (!cur.equals(packageName)) continue;
				return new File(line.substring(8, ind));
			}
		} catch (Throwable t) {
			Log.e(Const.T, "Failed to get apk file", t);
		}
		return null;
	}
}