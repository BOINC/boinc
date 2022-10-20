package edu.berkeley.boinc.attach

import android.content.pm.PackageManager
import android.content.pm.PackageManager.PackageInfoFlags
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import edu.berkeley.boinc.R
import edu.berkeley.boinc.utils.Logging

class AboutActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_about)

        val tvVersion = findViewById<TextView>(R.id.BOINCVersionTextView)

        try {
            val packageInfo = if (VERSION.SDK_INT >= VERSION_CODES.TIRAMISU) {
                packageManager.getPackageInfo(packageName, PackageInfoFlags.of(0))
            } else {
                @Suppress("DEPRECATION")
                packageManager.getPackageInfo(packageName, 0)
            }
            tvVersion.text = getString(R.string.about_version, packageInfo.versionName)
        } catch (e: PackageManager.NameNotFoundException) {
            Logging.logWarning(Logging.Category.USER_ACTION, "version name not found.")
        }
    }
}