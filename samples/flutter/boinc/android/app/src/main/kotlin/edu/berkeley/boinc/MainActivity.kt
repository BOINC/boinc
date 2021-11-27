package edu.berkeley.boinc

import androidx.annotation.NonNull
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodChannel
import java.io.File
import java.io.IOException
import java.io.InputStream
import android.util.Log

import org.apache.commons.io.FileUtils
import java.util.*


class MainActivity: FlutterActivity() {
  private val CHANNEL = "edu.berkeley.boinc/client"
  private val TAG = "Android kotlin FlutterActivity"
  private val INSTALL_FAILED = "Failed to install: "
  private val IOEXCEPTION_LOG = "IOException: "
  private var isInstall = false
  private val boincWorkingDir = "/data/data/edu.berkeley.boinc/client/"
  private val fileNameClient = "boinc"
  private val fileNameClientCmd = "boinccmd"
  private val fileNameCABundle = "ca-bundle.crt"
  private val fileNameClientConfig = "cc_config.xml"
  private val fileNameAllProjectsList = "all_projects_list.xml"
  private val fileNameNoMedia = "nomedia"

  internal fun InputStream.copyToFile(destFile: File) = FileUtils.copyInputStreamToFile(this, destFile)
  internal fun CharSequence.containsAny(vararg sequences: CharSequence) = sequences.any { it in this }

  override fun configureFlutterEngine(@NonNull flutterEngine: FlutterEngine) {
    super.configureFlutterEngine(flutterEngine)
    MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL).setMethodCallHandler { call, result ->
      // Note: this methods is invoked on the main thread.
      when (call.method) {
        "runClient" -> {
          val isRunning = runClient()
          result.success(isRunning)
        }
        else -> {
          result.notImplemented()
        }
      }
    }
  }

  private fun runClient(): Boolean {
    var success = false

    if (!isInstall) {
      success = installClient()
      isInstall = success
    }

    Log.i(TAG, "isInstall: $isInstall")

    if (isInstall) {
      success = false
      val param = "--allow_remote_gui_rpc"
      val cmd = arrayOf(boincWorkingDir + fileNameClient, "--daemon", param)
      try {
        Log.i(TAG, "Launching '${cmd[0]}' from '$boincWorkingDir'")

        Runtime.getRuntime().exec(cmd, null, File(boincWorkingDir))
        success = true
      } catch (e: IOException) {
        Log.e(TAG, "Starting BOINC client failed with exception: " + e.message)
        Log.e(TAG, "IOException", e)
      }
    }
    return success
  }


  /**
   * Copies given file from APK assets to internal storage.
   *
   * @param file       name of file as it appears in assets directory
   * @param executable set executable flag of file in internal storage
   * @param targetFile name of target file
   * @return Boolean success
   */
  private fun installFile(file: String, executable: Boolean, targetFile: String): Boolean {
    var success = false
    val flutter_prefix = "flutter_assets/assets/"
    // If file is executable, cpu architecture has to be evaluated
    // and assets directory select accordingly
    val source = flutter_prefix + if (executable) assetsDirForCpuArchitecture + file else file
    val target = if (targetFile.isNotEmpty()) {
      File(boincWorkingDir + targetFile)
    } else {
      File(boincWorkingDir + file)
    }
    try {
      // Copy file from the asset manager to clientPath
      applicationContext.assets.open(source).copyToFile(target)
      success = true //copy succeeded without exception

      // Set executable, if requested
      if (executable) {
        success = target.setExecutable(true) // return false, if not executable
      }

      Log.d(TAG, "Installation of " + source + " successful. Executable: " +
              executable + "/" + success)
    } catch (ioe: IOException) {
      Log.e(TAG, IOEXCEPTION_LOG + ioe.message)
      Log.e(TAG, "Install of $source failed.")
    }
    return success
  }

  /**
   * Installs required files from APK's asset directory to the applications' internal storage.
   * File attributes override and executable are defined here
   *
   * @return Boolean success
   */
  private fun installClient(): Boolean {
    if (!installFile(fileNameClient, true, "")) {
      Log.e(TAG, INSTALL_FAILED + fileNameClient)

      return false
    }
    if (!installFile(fileNameClientCmd, true, "")) {
      Log.e(TAG, INSTALL_FAILED + fileNameClientCmd)

      return false
    }
    if (!installFile(fileNameCABundle, false, "")) {
      Log.e(TAG, INSTALL_FAILED + fileNameCABundle)

      return false
    }
    if (!installFile(fileNameClientConfig, false, "")) {
      Log.e(TAG, INSTALL_FAILED + fileNameClientConfig)

      return false
    }
    if (!installFile(fileNameAllProjectsList, false, "")) {
      Log.e(TAG, INSTALL_FAILED + fileNameAllProjectsList)

      return false
    }
    if (!installFile(fileNameNoMedia, false, ".$fileNameNoMedia")) {
      Log.e(TAG, INSTALL_FAILED + fileNameNoMedia)

      return false
    }
    return true
  }

  /**
   * Determines BOINC platform name corresponding to device's cpu architecture (ARM, x86).
   * Defaults to ARM
   *
   * @return ID of BOINC platform name string in resources
   */
  private val boincPlatform: String
    get() {
      val platformId: String
      val arch = System.getProperty("os.arch") ?: ""
      val normalizedArch = arch.toUpperCase(Locale.US)
      platformId = when {
        normalizedArch.containsAny("ARM64", "AARCH64") -> "ARM64"
        "X86_64" in normalizedArch -> "X86_64"
        "ARMV6" in normalizedArch -> "ARMV6"
        "ARM" in normalizedArch -> "ARM"
        "86" in normalizedArch -> "X86"
        else -> {
          Log.w(TAG, "could not map os.arch ($arch) to platform, default to arm.")
          ""
        }
      }

      Log.i(TAG, "BOINC platform: $platformId for os.arch: $arch")

      return platformId
    }

  /**
   * Determines assets directory (contains BOINC client binaries) corresponding to device's cpu architecture (ARM, x86)
   *
   * @return name of assets directory for given platform, not an absolute path.
   */
  private val assetsDirForCpuArchitecture: String
    get() {
      var archAssetsDirectory = ""
      when (boincPlatform) {
        "ARMV6" -> archAssetsDirectory = "armeabi/"
        "ARM" -> archAssetsDirectory = "armeabi-v7a/"
        "ARM64" -> archAssetsDirectory = "arm64-v8a/"
        "X86" -> archAssetsDirectory = "x86/"
        "X86_64" -> archAssetsDirectory = "x86_64/"
        else -> {
        }
      }
      return archAssetsDirectory
    }
}
