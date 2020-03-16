<h1 align="center">GetDex</h1>
<p>
  <a href="https://github.com/Mivik/GetDex/blob/master/LICENSE.md" target="_blank">
    <img alt="License: GPL--3.0" src="https://img.shields.io/badge/License-GPL--3.0-yellow.svg" />
  </a>
</p>

> A powerful tool to dump dex files whose instructions are replaced with nop

## Author

👤 **Mivik**

* Website: https://mivik.gitee.io/
* Github: [@Mivik](https://github.com/Mivik)

## Introduction

GetDex is designed as a Xposed plugin to dump dex, which can also fix dex opcodes that are replaced with nop.

Before starting, you should have either [Xposed](https://github.com/rovo89/XposedInstaller) or [EdXposed](https://github.com/ElderDrivers/EdXposedManager) installed on your phone. **Since GetDex is only desinged for ART-supported android versions, I recommend EdXposed since Xposed has instable support on these android versions.**

Until now, GetDex is only tested in Android 10.0.

## Usage

Here's simple three steps to dump "nopped" dex files from an application.

### Step 1. Configure package names and classes you want to fix

Look into `app/src/main/java/com/mivik/mxp/Const.java` and modify the value of `SELF_PACKAGE_NAME` to the new package name if you want (note that `SELF_PACKAGE_NAME` must be equal with the one specified in project's build.gradle and AndroidManifest.xml), and **most importantly**, modify the value of `TARGET_PACKAGE_NAME` to the package you want to dump dex from. For example:

```java
public final class Const {
	public static String T = "MXP";
	public static final String SELF_PACKAGE_NAME = "com.mivik.getdex";
	public static final String TARGET_PACKAGE_NAME = "com.tencent.mobileqq";

	......
}
```

After configuring package names, you also need to configure the filter to specify what class you want to fix since fixing system classes would possibly cause plugin to crash. The filter is located in `app/src/main/kotlin/com/mivik/getdex/MainHook.kt`. The interface `ClassFilter` is defined to receive a class name and return whether the class should be fixed (all the classes in the application will be passed into this filter). For example: 

```kotlin
	......
	if (classLoader is BaseDexClassLoader) GetDex.fixAllClasses(
		classLoader,
		object : GetDex.ClassFilter {
			override fun filter(className: String): Boolean {
				return className.startsWith("com.tencent.mobileqq.")
			}
		})
	else Log.e(T, "ClassLoader is not BaseDexClassLoader, please fixClass manually")
	......
```

If you are not familiar with Kotlin, don't worry. You just need to write a java class `com.mivik.getdex.MainHook` which implemented `IXposedHookLoadPackage`, and hook the target application's `android.content.ContextWrapper.attachBaseContext` method, and do the following steps before the hooked method starts (remember to remove the corresponding kotlin file):

```java
	Context context = (Context) param.args[0]
	GetDex.initialize(context)
	if (context.classLoader instanceof BaseDexClassLoader)
		GetDex.fixAllClasses((BaseDexClassLoader) context.classLoader, new GetDex.ClassFilter() {
			@Override
			public boolean filter(String className) {
				// Your filter here
				return false;
			}
		})
```

BTW, you can call `GetDex.fixClass(Class)` to fix a single class.

### Step 2. Install the plugin and reboot your phone

It's a must-do to reboot your phone since Xposed plugins need rebooting to be activated.

Note that I used a trick to simplify this step, so you don't need to reboot anymore after your first rebooting even you changed some code in this plugin. You just need to restart the application you want to hook into (the application you want to dump dex from) to apply your changes.

### Step 3. Start the target application

That's all! After these three steps, you will see your dex dumped in `/data/data/[TARGET_PACKAGE_NAME]/files/getdex`. If you don't find any, please record all your logs contains 'GetDex' and post it as an issue.

## 🤝 Contributing

Contributions, issues and feature requests are welcome!<br />Feel free to check [issues page](https://github.com/Mivik/GetDex/issues). 

## Show your support

Give a ⭐️ if this project helped you!

## 📝 License

Copyright © 2020 [Mivik](https://github.com/Mivik).<br />
This project is [GPL-3.0](https://github.com/Mivik/GetDex/blob/master/LICENSE.md) licensed.
