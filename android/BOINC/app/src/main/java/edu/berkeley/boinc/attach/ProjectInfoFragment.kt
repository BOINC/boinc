/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */
package edu.berkeley.boinc.attach

import android.app.Dialog
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.widget.*
import androidx.core.os.bundleOf
import androidx.fragment.app.DialogFragment
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.utils.Logging
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.net.URL

class ProjectInfoFragment : DialogFragment() {
    private lateinit var logoWrapper: LinearLayout
    private lateinit var logoPb: ProgressBar
    private lateinit var logoIv: ImageView

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment onCreateView")
        }
        val v = inflater.inflate(R.layout.attach_project_info_layout, container, false)

        // get data
        val info: ProjectInfo? = arguments!!.getParcelable("info")
        if (info == null) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectInfoFragment info is null, return.")
            }
            dismiss()
            return v
        }
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment project: " + info.name)
        }

        // set texts
        v.findViewById<TextView>(R.id.project_name).text = info.name
        v.findViewById<TextView>(R.id.project_summary).text = info.summary
        v.findViewById<TextView>(R.id.project_area).text = "${info.generalArea}: ${info.specificArea}"
        v.findViewById<TextView>(R.id.project_desc).text = info.description
        v.findViewById<TextView>(R.id.project_home).text =
                resources.getString(R.string.attachproject_login_header_home) + " ${info.home}"

        // find view elements for later use in image download
        logoWrapper = v.findViewById(R.id.project_logo_wrapper)
        logoPb = v.findViewById(R.id.project_logo_loading_pb)
        logoIv = v.findViewById(R.id.project_logo)

        // setup return button
        v.findViewById<Button>(R.id.continue_button).setOnClickListener {
            if (Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectInfoFragment continue clicked")
            }
            dismiss()
        }
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment image url: " + info.imageUrl)
        }
        lifecycleScope.launch {
            downloadLogo(info.imageUrl)
        }
        return v
    }

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        return super.onCreateDialog(savedInstanceState).apply {
            // request a window without the title
            window?.requestFeature(Window.FEATURE_NO_TITLE)
        }
    }

    private suspend fun downloadLogo(url: String?) {
        val logo = withContext(Dispatchers.IO) {
            if (url.isNullOrEmpty()) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync url is empty, return.")
                }
                return@withContext null
            }
            if (Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync for url: $url")
            }
            var logo: Bitmap
            try {
                val logoStream = URL(url).openStream()
                logo = BitmapFactory.decodeStream(logoStream)
                // scale
                logo = Bitmap.createScaledBitmap(logo, logo.width * 2, logo.height * 2, false)
            } catch (e: Exception) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync image download failed")
                }
                return@withContext null
            }
            return@withContext logo
        }

        if (logo == null) {
            // failed.
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync failed.")
            }
            logoWrapper.visibility = View.GONE
        } else {
            // success.
            if (Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync successful.")
            }
            logoPb.visibility = View.GONE
            logoIv.visibility = View.VISIBLE
            logoIv.setImageBitmap(logo)
        }
    }

    companion object {
        @JvmStatic
        fun newInstance(info: ProjectInfo?): ProjectInfoFragment {
            return ProjectInfoFragment().apply {
                arguments = bundleOf("info" to info)
            }
        }
    }
}
