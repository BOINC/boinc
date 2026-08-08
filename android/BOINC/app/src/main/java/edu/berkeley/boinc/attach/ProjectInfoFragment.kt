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
import android.os.Build.VERSION
import android.os.Build.VERSION_CODES
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import androidx.core.os.bundleOf
import androidx.fragment.app.DialogFragment
import com.bumptech.glide.Glide
import edu.berkeley.boinc.R
import edu.berkeley.boinc.attach.glide.ScaleBitmapBy2
import edu.berkeley.boinc.databinding.AttachProjectInfoLayoutBinding
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.utils.Logging

class ProjectInfoFragment : DialogFragment() {
    private var _binding: AttachProjectInfoLayoutBinding? = null
    private val binding get() = _binding!!

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "ProjectInfoFragment onCreateView")

        _binding = AttachProjectInfoLayoutBinding.inflate(inflater, container, false)

        // get data
        val info: ProjectInfo? = if (VERSION.SDK_INT >= VERSION_CODES.TIRAMISU) {
            requireArguments().getParcelable("info", ProjectInfo::class.java)
        } else {
            @Suppress("DEPRECATION")
            requireArguments().getParcelable("info")
        }
        if (info == null) {
            Logging.logError(Logging.Category.GUI_VIEW, "ProjectInfoFragment info is null, return.")

            dismiss()
            return binding.root
        }

        Logging.logVerbose(Logging.Category.GUI_VIEW, "ProjectInfoFragment project: " + info.name)

        // set texts
        binding.projectName.text = info.name
        binding.projectDesc.text = info.summary
        binding.projectArea.text = "${info.generalArea}: ${info.specificArea}"
        binding.projectDesc.text = info.description
        binding.projectHome.text =
                resources.getString(R.string.attachproject_login_header_home) + " ${info.home}"

        // setup return button
        binding.continueButton.setOnClickListener {
            Logging.logVerbose(Logging.Category.USER_ACTION, "ProjectInfoFragment continue clicked")

            dismiss()
        }

        Logging.logVerbose(Logging.Category.GUI_VIEW, "ProjectInfoFragment image url: " + info.imageUrl)

        Glide.with(this).asBitmap().placeholder(R.drawable.ic_boinc).load(info.imageUrl)
                .transform(ScaleBitmapBy2()).into(binding.projectLogo)
        return binding.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
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
