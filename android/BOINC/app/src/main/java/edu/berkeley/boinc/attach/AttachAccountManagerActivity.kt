/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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

import android.app.Service
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.net.ConnectivityManager
import android.os.Bundle
import android.os.IBinder
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.getSystemService
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.client.MonitorAsync
import edu.berkeley.boinc.databinding.ActivityAttachProjectAcctMgrBinding
import edu.berkeley.boinc.rpc.AccountManager
import edu.berkeley.boinc.utils.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class AttachAccountManagerActivity  : AppCompatActivity() {
    // services
    private var monitor: MonitorAsync? = null
    private var mIsBound = false
    private var attachService: ProjectAttachService? = null
    private var asIsBound = false

    private var _binding: ActivityAttachProjectAcctMgrBinding? = null
    private val binding get() = _binding!!

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        _binding = ActivityAttachProjectAcctMgrBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // change url text field on url spinner change
        binding.urlSpinner.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onItemSelected(adapterView: AdapterView<*>?, view: View, i: Int, l: Long) {
                val accountManagerSpinner = binding.urlSpinner.selectedItem as AccountManagerSpinner
                binding.urlInput.setText(accountManagerSpinner.url)
            }

            override fun onNothingSelected(adapterView: AdapterView<*>?) {}
        }

        binding.continueButton.setOnClickListener {
            Logging.logVerbose(Logging.Category.USER_ACTION, "AttachAccountManagerActivity continue clicked")

            if (!checkDeviceOnline()) return@setOnClickListener
            if (asIsBound) {
                // get user input
                val url = binding.urlInput.text.toString()
                val name = binding.nameInput.text.toString()
                val pwd = binding.pwdInput.text.toString()

                // verify input
                val res = verifyInput(url, name, pwd)
                if (res != 0) {
                    binding.warning.setText(res)
                    binding.warning.visibility = View.VISIBLE
                    return@setOnClickListener
                }

                // adapt layout
                binding.cancelButton.visibility = View.GONE
                binding.continueButton.visibility = View.GONE
                binding.warning.visibility = View.GONE
                binding.ongoingWrapper.visibility = View.VISIBLE
                lifecycleScope.launch {
                    attachProject(url, name, pwd)
                }
            } else {
                Logging.logDebug(Logging.Category.GUI_ACTIVITY, "AttachAccountManagerActivity service not bound, do nothing...")
            }
        }

        doBindService()

        binding.cancelButton.setOnClickListener {
            finish()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        doUnbindService()
        _binding = null
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not imply connection to project server
    // is possible!
    private fun checkDeviceOnline(): Boolean {
        val connectivityManager = getSystemService<ConnectivityManager>()!!
        val online = connectivityManager.isOnline
        if (!online) {
            val toast = Toast.makeText(this, R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT)
            toast.show()

            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "AttachAccountManagerActivity not online, stop!")
        }
        return online
    }

    private fun verifyInput(url: String, name: String, pwd: String) = when {
        url.isEmpty() -> R.string.attachproject_error_no_url
        name.isEmpty() -> R.string.attachproject_error_no_name
        pwd.isEmpty() -> R.string.attachproject_error_no_pwd
        else -> 0
    }

    private suspend fun attachProject(url: String, name: String, pwd: String) = coroutineScope {
        val result = withContext(Dispatchers.Default) { attachService!!.attachAcctMgr(url, name, pwd) }

        if (result.isOK) {
            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "AttachAccountManagerActivity attachProject() finished, start main activity")

            val intent = Intent(this@AttachAccountManagerActivity, BOINCActivity::class.java).apply {
                // add flags to return to main activity and clearing all others and clear the back stack
                addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                // make activity display projects fragment
                putExtra("targetFragment", R.string.tab_projects)
            }
            startActivity(intent)
        } else {
            binding.ongoingWrapper.visibility = View.GONE
            binding.continueButton.visibility = View.VISIBLE
            binding.cancelButton.visibility = View.VISIBLE
            binding.warning.visibility = View.VISIBLE
            if (!result.description.isNullOrEmpty()) {
                binding.warning.text = result.description
            } else {
                binding.warning.text = mapErrorNumToString(result.code)
            }
        }
    }

    private fun mapErrorNumToString(code: Int): String {
        Logging.logDebug(Logging.Category.GUI_ACTIVITY, "mapErrorNumToString for error: $code")

        val stringResource = when (code) {
            ERR_DB_NOT_FOUND -> R.string.attachproject_error_wrong_name
            ERR_GETHOSTBYNAME -> R.string.attachproject_error_no_internet
            ERR_NONUNIQUE_EMAIL, ERR_DB_NOT_UNIQUE -> R.string.attachproject_error_email_in_use
            ERR_PROJECT_DOWN -> R.string.attachproject_error_project_down
            ERR_BAD_EMAIL_ADDR -> R.string.attachproject_error_email_bad_syntax
            ERR_BAD_PASSWD -> R.string.attachproject_error_bad_pwd
            ERR_BAD_USER_NAME -> R.string.attachproject_error_bad_username
            ERR_ACCT_CREATION_DISABLED -> R.string.attachproject_error_creation_disabled
            ERR_INVALID_URL -> R.string.attachproject_error_invalid_url
            else -> R.string.attachproject_error_unknown
        }
        return getString(stringResource)
    }

    private fun doBindService() {
        // start service to allow setForeground later on...
        startService(Intent(this@AttachAccountManagerActivity, Monitor::class.java))
        // Establish a connection with the service, onServiceConnected gets called when
        bindService(Intent(this@AttachAccountManagerActivity, Monitor::class.java), mMonitorConnection, Service.BIND_AUTO_CREATE)
        // bind to attach service
        bindService(Intent(this@AttachAccountManagerActivity, ProjectAttachService::class.java), mASConnection, Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (mIsBound) {
            // Detach existing connection.
            unbindService(mMonitorConnection)
            mIsBound = false
        }
        if (asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection)
            asIsBound = false
        }
    }

    private val mMonitorConnection: ServiceConnection = object : ServiceConnection {
        private fun fillAdapterData() {
            if (mIsBound) {
                val accountManagers = try {
                    monitor!!.getAccountManagersAsync().await()
                } catch (e: Exception) {
                    Logging.logError(Logging.Category.MONITOR, "AttachAccountManagerActivity onCreate() mMonitorConnection error: $e")

                    emptyList<AccountManager>()
                }
                val adapterData = accountManagers.map { AccountManagerSpinner(it.name, it.url) }
                val adapter = ArrayAdapter(this@AttachAccountManagerActivity, android.R.layout.simple_spinner_item, adapterData)
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
                binding.urlSpinner.adapter = adapter
            }
        }

        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = MonitorAsync(IMonitor.Stub.asInterface(service))
            mIsBound = true
            fillAdapterData()
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            monitor = null
            mIsBound = false

            Logging.logError(Logging.Category.GUI_ACTIVITY, "AttachAccountManagerActivity onServiceDisconnected")
        }
    }
    private val mASConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = (service as ProjectAttachService.LocalBinder).service
            asIsBound = true
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            attachService = null
            asIsBound = false
        }
    }
}
