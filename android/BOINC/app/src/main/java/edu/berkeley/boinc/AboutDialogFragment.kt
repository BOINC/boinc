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
package edu.berkeley.boinc

import android.content.pm.PackageManager
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.DialogFragment
import edu.berkeley.boinc.databinding.DialogAboutBinding
import edu.berkeley.boinc.utils.Logging

class AboutDialogFragment : DialogFragment() {
    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        val binding = DialogAboutBinding.inflate(inflater)
        try {
            binding.version.text = requireContext().getString(R.string.about_version,
                    requireContext().packageManager?.getPackageInfo(requireContext().packageName, 0)?.versionName)
        } catch (e: PackageManager.NameNotFoundException) {
            if (Logging.WARNING) {
                Log.w(Logging.TAG, "version name not found.")
            }
        }
        binding.returnButton.setOnClickListener { dismiss() }
        return binding.root
    }
}
