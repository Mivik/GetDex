package com.mivik.getdex

import android.app.Activity
import android.graphics.Color
import android.os.Bundle
import android.util.Log
import android.view.Gravity
import android.view.Window
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import com.mivik.mxp.Const

class MainActivity : Activity() {
	override fun onCreate(savedInstanceState: Bundle?) {
		requestWindowFeature(Window.FEATURE_NO_TITLE)
		super.onCreate(savedInstanceState)
		val root = LinearLayout(this)
		root.orientation = LinearLayout.VERTICAL
		root.gravity = Gravity.CENTER
		TextView(this).apply {
			text =
				String.format(getString(R.string.constants_display), Const.SELF_PACKAGE_NAME, Const.TARGET_PACKAGE_NAME)
			gravity = Gravity.CENTER
			root.addView(this)
		}
		if (Const.SELF_PACKAGE_NAME != packageName) {
			TextView(this).apply {
				text = String.format(getString(R.string.wrong_config), packageName)
				setTextColor(Color.RED)
				gravity = Gravity.CENTER
				root.addView(this)
			}
		}
		setContentView(root)
	}
}