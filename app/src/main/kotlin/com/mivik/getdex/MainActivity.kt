package com.mivik.getdex

import android.app.Activity
import android.app.AlertDialog
import android.content.res.Resources
import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.Gravity
import android.view.View
import android.view.Window
import android.widget.*
import com.mivik.mxp.Const
import com.mivik.mxp.MXP
import kotlin.concurrent.thread
import kotlin.math.roundToInt

class MainActivity : Activity() {
	companion object {
		fun dp2px(dp: Int): Int = (Resources.getSystem().displayMetrics.density * dp).roundToInt()
	}

	private var activateLayouts = arrayOfNulls<View>(2)
	private var targetEdit: EditText? = null
	private var targetButton: Button? = null

	private val db = MXP.database

	override fun onCreate(savedInstanceState: Bundle?) {
		requestWindowFeature(Window.FEATURE_NO_TITLE)
		super.onCreate(savedInstanceState)
		setContentView(buildLayout())
	}

	private fun buildInputLayout(): View {
		val cx = this
		val root = LinearLayout(cx).apply {
			orientation = LinearLayout.HORIZONTAL
			gravity = Gravity.CENTER
		}
		root.addView(EditText(this).apply {
			hint = getString(R.string.target_package_name)
			setText(db["target"] as String?)
			layoutParams = LinearLayout.LayoutParams(0, -2).apply { weight = 1f }
		}.also { targetEdit = it })
		root.addView(Button(this).apply {
			text = getString(R.string.confirm)
			layoutParams = LinearLayout.LayoutParams(-2, -2)
			setOnClickListener {
				db["target"] = targetEdit!!.text.toString()
				db.apply()
				toast(R.string.saved)
			}
		}.also { targetButton = it })
		return root
	}

	private fun buildLayout(): View {
		val cx = this
		val root = LinearLayout(cx).apply {
			orientation = LinearLayout.VERTICAL
			gravity = Gravity.CENTER
		}
		root.addView(buildInputLayout().apply {
			layoutParams = LinearLayout.LayoutParams(-1, -2).apply {
				bottomMargin = dp2px(10)
				val margin = dp2px(30)
				leftMargin = margin
				rightMargin = margin
			}
		})
		root.addView(TextView(cx).apply {
			if (packageName == Const.SELF_PACKAGE_NAME) {
				text = String.format(getString(R.string.constants_display), Const.SELF_PACKAGE_NAME)
			} else {
				text = String.format(getString(R.string.wrong_config), packageName)
				setTextColor(Color.RED)
			}
			gravity = Gravity.CENTER
			layoutParams = LinearLayout.LayoutParams(-2, -2).apply { bottomMargin = dp2px(5) }
		})
		activateLayouts[0] = LinearLayout(cx).apply {
			orientation = LinearLayout.VERTICAL
			gravity = Gravity.CENTER
			addView(TextView(cx).apply {
				text = getString(R.string.mxp_not_activated)
				gravity = Gravity.CENTER
				setTextColor(Color.RED)
			})
			addView(Button(cx).apply {
				text = getString(R.string.mxp_activate)
				isAllCaps = true
				layoutParams = LinearLayout.LayoutParams(-2, -2).apply { topMargin = dp2px(5) }
				setOnClickListener {
					thread {
						try {
							MXP.activate()
						} catch (t: Throwable) {
							ui {
								AlertDialog.Builder(this@MainActivity).apply {
									setTitle(getString(R.string.error))
									setMessage(Log.getStackTraceString(t))
									setCancelable(true)
									setPositiveButton(R.string.confirm, null)
								}.show()
							}
						}
						ui { updateLayout() }
					}
				}
			})
		}
		activateLayouts[1] = LinearLayout(cx).apply {
			orientation = LinearLayout.VERTICAL
			gravity = Gravity.CENTER
			addView(TextView(cx).apply {
				text = getString(R.string.mxp_activated)
				gravity = Gravity.CENTER
				setTextColor(Color.GREEN)
			})
		}
		root.addView(activateLayouts[0])
		root.addView(activateLayouts[1])
		updateLayout()
		return root
	}

	private fun updateLayout() {
		val activated = MXP.isActivated()
		activateLayouts[if (activated) 1 else 0]!!.visibility = View.VISIBLE
		activateLayouts[if (activated) 0 else 1]!!.visibility = View.GONE
		targetEdit!!.isEnabled = activated
		targetButton!!.isEnabled = activated
	}

	val mainHandler by lazy { Handler(Looper.getMainLooper()) }

	@SuppressWarnings("private")
	inline fun ui(crossinline lambda: () -> Unit) {
		if (Looper.myLooper() == Looper.getMainLooper()) lambda()
		else mainHandler.post { lambda() }
	}

	fun toast(cs: CharSequence) = Toast.makeText(this, cs, Toast.LENGTH_SHORT).show()

	fun toast(resId: Int) = Toast.makeText(this, resId, Toast.LENGTH_SHORT).show()
}