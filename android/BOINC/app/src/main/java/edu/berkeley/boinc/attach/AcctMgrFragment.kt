/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2019 University of California
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
import android.app.Service
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.net.ConnectivityManager
import android.os.Bundle
import android.os.IBinder
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.widget.*
import android.widget.AdapterView.OnItemSelectedListener
import androidx.core.content.getSystemService
import androidx.fragment.app.DialogFragment
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.attach.ProjectAttachService.LocalBinder
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.rpc.AccountManager
import edu.berkeley.boinc.utils.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class AcctMgrFragment : DialogFragment() {
    // services
    private var monitor: IMonitor? = null
    private var mIsBound = false
    private var attachService: ProjectAttachService? = null
    private var asIsBound = false
    private lateinit var urlSpinner: Spinner
    private lateinit var urlInput: EditText
    private lateinit var nameInput: EditText
    private lateinit var pwdInput: EditText
    private lateinit var warning: TextView
    private lateinit var ongoingWrapper: LinearLayout
    private lateinit var continueB: Button
    private var returnToMainActivity = false

    private val mMonitorConnection: ServiceConnection = object : ServiceConnection {
        private fun fillAdapterData() {
            if (mIsBound) {
                val accountManagers = try {
                    monitor!!.accountManagers
                } catch (e: Exception) {
                    if (Logging.ERROR) Log.e(Logging.TAG, "AcctMgrFragment onCreateView() error: $e")
                    emptyList<AccountManager>()
                }
                val adapterData = accountManagers.map { AccountManagerSpinner(it.name, it.url) }
                val adapter = ArrayAdapter(requireActivity(), android.R.layout.simple_spinner_item, adapterData)
                adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
                urlSpinner.adapter = adapter
            }
        }

        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            monitor = IMonitor.Stub.asInterface(service)
            mIsBound = true
            fillAdapterData()
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            monitor = null
            mIsBound = false
            Log.e(Logging.TAG, "BOINCActivity onServiceDisconnected")
        }
    }
    private val mASConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = (service as LocalBinder).service
            asIsBound = true
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            attachService = null
            asIsBound = false
        }
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "AcctMgrFragment onCreateView")
        }
        val v = inflater.inflate(R.layout.attach_project_acctmgr_dialog, container, false)
        urlSpinner = v.findViewById(R.id.url_spinner)
        urlInput = v.findViewById(R.id.url_input)
        nameInput = v.findViewById(R.id.name_input)
        pwdInput = v.findViewById(R.id.pwd_input)
        warning = v.findViewById(R.id.warning)
        ongoingWrapper = v.findViewById(R.id.ongoing_wrapper)
        continueB = v.findViewById(R.id.continue_button)

        // change url text field on url spinner change
        urlSpinner.onItemSelectedListener = object : OnItemSelectedListener {
            override fun onItemSelected(adapterView: AdapterView<*>?, view: View, i: Int, l: Long) {
                val accountManagerSpinner = urlSpinner.selectedItem as AccountManagerSpinner
                urlInput.setText(accountManagerSpinner.url)
            }

            override fun onNothingSelected(adapterView: AdapterView<*>?) {}
        }
        continueB.setOnClickListener {
            if (Logging.DEBUG) Log.d(Logging.TAG, "AcctMgrFragment continue clicked")
            if (!checkDeviceOnline()) return@setOnClickListener
            if (asIsBound) {
                // get user input
                val url = urlInput.text.toString()
                val name = nameInput.text.toString()
                val pwd = pwdInput.text.toString()

                // verify input
                val res = verifyInput(url, name, pwd)
                if (res != 0) {
                    warning.setText(res)
                    warning.visibility = View.VISIBLE
                    return@setOnClickListener
                }

                // adapt layout
                continueB.visibility = View.GONE
                warning.visibility = View.GONE
                ongoingWrapper.visibility = View.VISIBLE
                lifecycleScope.launch {
                    attachProject(url, name, pwd)
                }
            } else if (Logging.DEBUG) Log.d(Logging.TAG, "AcctMgrFragment service not bound, do nothing...")
        }
        doBindService()
        return v
    }

    override fun onDestroyView() {
        doUnbindService()
        super.onDestroyView()
    }

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        val dialog = super.onCreateDialog(savedInstanceState)

        // request a window without the title
        dialog.window!!.requestFeature(Window.FEATURE_NO_TITLE)
        return dialog
    }

    fun setReturnToMainActivity() {
        returnToMainActivity = true
    }

    private fun verifyInput(url: String, name: String, pwd: String) = when {
        url.isEmpty() -> R.string.attachproject_error_no_url
        name.isEmpty() -> R.string.attachproject_error_no_name
        pwd.isEmpty() -> R.string.attachproject_error_no_pwd
        else -> 0
    }

    // check whether device is online before starting connection attempt
    // as needed for AttachProjectLoginActivity (retrieval of ProjectConfig)
    // note: available internet does not imply connection to project server
    // is possible!
    private fun checkDeviceOnline(): Boolean {
        val connectivityManager = requireActivity().getSystemService<ConnectivityManager>()!!
        val online = connectivityManager.isOnline
        if (!online) {
            val toast = Toast.makeText(activity, R.string.attachproject_list_no_internet, Toast.LENGTH_SHORT)
            toast.show()
            if (Logging.DEBUG) Log.d(Logging.TAG, "AttachProjectListActivity not online, stop!")
        }
        return online
    }

    private fun doBindService() {
        // start service to allow setForeground later on...
        requireActivity().startService(Intent(activity, Monitor::class.java))
        // Establish a connection with the service, onServiceConnected gets called when
        requireActivity().bindService(Intent(activity, Monitor::class.java), mMonitorConnection, Service.BIND_AUTO_CREATE)

        // bind to attach service
        requireActivity().bindService(Intent(activity, ProjectAttachService::class.java), mASConnection, Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (mIsBound) {
            // Detach existing connection.
            requireActivity().unbindService(mMonitorConnection)
            mIsBound = false
        }
        if (asIsBound) {
            // Detach existing connection.
            requireActivity().unbindService(mASConnection)
            asIsBound = false
        }
    }

    private fun mapErrorNumToString(code: Int): String {
        if (Logging.DEBUG) Log.d(Logging.TAG, "mapErrorNumToString for error: $code")
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

    private suspend fun attachProject(url: String, name: String, pwd: String) = coroutineScope {
        val result = withContext(Dispatchers.Default) { attachService!!.attachAcctMgr(url, name, pwd) }

        if (result.isOK) {
            dismiss()
            if (returnToMainActivity) {
                if (Logging.DEBUG) Log.d(Logging.TAG, "attachProject() finished, start main activity")
                val intent = Intent(activity, BOINCActivity::class.java).apply {
                    // add flags to return to main activity and clearing all others and clear the back stack
                    addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                    addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    // make activity display projects fragment
                    putExtra("targetFragment", R.string.tab_projects)
                }
                startActivity(intent)
            }
        } else {
            ongoingWrapper.visibility = View.GONE
            continueB.visibility = View.VISIBLE
            warning.visibility = View.VISIBLE
            if (!result.description.isNullOrEmpty()) {
                warning.text = result.description
            } else {
                warning.text = mapErrorNumToString(result.code)
            }
        }
    }
}
