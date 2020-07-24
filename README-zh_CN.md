<h1 align="center">GetDex</h1>
<p>
  <a href="https://github.com/Mivik/GetDex/blob/master/LICENSE.md" target="_blank">
    <img alt="License: GPL--3.0" src="https://img.shields.io/badge/License-GPL--3.0-yellow.svg" />
  </a>
</p>

> 一个修复 "被nop" 的 dex 文件的工具

## 介绍

GetDex 是一个用于导出 dex 的 Xposed 插件，同时也能修复指令被替换为 nop 的 dex 文件。

在开始之前，你应该在你的手机上安装 Xposed 或 EdXposed。**由于 GetDex 只为支持 ART 的安卓版本设计，我建议使用 EdXposed 因为 Xposed 对于高版本支持 ART 的手机支持并不完善。**

**到目前为止，GetDex 只在 Android 10 上测试通过。**

## 用法

下面是一个简单的从一个应用中导出并修复 "被nop" 的 dex 文件的过程。

### 第一步. 安装插件并重启手机

你必须要重启手机来让 Xposed 模块生效。

值得一提的是我用了一个 [小技巧](https://github.com/Mivik/MXP) 来简化这一步骤。在首次重启后，你就再也不需要重启了，即使你对 GetDex 插件的内容作出了修改并重新安装。你只需要简单地重启目标程序即可应用你的更改。 

### 第二步. 激活 MXP 并输入目标包名

**如果你的 Android 版本大于等于 Q，MXP 在每次被重装或者手机重启后将需要人工激活**

**否则，MXP 将总会是激活的，不需要人工操作**

**请注意人工激活需要 ROOT 权限**

在 MXP 被激活后，你就可以在 GetDex 的主界面中输入目标程序的包名，并点击“确定”来保存。在此之后，关闭目标程序（如果需要）并重启它，你应该就能在 `/data/data/[目标程序包名]/files/getdex` 文件夹下找到修复好的 dex 文件。如果你并没有找到任何文件，请收集你的日志并提一个 issue。

## 预览

原本的 "被nop" 的 dex：

![Before](https://s1.ax1x.com/2020/04/06/Gyy1Ff.jpg)

GetDex 修复后：

![After](https://s1.ax1x.com/2020/04/06/Gyy8fS.jpg)

## 作者

👤 **Mivik**

* 个人主页: https://mivik.gitee.io/
* Github: [@Mivik](https://github.com/Mivik)

## 喜欢这个项目？

那就给个 ⭐️ 吧！

## 📝 许可协议

Copyright © 2020 [Mivik](https://github.com/Mivik).<br />
This project is [GPL-3.0](https://github.com/Mivik/GetDex/blob/master/LICENSE.md) licensed.
