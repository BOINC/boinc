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
package edu.berkeley.boinc

import android.app.Activity
import android.app.Dialog
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.graphics.Point
import android.os.Build
import android.os.Bundle
import android.os.RemoteException
import android.text.SpannableString
import android.text.style.UnderlineSpan
import android.view.*
import android.widget.Button
import android.widget.TextView
import androidx.core.graphics.scale
import androidx.core.net.toUri
import androidx.core.view.MenuHost
import androidx.core.view.MenuProvider
import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import edu.berkeley.boinc.databinding.ProjectDetailsLayoutBinding
import edu.berkeley.boinc.databinding.ProjectDetailsSlideshowImageLayoutBinding
import edu.berkeley.boinc.rpc.ImageWrapper
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.utils.Logging
import java.util.*
import kotlinx.coroutines.*

class ProjectDetailsFragment : Fragment() {
    private var url: String = ""

    // might be null for projects added via manual URL attach
    private var projectInfo: ProjectInfo? = null

    private var project: Project? = null
    private var slideshowImages: List<ImageWrapper> = ArrayList()

    private var _binding: ProjectDetailsLayoutBinding? = null
    private val binding get() = _binding!!

    // display dimensions
    private var width = 0
    private var height = 0
    private var retryLayout = true

    private val currentProjectData: Unit
        get() {
            try {
                project = BOINCActivity.monitor!!.projects.firstOrNull { it.masterURL == url }
                projectInfo = BOINCActivity.monitor!!.getProjectInfoAsync(url).await()
            } catch (e: Exception) {
                Logging.logError(Logging.Category.GUI_VIEW, "ProjectDetailsFragment getCurrentProjectData could not" +
                        " retrieve project list")
            }
            if (project == null) {
                Logging.logWarning(Logging.Category.GUI_VIEW,
                        "ProjectDetailsFragment getCurrentProjectData could not find project for URL: $url")
            }
            if (projectInfo == null) {
                Logging.logDebug(Logging.Category.GUI_VIEW,
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
                updateChangingItems()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        // get data
        url = requireArguments().getString("url") ?: ""
        currentProjectData

        super.onCreate(savedInstanceState)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val menuHost: MenuHost = requireActivity() // enables fragment specific menu

        // add the project menu to the fragment
        menuHost.addMenuProvider(object: MenuProvider {
            override fun onCreateMenu(menu: Menu, menuInflater: MenuInflater) {
                menuInflater.inflate(R.menu.project_details_menu, menu)
            }

            override fun onPrepareMenu(menu: Menu) {
                super.onPrepareMenu(menu)

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

            override fun onMenuItemSelected(menuItem: MenuItem): Boolean {
                lifecycleScope.launch {
                    when (menuItem.itemId) {
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
                        else -> {
                            Logging.logError(Logging.Category.USER_ACTION, "ProjectDetailsFragment onOptionsItemSelected: could not match ID")
                        }
                    }
                }

                return true
            }
        }, viewLifecycleOwner, Lifecycle.State.RESUMED)
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "ProjectDetailsFragment onCreateView")

        // Inflate the layout for this fragment
        _binding = ProjectDetailsLayoutBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    override fun onAttach(context: Context) {
        if (context is Activity) {
            val size = Point()
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
                @Suppress("DEPRECATION")
                context.windowManager.defaultDisplay.getSize(size)
            } else {
                val r = context.windowManager.currentWindowMetrics.bounds
                size.x = r.width()
                size.y = r.height()
            }
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
                removeStr.lowercase(Locale.ROOT), project!!.projectName + " "
                            + getString(R.string.projects_confirm_detach_message))
            confirm.text = removeStr
        } else if (operation == RpcClient.PROJECT_RESET) {
            val resetStr = getString(R.string.projects_confirm_reset_confirm)
            tvTitle.text = getString(R.string.projects_confirm_title, resetStr)
            tvMessage.text = getString(R.string.projects_confirm_message,
                resetStr.lowercase(Locale.ROOT),
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
        updateChangingItems()

        // set website
        val content = SpannableString(project!!.masterURL)
        content.setSpan(UnderlineSpan(), 0, content.length, 0)
        binding.projectUrl.text = content
        binding.projectUrl.setOnClickListener {
            startActivity(Intent(Intent.ACTION_VIEW, project!!.masterURL.toUri()))
        }

        // set general area
        if (projectInfo?.generalArea != null) {
            binding.generalArea.text = projectInfo!!.generalArea
        } else {
            binding.generalAreaWrapper.visibility = View.GONE
        }

        // set specific area
        if (projectInfo?.specificArea != null) {
            binding.specificArea.text = projectInfo!!.specificArea
        } else {
            binding.specificAreaWrapper.visibility = View.GONE
        }

        // set description
        if (projectInfo?.description != null) {
            binding.description.text = projectInfo!!.description
        } else {
            binding.descriptionWrapper.visibility = View.GONE
        }

        // set home
        if (projectInfo?.home != null) {
            binding.basedAt.text = projectInfo!!.home
        } else {
            binding.basedAtWrapper.visibility = View.GONE
        }

        // load slideshow
        lifecycleScope.launch {
            updateSlideshowImages()
        }
    }

    private fun updateChangingItems() {
        try {
            // status
            val newStatus = BOINCActivity.monitor!!.getProjectStatus(project!!.masterURL)
            if (newStatus.isNotEmpty()) {
                binding.statusWrapper.visibility = View.VISIBLE
                binding.statusText.text = newStatus
            } else {
                binding.statusWrapper.visibility = View.GONE
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.GUI_VIEW, "ProjectDetailsFragment.updateChangingItems error: ", e)
        }
    }

    // executes project operations in new thread
    private suspend fun performProjectOperation(operation: Int) = coroutineScope {
        Logging.logVerbose(Logging.Category.USER_ACTION, "performProjectOperation()")

        val success = async {
            return@async try {
                BOINCActivity.monitor!!.projectOp(operation, project!!.masterURL)
            } catch (e: Exception) {
                Logging.logException(Logging.Category.USER_ACTION, "performProjectOperation() error: ", e)

                false
            }
        }.await()

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                Logging.logException(Logging.Category.USER_ACTION, "performProjectOperation() error: ", e)
            }
        } else {
            Logging.logError(Logging.Category.USER_ACTION, "performProjectOperation() failed.")
        }
    }

    private suspend fun updateSlideshowImages() = coroutineScope {
        Logging.logDebug(Logging.Category.GUI_VIEW,
                "UpdateSlideshowImagesAsync updating images in new thread. project:" +
                " $project!!.masterURL")

        val success = withContext(Dispatchers.Default) {
            slideshowImages = try {
                BOINCActivity.monitor!!.getSlideshowForProject(project!!.masterURL)
            } catch (e: Exception) {
                Logging.logError(Logging.Category.GUI_VIEW, "updateSlideshowImages: Could not load data, " +
                        "clientStatus not initialized.")

                return@withContext false
            }
            return@withContext slideshowImages.isNotEmpty()
        }

        Logging.logDebug(Logging.Category.GUI_VIEW,
                "UpdateSlideshowImagesAsync success: $success, images: ${slideshowImages.size}")
        if (success && slideshowImages.isNotEmpty()) {
            binding.slideshowLoading.visibility = View.GONE
            for (image in slideshowImages) {
                val slideshowBinding = ProjectDetailsSlideshowImageLayoutBinding.inflate(layoutInflater)
                var bitmap = image.image!!
                if (scaleImages(bitmap.height, bitmap.width)) {
                    bitmap = bitmap.scale(bitmap.width * 2, bitmap.height * 2, filter = false)
                }
                slideshowBinding.slideshowImage.setImageBitmap(bitmap)
                binding.slideshowHook.addView(slideshowBinding.slideshowImage)
            }
        } else {
            binding.slideshowWrapper.visibility = View.GONE
        }
    }

    private fun scaleImages(imageHeight: Int, imageWidth: Int) = height >= imageHeight * 2 && width >= imageWidth * 2
}
