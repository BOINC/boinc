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
import android.os.Build
import android.os.Bundle
import android.os.IBinder
import android.view.View
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.viewpager2.adapter.FragmentStateAdapter
import androidx.viewpager2.widget.ViewPager2
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.attach.HintFragment.Companion.HINT_TYPE_CONTRIBUTION
import edu.berkeley.boinc.attach.HintFragment.Companion.HINT_TYPE_PLATFORMS
import edu.berkeley.boinc.attach.HintFragment.Companion.HINT_TYPE_PROJECTWEBSITE
import edu.berkeley.boinc.attach.HintFragment.Companion.newInstance
import edu.berkeley.boinc.attach.ProjectAttachService.Companion.RESULT_READY
import edu.berkeley.boinc.attach.ProjectAttachService.Companion.RESULT_SUCCESS
import edu.berkeley.boinc.attach.ProjectAttachService.LocalBinder
import edu.berkeley.boinc.databinding.AttachProjectBatchProcessingLayoutBinding
import edu.berkeley.boinc.utils.Logging
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class BatchProcessingActivity : AppCompatActivity() {
    private lateinit var binding: AttachProjectBatchProcessingLayoutBinding

    private var attachService: ProjectAttachService? = null
    private var asIsBound = false

    public override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BatchProcessingActivity onCreate")

        // setup layout
        binding = AttachProjectBatchProcessingLayoutBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Instantiate a PagerAdapter.
        // provides content to pager
        val mPagerAdapter = HintPagerAdapter(supportFragmentManager, lifecycle)
        binding.hintContainer.adapter = mPagerAdapter
        binding.hintContainer.registerOnPageChangeCallback(object : ViewPager2.OnPageChangeCallback() {
            override fun onPageSelected(arg0: Int) {
                adaptHintHeader()
            }
        })
        adaptHintHeader()
        doBindService()

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                if (binding.hintContainer.currentItem == 0) {
                    // If the user is currently looking at the first step, allow the system to handle the
                    // Back button. This calls finish() on this activity and pops the back stack.
                    finish()
                } else {
                    // Otherwise, select the previous step.
                    binding.hintContainer.currentItem--
                }
            }
        })
    }

    override fun onDestroy() {
        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "BatchProcessingActivity onDestroy")

        super.onDestroy()
        doUnbindService()
    }

    // triggered by continue button
    fun continueClicked(@Suppress("UNUSED_PARAMETER") v: View) {
        val conflicts = attachService!!.anyUnresolvedConflicts()

        Logging.logDebug(Logging.Category.USER_ACTION, "BatchProcessingActivity.continueClicked: conflicts? $conflicts")

        if (conflicts) {
            // conflicts occurred, bring up resolution screen
            Logging.logDebug(Logging.Category.GUI_ACTIVITY, "attachProject(): conflicts exists, open resolution activity...")

            startActivity(Intent(this@BatchProcessingActivity,
                    BatchConflictListActivity::class.java).apply {
                putExtra("conflicts", true)
            })
        } else {
            // everything successful, go back to projects screen and clear history
            startActivity(Intent(this, BOINCActivity::class.java).apply {
                // add flags to return to main activity and clearing all others and clear the back stack
                addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                putExtra("targetFragment", R.string.tab_projects) // make activity display projects fragment
            })
        }
    }

    // triggered by share button
    fun shareClicked(@Suppress("UNUSED_PARAMETER") v: View) {
        Logging.logVerbose(Logging.Category.USER_ACTION, "BatchProcessingActivity.shareClicked.")

        val intent = Intent(Intent.ACTION_SEND).apply {
            type = "text/plain"

            val flag = if (Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
                @Suppress("DEPRECATION")
                Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET
            } else {
                Intent.FLAG_ACTIVITY_NEW_DOCUMENT
            }
            addFlags(flag)

            // Add data to the intent, the receiving app will decide what to do with it.
            putExtra(Intent.EXTRA_SUBJECT, getString(R.string.social_invite_content_title))
        }
        if (Build.MANUFACTURER.equals("Amazon", ignoreCase = true)) {
            intent.putExtra(Intent.EXTRA_TEXT, String.format(getString(R.string.social_invite_content_body),
                    Build.MANUFACTURER, getString(R.string.social_invite_content_url_amazon)))
        } else {
            intent.putExtra(Intent.EXTRA_TEXT, String.format(getString(R.string.social_invite_content_body),
                    Build.MANUFACTURER, getString(R.string.social_invite_content_url_google)))
        }
        startActivity(Intent.createChooser(intent, getString(R.string.social_invite_intent_title)))
    }

    // adapts header text and icons when hint selection changes
    private fun adaptHintHeader() {
        val position = binding.hintContainer.currentItem

        Logging.logVerbose(Logging.Category.GUI_VIEW, "BatchProcessingActivity.adaptHintHeader position: $position")

        val hintText = getString(R.string.attachproject_hints_header) + " ${position + 1}/$NUM_HINTS"
        binding.hintHeaderText.text = hintText
        var leftVisibility = View.VISIBLE
        var rightVisibility = View.VISIBLE
        if (position == 0) {
            // first element reached
            leftVisibility = View.GONE
        } else if (position == NUM_HINTS - 1) {
            // last element reached
            rightVisibility = View.GONE
        }
        binding.hintHeaderImageLeft.visibility = leftVisibility
        binding.hintHeaderImageRight.visibility = rightVisibility
    }

    // previous image in hint header clicked
    fun previousHintClicked(@Suppress("UNUSED_PARAMETER") view: View) {
        Logging.logVerbose(Logging.Category.USER_ACTION, "BatchProcessingActivity.previousHintClicked.")

        binding.hintContainer.currentItem--
    }

    // previous image in hint header clicked
    fun nextHintClicked(@Suppress("UNUSED_PARAMETER") view: View) {
        Logging.logVerbose(Logging.Category.USER_ACTION, "BatchProcessingActivity.nextHintClicked.")

        binding.hintContainer.currentItem++
    }

    private val mASConnection: ServiceConnection = object : ServiceConnection {
        override fun onServiceConnected(className: ComponentName, service: IBinder) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = (service as LocalBinder).service
            asIsBound = true

            // start attaching projects
            lifecycleScope.launch {
                attachProject()
            }
        }

        override fun onServiceDisconnected(className: ComponentName) {
            // This should not happen
            attachService = null
            asIsBound = false
        }
    }

    private fun doBindService() {
        // bind to attach service
        bindService(Intent(this, ProjectAttachService::class.java), mASConnection,
                Service.BIND_AUTO_CREATE)
    }

    private fun doUnbindService() {
        if (asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection)
            asIsBound = false
        }
    }

    private suspend fun attachProject() {
        Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): ${attachService!!.numberOfSelectedProjects}" +
                " projects to attach....")

        // shown while project configs are loaded
        binding.attachStatusText.text = getString(R.string.attachproject_login_loading)

        withContext(Dispatchers.Default) {
            // wait until service is ready
            while (!attachService!!.projectConfigRetrievalFinished) {
                Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): project config retrieval has" +
                        " not finished yet, wait...")

                delay(1000)
            }

            Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): project config retrieval finished," +
                    " continue with attach.")

            // attach projects, one at a time
            attachService!!.selectedProjects
                    // skip already tried projects in batch processing
                    .filter { it.result == RESULT_READY }
                    .onEach {
                        Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): trying: ${it.info?.name}")

                        binding.attachStatusText.text = getString(R.string.attachproject_working_attaching,
                                it.info?.name)
                    }
                    .map { it.lookupAndAttach(false) }
                    .filter { it != RESULT_SUCCESS }
                    .forEach { Logging.logError(Logging.Category.PROJECTS, "attachProject() attach returned conflict: $it") }

            Logging.logDebug(Logging.Category.PROJECTS, "attachProject(): finished.")
        }

        binding.attachStatusOngoingWrapper.visibility = View.GONE
        binding.continueButton.visibility = View.VISIBLE
        binding.shareButton.visibility = View.VISIBLE
    }

    private inner class HintPagerAdapter constructor(fm: FragmentManager, lc: Lifecycle)
        : FragmentStateAdapter(fm, lc) {
        override fun createFragment(position: Int): Fragment {
            return when (position) {
                0 -> newInstance(HINT_TYPE_CONTRIBUTION)
                1 -> newInstance(HINT_TYPE_PROJECTWEBSITE)
                else -> newInstance(HINT_TYPE_PLATFORMS)
            }
        }
        override fun getItemCount(): Int = NUM_HINTS
    }

    companion object {
        private const val NUM_HINTS = 3 // number of available hint screens
    }
}
