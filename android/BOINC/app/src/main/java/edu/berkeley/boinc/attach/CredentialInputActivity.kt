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

import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.Bundle
import android.os.IBinder
import android.text.InputType
import android.text.method.PasswordTransformationMethod
import android.view.View
import android.widget.CheckBox
import androidx.appcompat.app.AppCompatActivity
import edu.berkeley.boinc.attach.BatchProcessingActivity
import edu.berkeley.boinc.attach.ProjectAttachService
import edu.berkeley.boinc.attach.ProjectAttachService.LocalBinder
import edu.berkeley.boinc.databinding.AttachProjectCredentialInputLayoutBinding
import edu.berkeley.boinc.utils.Logging.Category.GUI_ACTIVITY
import edu.berkeley.boinc.utils.Logging.Category.USER_ACTION
import edu.berkeley.boinc.utils.Logging.logError
import edu.berkeley.boinc.utils.Logging.logVerbose
import edu.berkeley.boinc.utils.Logging.logWarning

class CredentialInputActivity : AppCompatActivity() {
    private var binding: AttachProjectCredentialInputLayoutBinding? = null

    private var attachService: ProjectAttachService? = null
    private var asIsBound = false

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        logVerbose(GUI_ACTIVITY, "CredentialInputActivity onCreate")

        doBindService()
        binding = AttachProjectCredentialInputLayoutBinding.inflate(
            layoutInflater
        )
        setContentView(binding!!.root)

        binding!!.showPwdCb.setOnClickListener { view: View ->
            if ((view as CheckBox).isChecked) {
                binding!!.pwdInput.inputType =
                    InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
            } else {
                binding!!.pwdInput.inputType =
                    InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
                binding!!.pwdInput.transformationMethod = PasswordTransformationMethod.getInstance()
            }
        }
    }

    override fun onDestroy() {
        doUnbindService()
        super.onDestroy()
    }

    // triggered by continue button
    fun continueClicked(@Suppress("UNUSED_PARAMETER") v: View?) {
        logVerbose(USER_ACTION, "CredentialInputActivity.continueClicked.")

        // set credentials in service
        if (asIsBound) {
            // verify input and set credentials if valid.
            val email = binding!!.emailInput.text.toString()
            val name = binding!!.nameInput.text.toString()
            val password = binding!!.pwdInput.text.toString()
            if (attachService!!.verifyInput(email, name, password)) {
                attachService!!.setCredentials(email, name, password)
            } else {
                logWarning(
                    USER_ACTION,
                    "CredentialInputActivity.continueClicked: empty credentials found"
                )

                return
            }
        } else {
            logError(GUI_ACTIVITY, "CredentialInputActivity.continueClicked: service not bound.")

            return
        }

        logVerbose(
            USER_ACTION,
            "CredentialInputActivity.continueClicked: starting BatchProcessingActivity..."
        )

        startActivity(Intent(this, BatchProcessingActivity::class.java))
    }

    // triggered by individual button
    fun individualClicked(@Suppress("UNUSED_PARAMETER") v: View?) {
        logVerbose(USER_ACTION, "CredentialInputActivity.individualClicked.")

        // set credentials in service, in case user typed before deciding btwn batch and individual attach
        if (asIsBound) {
            val email = binding!!.emailInput.text.toString()
            val name = binding!!.nameInput.text.toString()
            val password = binding!!.pwdInput.text.toString()
            attachService!!.setCredentials(email, name, password)
        }

        val intent = Intent(this, BatchConflictListActivity::class.java)
        intent.putExtra("conflicts", false)
        startActivity(Intent(this, BatchConflictListActivity::class.java))
    }

    private val mASConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = (service as LocalBinder).service
            asIsBound = true

            val values = attachService!!.userDefaultValues
            binding!!.emailInput.setText(values[0])
            binding!!.nameInput.setText(values[1])
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            attachService = null
            asIsBound = false
        }
    }

    private fun doBindService() {
        // bind to attach service
        bindService(Intent(this, ProjectAttachService::class.java), mASConnection, BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection)
            asIsBound = false
        }
    }
}
