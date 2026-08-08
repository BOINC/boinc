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
package edu.berkeley.boinc.ui.eventlog

import android.content.ClipData
import android.content.ClipboardManager
import android.content.ComponentName
import android.content.Intent
import android.content.ServiceConnection
import android.os.Bundle
import android.os.IBinder
import android.view.Menu
import android.view.MenuItem
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.getSystemService
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.tabs.TabLayoutMediator
import edu.berkeley.boinc.R
import edu.berkeley.boinc.adapter.ClientLogRecyclerViewAdapter
import edu.berkeley.boinc.client.IMonitor
import edu.berkeley.boinc.client.Monitor
import edu.berkeley.boinc.databinding.ActivityEventLogBinding
import edu.berkeley.boinc.rpc.Message
import edu.berkeley.boinc.utils.Logging
import java.util.*

class EventLogActivity : AppCompatActivity() {
    private lateinit var binding: ActivityEventLogBinding

    private var monitor: IMonitor? = null
    private var mIsBound = false

    lateinit var clientLogList: RecyclerView
    lateinit var clientLogRecyclerViewAdapter: ClientLogRecyclerViewAdapter
    val clientLogData: MutableList<Message> = ArrayList()

    val guiLogData: MutableList<String> = ArrayList()

    private val mConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "EventLogActivity onServiceConnected")

            monitor = IMonitor.Stub.asInterface(service)
            mIsBound = true

            // initialize default fragment
            (supportFragmentManager.findFragmentByTag("f0") as EventLogClientFragment).init()
        }

        override fun onServiceDisconnected(className: ComponentName) {
            monitor = null
            mIsBound = false
        }
    }

    val monitorService: IMonitor
        get() {
            if (!mIsBound) {
                Logging.logWarning(Logging.Category.MONITOR, "Fragment trying to obtain service reference, but Monitor" +
                        " not bound in EventLogActivity")
            }
            return monitor!!
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityEventLogBinding.inflate(layoutInflater)
        setContentView(binding.root)
        val eventLogPagerAdapter = EventLogPagerAdapter(this)
        binding.viewPager.adapter = eventLogPagerAdapter
        TabLayoutMediator(binding.tabs, binding.viewPager) { tab, position ->
            tab.text = if (position == 0)
                getString(R.string.eventlog_client_header)
            else
                getString(R.string.eventlog_gui_header)
        }.attach()

        doBindService()
    }

    override fun onDestroy() {
        doUnbindService()
        super.onDestroy()
    }

    override fun onCreateOptionsMenu(menu: Menu?): Boolean {
        menuInflater.inflate(R.menu.eventlog_menu, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            R.id.refresh -> {
                updateCurrentFragment()
                return true
            }
            R.id.email_to -> {
                onEmailTo()
                return true
            }
            R.id.copy -> {
                onCopy()
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    private fun doBindService() {
        if (!mIsBound) {
            applicationContext.bindService(Intent(this, Monitor::class.java), mConnection, 0) //calling within Tab needs getApplicationContext() for bindService to work!
        }
    }

    private fun doUnbindService() {
        if (mIsBound) {
            applicationContext.unbindService(mConnection)
            mIsBound = false
        }
    }

    private fun updateCurrentFragment() {
        val currentFragment = supportFragmentManager.findFragmentByTag("f${binding.viewPager.currentItem}")
        if (currentFragment is EventLogClientFragment) {
            currentFragment.update()
        } else if (currentFragment is EventLogGuiFragment) {
            currentFragment.update()
        }
    }

    private fun onCopy() {
        try {
            val clipboard = getSystemService<ClipboardManager>()!!
            val clipData = ClipData.newPlainText("log", getLogDataAsString())
            clipboard.setPrimaryClip(clipData)
            Toast.makeText(applicationContext, R.string.eventlog_copy_toast, Toast.LENGTH_SHORT).show()
        } catch (e: Exception) {
            Logging.logError(Logging.Category.USER_ACTION, "onCopy failed")
        }
    }

    private fun onEmailTo() {
        try {
            val emailText = getLogDataAsString()
            val emailIntent = Intent(Intent.ACTION_SEND)

            // Put together the email intent
            emailIntent.type = "plain/text"
            emailIntent.putExtra(Intent.EXTRA_SUBJECT, getString(R.string.eventlog_email_subject))
            emailIntent.putExtra(Intent.EXTRA_TEXT, emailText)

            // Send it off to the Activity-Chooser
            startActivity(Intent.createChooser(emailIntent, "Send mail..."))
        } catch (e: Exception) {
            Logging.logError(Logging.Category.USER_ACTION, "onEmailTo failed")
        }
    }

    // returns the content of the log as string
    // clientLog = true: client log
    // clientlog = false: gui log
    private fun getLogDataAsString(): String {
        val text = StringBuilder()
        val type = binding.viewPager.currentItem
        when {
            type == 0 -> {
                text.append(getString(R.string.eventlog_client_header)).append("\n\n")
                for (index in 0 until clientLogList.adapter?.itemCount!!) {
                    text.append(clientLogRecyclerViewAdapter.getDateTimeString(index))
                    text.append("|")
                    text.append(clientLogRecyclerViewAdapter.getProject(index))
                    text.append("|")
                    text.append(clientLogRecyclerViewAdapter.getMessage(index))
                    text.append("\n")
                }
            }
            type == 1 -> {
                text.append(getString(R.string.eventlog_gui_header)).append("\n\n")
                for (line in guiLogData) {
                    text.append(line)
                    text.append("\n")
                }
            }
            else -> {
                Logging.logError(Logging.Category.GUI_ACTIVITY, "EventLogActivity could not determine which log active.")
            }
        }
        return text.toString()
    }
}
