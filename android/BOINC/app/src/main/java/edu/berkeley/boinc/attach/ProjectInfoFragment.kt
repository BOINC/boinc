/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.widget.Button
import android.widget.TextView
import androidx.core.os.bundleOf
import androidx.fragment.app.DialogFragment
import com.bumptech.glide.Glide
import edu.berkeley.boinc.R
import edu.berkeley.boinc.attach.glide.ScaleBitmapBy2
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.utils.Logging

class ProjectInfoFragment : DialogFragment() {
    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment onCreateView")
        }
        val v = inflater.inflate(R.layout.attach_project_info_layout, container, false)

        // get data
        val info: ProjectInfo? = requireArguments().getParcelable("info")
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
        Glide.with(this).asBitmap().placeholder(R.drawable.ic_boinc).load(info.imageUrl)
                .transform(ScaleBitmapBy2()).into(v.findViewById(R.id.project_logo))
        return v
    }

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        return super.onCreateDialog(savedInstanceState).apply {
            // request a window without the title
            window?.requestFeature(Window.FEATURE_NO_TITLE)
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
