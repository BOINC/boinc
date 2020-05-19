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

import android.app.Activity
import android.app.Dialog
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.graphics.Bitmap
import android.graphics.Point
import android.net.Uri
import android.os.Bundle
import android.os.RemoteException
import android.text.SpannableString
import android.text.style.UnderlineSpan
import android.util.Log
import android.view.*
import android.widget.*
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.rpc.ImageWrapper
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.utils.Logging
import kotlinx.coroutines.*
import java.util.*

class ProjectDetailsFragment : Fragment() {
    private var url: String? = null

    // might be null for projects added via manual URL attach
    private var projectInfo: ProjectInfo? = null

    private var project: Project? = null
    private var slideshowImages: List<ImageWrapper> = ArrayList()
    private lateinit var li: LayoutInflater
    private lateinit var root: View
    private lateinit var slideshowWrapper: HorizontalScrollView
    private lateinit var slideshowHook: LinearLayout
    private lateinit var slideshowLoading: ProgressBar

    // display dimensions
    private var width = 0
    private var height = 0
    private var retryLayout = true

    private val currentProjectData: Unit
        get() {
            try {
                project = BOINCActivity.monitor!!.projects.firstOrNull { it.masterURL == url }
                projectInfo = BOINCActivity.monitor!!.getProjectInfo(url)
            } catch (e: Exception) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectDetailsFragment getCurrentProjectData could not" +
                            " retrieve project list")
                }
            }
            if (project == null && Logging.WARNING) {
                Log.w(Logging.TAG,
                        "ProjectDetailsFragment getCurrentProjectData could not find project for URL: $url")
            }
            if (projectInfo == null && Logging.WARNING) {
                Log.d(Logging.TAG,
                        "ProjectDetailsFragment getCurrentProjectData could not find project" +
                                " attach list for URL: $url")
            }
        }

    // BroadcastReceiver event is used to update the UI with updated information from
    // the client.  This is generally called once a second.
    //
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            currentProjectData
            if (retryLayout) {
                populateLayout()
            } else {
                updateChangingItems(root)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        // get data
        url = requireArguments().getString("url")
        currentProjectData
        setHasOptionsMenu(true) // enables fragment specific menu
        super.onCreate(savedInstanceState)
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        if (Logging.VERBOSE) {
            Log.v(Logging.TAG, "ProjectDetailsFragment onCreateView")
        }
        // Inflate the layout for this fragment
        li = inflater
        val layout = inflater.inflate(R.layout.project_details_layout, container, false)
        root = layout
        return layout
    }

    override fun onAttach(context: Context) {
        if (context is Activity) {
            val size = Point()
            context.windowManager.defaultDisplay.getSize(size)
            width = size.x
            height = size.y
        }
        super.onAttach(context)
    }

    override fun onPause() {
        activity?.unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        activity?.registerReceiver(mClientStatusChangeRec, ifcsc)
    }

    override fun onCreateOptionsMenu(menu: Menu, inflater: MenuInflater) {
        // appends the project specific menu to the main menu.
        inflater.inflate(R.menu.project_details_menu, menu)
        super.onCreateOptionsMenu(menu, inflater)
    }

    override fun onPrepareOptionsMenu(menu: Menu) {
        super.onPrepareOptionsMenu(menu)
        if (project == null) {
            return
        }

        // no new tasks, adapt based on status
        val nnt = menu.findItem(R.id.projects_control_nonewtasks)
        if (project!!.doNotRequestMoreWork) {
            nnt.setTitle(R.string.projects_control_allownewtasks)
        } else {
            nnt.setTitle(R.string.projects_control_nonewtasks)
        }

        // project suspension, adapt based on status
        val suspend = menu.findItem(R.id.projects_control_suspend)
        if (project!!.suspendedViaGUI) {
            suspend.setTitle(R.string.projects_control_resume)
        } else {
            suspend.setTitle(R.string.projects_control_suspend)
        }

        // detach, only show when project not managed
        val remove = menu.findItem(R.id.projects_control_remove)
        if (project!!.attachedViaAcctMgr) {
            remove.isVisible = false
        }
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        lifecycleScope.launch {
            when (item.itemId) {
                R.id.projects_control_update -> performProjectOperation(RpcClient.PROJECT_UPDATE)
                R.id.projects_control_suspend -> if (project!!.suspendedViaGUI) {
                    performProjectOperation(RpcClient.PROJECT_RESUME)
                } else {
                    performProjectOperation(RpcClient.PROJECT_SUSPEND)
                }
                R.id.projects_control_nonewtasks -> if (project!!.doNotRequestMoreWork) {
                    performProjectOperation(RpcClient.PROJECT_ANW)
                } else {
                    performProjectOperation(RpcClient.PROJECT_NNW)
                }
                R.id.projects_control_reset -> showConfirmationDialog(RpcClient.PROJECT_RESET)
                R.id.projects_control_remove -> showConfirmationDialog(RpcClient.PROJECT_DETACH)
                else -> if (Logging.WARNING) {
                    Log.w(Logging.TAG, "ProjectDetailsFragment onOptionsItemSelected: could not match ID")
                }
            }
        }
        return super.onOptionsItemSelected(item)
    }

    private fun showConfirmationDialog(operation: Int) {
        val dialog = Dialog(requireActivity()).apply {
            requestWindowFeature(Window.FEATURE_NO_TITLE)
            setContentView(R.layout.dialog_confirm)
        }
        val confirm = dialog.findViewById<Button>(R.id.confirm)
        val tvTitle = dialog.findViewById<TextView>(R.id.title)
        val tvMessage = dialog.findViewById<TextView>(R.id.message)

        // operation-dependent texts
        if (operation == RpcClient.PROJECT_DETACH) {
            val removeStr = getString(R.string.projects_confirm_detach_confirm)
            tvTitle.text = getString(R.string.projects_confirm_title, removeStr)
            tvMessage.text = getString(R.string.projects_confirm_message,
                    removeStr.toLowerCase(Locale.ROOT), project!!.projectName + " "
                            + getString(R.string.projects_confirm_detach_message))
            confirm.text = removeStr
        } else if (operation == RpcClient.PROJECT_RESET) {
            val resetStr = getString(R.string.projects_confirm_reset_confirm)
            tvTitle.text = getString(R.string.projects_confirm_title, resetStr)
            tvMessage.text = getString(R.string.projects_confirm_message, resetStr.toLowerCase(Locale.ROOT),
                    project!!.projectName)
            confirm.text = resetStr
        }
        confirm.setOnClickListener {
            lifecycleScope.launch {
                performProjectOperation(operation)
            }
            dialog.dismiss()
        }
        dialog.findViewById<Button>(R.id.cancel).apply {
            setOnClickListener { dialog.dismiss() }
        }
        dialog.show()
    }

    private fun populateLayout() {
        if (project == null) {
            retryLayout = true
            return  // if data not available yet, return. Frequently retry with onReceive
        }
        retryLayout = false
        val v = root
        updateChangingItems(v)
        slideshowWrapper = v.findViewById(R.id.slideshow_wrapper)
        slideshowHook = v.findViewById(R.id.slideshow_hook)
        slideshowLoading = v.findViewById(R.id.slideshow_loading)

        // set website
        val website = v.findViewById<TextView>(R.id.project_url)
        val content = SpannableString(project!!.masterURL)
        content.setSpan(UnderlineSpan(), 0, content.length, 0)
        website.text = content
        website.setOnClickListener {
            startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(project!!.masterURL)))
        }

        // set general area
        if (projectInfo?.generalArea != null) {
            v.findViewById<TextView>(R.id.general_area).apply {
                text = projectInfo!!.generalArea
            }
        } else {
            v.findViewById<LinearLayout>(R.id.general_area_wrapper).apply {
                visibility = View.GONE
            }
        }

        // set specific area
        if (projectInfo?.specificArea != null) {
            v.findViewById<TextView>(R.id.specific_area).apply {
                text = projectInfo!!.specificArea
            }
        } else {
            v.findViewById<LinearLayout>(R.id.specific_area_wrapper).apply {
                visibility = View.GONE
            }
        }

        // set description
        if (projectInfo?.description != null) {
            v.findViewById<TextView>(R.id.description).apply {
                text = projectInfo!!.description
            }
        } else {
            v.findViewById<LinearLayout>(R.id.description_wrapper).apply {
                visibility = View.GONE
            }
        }

        // set home
        if (projectInfo?.home != null) {
            v.findViewById<TextView>(R.id.based_at).apply {
                text = projectInfo!!.home
            }
        } else {
            v.findViewById<LinearLayout>(R.id.based_at_wrapper).apply {
                visibility = View.GONE
            }
        }

        // load slideshow
        lifecycleScope.launch {
            updateSlideshowImages()
        }
    }

    private fun updateChangingItems(v: View?) {
        try {
            // status
            val newStatus = BOINCActivity.monitor!!.getProjectStatus(project!!.masterURL)
            val wrapper = v!!.findViewById<LinearLayout>(R.id.status_wrapper)
            if (newStatus.isNotEmpty()) {
                wrapper.visibility = View.VISIBLE
                v.findViewById<TextView>(R.id.status_text).apply {
                    text = newStatus
                }
            } else {
                wrapper.visibility = View.GONE
            }
        } catch (e: Exception) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectDetailsFragment.updateChangingItems error: ", e)
            }
        }
    }

    // executes project operations in new thread
    private suspend fun performProjectOperation(operation: Int) = coroutineScope {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "performProjectOperation()")
        }

        val success = async {
            return@async try {
                BOINCActivity.monitor!!.projectOp(operation, project!!.masterURL)
            } catch (e: Exception) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "performProjectOperation() error: ", e)
                }
                false
            }
        }.await()

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                if (Logging.ERROR) {
                    Log.e(Logging.TAG, "performProjectOperation() error: ", e)
                }
            }
        } else if (Logging.WARNING) {
            Log.w(Logging.TAG, "performProjectOperation() failed.")
        }
    }

    private suspend fun updateSlideshowImages() = coroutineScope {
        if (Logging.DEBUG) {
            Log.d(Logging.TAG,
                    "UpdateSlideshowImagesAsync updating images in new thread. project:" +
                            " $project!!.masterURL")
        }
        val success = withContext(Dispatchers.Default) {
            slideshowImages = try {
                BOINCActivity.monitor!!.getSlideshowForProject(project!!.masterURL)
            } catch (e: Exception) {
                if (Logging.WARNING) {
                    Log.w(Logging.TAG, "updateSlideshowImages: Could not load data, " +
                            "clientStatus not initialized.")
                }
                return@withContext false
            }
            return@withContext slideshowImages.isNotEmpty()
        }

        if (Logging.DEBUG) {
            Log.d(Logging.TAG,
                    "UpdateSlideshowImagesAsync success: $success, images: ${slideshowImages.size}")
        }
        if (success && slideshowImages.isNotEmpty()) {
            slideshowLoading.visibility = View.GONE
            for (image in slideshowImages) {
                val iv = li.inflate(R.layout.project_details_slideshow_image_layout, null) as ImageView
                var bitmap = image.image!!
                if (scaleImages(bitmap.height, bitmap.width)) {
                    bitmap = Bitmap.createScaledBitmap(image.image!!,
                            image.image!!.width * 2,
                            image.image!!.height * 2, false)
                }
                iv.setImageBitmap(bitmap)
                slideshowHook.addView(iv)
            }
        } else {
            slideshowWrapper.visibility = View.GONE
        }
    }

    private fun scaleImages(imageHeight: Int, imageWidth: Int) = height >= imageHeight * 2 && width >= imageWidth * 2
}
