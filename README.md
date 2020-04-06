<h1 align="center">GetDex</h1>
<p>
  <a href="https://github.com/Mivik/GetDex/blob/master/LICENSE.md" target="_blank">
    <img alt="License: GPL--3.0" src="https://img.shields.io/badge/License-GPL--3.0-yellow.svg" />
  </a>
</p>

> A powerful tool to dump dex files whose instructions are replaced with nop

[中文版本](./README-zh_CN.md)

## Introduction

GetDex is designed as a Xposed plugin to dump dex, which can also fix dex opcodes that are replaced with nop.

Before starting, you should have either [Xposed](https://github.com/rovo89/XposedInstaller) or [EdXposed](https://github.com/ElderDrivers/EdXposedManager) installed on your phone. **Since GetDex is only desinged for ART-supported android versions, I recommend EdXposed since Xposed has instable support on these android versions.**

**Until now, GetDex is only tested in Android 10**.

## Usage

Here's simple two steps to dump "nopped" dex files from an application.

### Step 1. Install the plugin and reboot your phone

It's a must-do to reboot your phone since Xposed plugins need rebooting to be activated.

Note that I used [a trick](https://github.com/Mivik/MXP) to simplify this step, so you don't need to reboot anymore after your first rebooting even you changed some code in this plugin. You just need to restart the application you want to hook into (the application you want to dump dex from) to apply your changes.

### Step 2. Activate MXP and input target package name

**If your Android version is above or equal to Q, MXP should be activated manually again everytime MXP is re-installed or your phone is rebooted**

**In other situations, MXP is always activated and do not require manual activatation**

**Note that manual activatation requires ROOT**

After MXP is activated, you can input target package name in GetDex app, and click 'Confirm' to save it. After that, shutdown the target app (if needed) and restart it. You will see your dex dumped in `/data/data/[TARGET_PACKAGE_NAME]/files/getdex`. If you don't find any, please record all recent logs and post it as an issue.

## Author

👤 **Mivik**

* Website: https://mivik.gitee.io/
* Github: [@Mivik](https://github.com/Mivik)

## Show your support

Give a ⭐️ if this project helped you!

## 📝 License

Copyright © 2020 [Mivik](https://github.com/Mivik).
This project is [GPL-3.0](https://github.com/Mivik/GetDex/blob/master/LICENSE.md) licensed.
