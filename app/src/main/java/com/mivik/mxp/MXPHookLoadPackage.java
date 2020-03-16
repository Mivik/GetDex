package com.mivik.mxp;

import android.util.Log;
import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class MXPHookLoadPackage implements IXposedHookLoadPackage {
	@Override
	public void handleLoadPackage(XC_LoadPackage.LoadPackageParam param) {
		MXPModule module = MXPModule.getInstance(Const.getApkFile());
		if (module == null) return;
		Log.v(Const.T, "handleLoadPackage");
		module.ensureLoaded();
		module.handleLoadPackage(param);
	}
}