/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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

import android.app.Activity
import android.app.Dialog
import android.content.Intent
import android.net.ConnectivityManager
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.View.OnFocusChangeListener
import android.view.ViewGroup
import android.view.Window
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.core.content.ContextCompat
import androidx.fragment.app.DialogFragment
import edu.berkeley.boinc.R
import edu.berkeley.boinc.utils.Logging.Category.GUI_ACTIVITY
import edu.berkeley.boinc.utils.Logging.Category.USER_ACTION
import edu.berkeley.boinc.utils.Logging.logDebug
import edu.berkeley.boinc.utils.Logging.logVerbose
import edu.berkeley.boinc.utils.isOnline

class ManualUrlInputFragment : DialogFragment() {
    private var urlInputET: EditText? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val v = inflater.inflate(R.layout.attach_project_manual_url_input_dialog, container, false)

        urlInputET = v.findViewById(R.id.url_input)
        urlInputET!!.onFocusChangeListener = OnFocusChangeListener { _: View?, hasFocus: Boolean ->
            if (hasFocus && urlInputET!!.text.isEmpty()) {
                urlInputET!!.setText("https://")
            }
        }

        val continueButton = v.findViewById<Button>(R.id.continue_button)
        continueButton.setOnClickListener { _: View? ->
            logVerbose(
                USER_ACTION,
                "ManualUrlInputFragment: continue clicked"
            )
            if (!checkDeviceOnline()) {
                return@setOnClickListener
            }

            //startActivity
            val intent = Intent(activity, BatchConflictListActivity::class.java)
            intent.putExtra("conflicts", false)
            intent.putExtra("manualUrl", urlInputET!!.text.toString())
            startActivity(intent)
            dismiss()
        }

        return v
    }

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        val dialog = super.onCreateDialog(savedInstanceState)

        // request a window without the title
        dialog.window!!.requestFeature(Window.FEATURE_NO_TITLE)
        return dialog
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not imply connection to project server
    // is possible!
    private fun checkDeviceOnline(): Boolean {
        val activity: Activity = requireActivity()
        val connectivityManager = checkNotNull(
            ContextCompat.getSystemService(
                activity,
                ConnectivityManager::class.java
            )
        )
        val online = connectivityManager.isOnline
        if (!online) {
            val toast = Toast.makeText(
                getActivity(),
                R.string.attachproject_list_no_internet,
                Toast.LENGTH_SHORT
            )
            toast.show()

            logDebug(GUI_ACTIVITY, "ManualUrlInputFragment not online, stop!")
        }
        return online
    }
}
