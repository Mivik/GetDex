package com.mivik.mxp;

import android.util.Log;
import de.robv.android.xposed.IXposedHookInitPackageResources;
import de.robv.android.xposed.callbacks.XC_InitPackageResources;

public class MXPHookInitPackageResources implements IXposedHookInitPackageResources {
	@Override
	public void handleInitPackageResources(XC_InitPackageResources.InitPackageResourcesParam param) {
		MXPModule module = MXPModule.getInstance(Const.getApkFile());
		if (module == null) return;
		Log.v(Const.T, "handleLoadPackage");
		module.ensureLoaded();
		module.handleInitPackageResources(param);
	}
}