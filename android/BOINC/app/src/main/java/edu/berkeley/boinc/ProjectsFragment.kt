/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2022 University of California
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

import android.app.Dialog
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Bundle
import android.os.RemoteException
import android.view.LayoutInflater
import android.view.Menu
import android.view.MenuInflater
import android.view.MenuItem
import android.view.View
import android.view.ViewGroup
import android.view.Window
import androidx.core.net.toUri
import androidx.core.view.MenuHost
import androidx.core.view.MenuProvider
import androidx.fragment.app.Fragment
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.adapter.ProjectControlsRecyclerViewAdapter
import edu.berkeley.boinc.adapter.ProjectsListAdapter
import edu.berkeley.boinc.attach.ManualUrlInputFragment
import edu.berkeley.boinc.databinding.DialogConfirmBinding
import edu.berkeley.boinc.databinding.DialogListBinding
import edu.berkeley.boinc.databinding.ProjectsLayoutBinding
import edu.berkeley.boinc.rpc.AcctMgrInfo
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.RpcClient
import edu.berkeley.boinc.rpc.Transfer
import edu.berkeley.boinc.utils.ERR_OK
import edu.berkeley.boinc.utils.Logging
import java.util.*
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.coroutineScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class ProjectsFragment : Fragment() {
    private lateinit var listAdapter: ProjectsListAdapter
    private val data: MutableList<ProjectsListData> = ArrayList()

    // controls popup dialog
    var dialogControls: Dialog? = null

    // BroadcastReceiver event is used to update the UI with updated information from
    // the client.  This is generally called once a second.
    //
    private val ifcsc = IntentFilter("edu.berkeley.boinc.clientstatuschange")
    private val mClientStatusChangeRec: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            populateLayout()
        }
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
        Logging.logVerbose(Logging.Category.GUI_VIEW, "ProjectsFragment onCreateView")

        // Inflate the layout for this fragment
        val binding = ProjectsLayoutBinding.inflate(inflater, container, false)
        listAdapter = ProjectsListAdapter(requireActivity(), binding.projectsList, R.id.projects_list, data)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val menuHost: MenuHost = requireActivity() // enables fragment specific menu

        // add the project menu to the fragment
        menuHost.addMenuProvider(object: MenuProvider {
            override fun onCreateMenu(menu: Menu, menuInflater: MenuInflater) {
                menuInflater.inflate(R.menu.projects_menu, menu)
            }

            override fun onMenuItemSelected(menuItem: MenuItem): Boolean {
                Logging.logDebug(Logging.Category.USER_ACTION, "AttachProjectListActivity onOptionsItemSelected()")

                return when (menuItem.itemId) {
                    R.id.projects_add_url -> {
                        val dialog2 = ManualUrlInputFragment()
                        dialog2.show(parentFragmentManager, getString(R.string.attachproject_list_manual_button))
                        true
                    }
                    else -> true
                }
            }
        }, viewLifecycleOwner, Lifecycle.State.RESUMED)

    }

    override fun onPause() {
        Logging.logDebug(Logging.Category.GUI_VIEW, "ProjectsFragment onPause()")

        requireActivity().unregisterReceiver(mClientStatusChangeRec)
        super.onPause()
    }

    override fun onResume() {
        Logging.logDebug(Logging.Category.GUI_VIEW, "ProjectsFragment onResume()")

        super.onResume()
        populateLayout()
        requireActivity().registerReceiver(mClientStatusChangeRec, ifcsc)
    }

    private fun populateLayout() {
        try {
            // read projects from state saved in ClientStatus
            val statusProjects = BOINCActivity.monitor!!.projects
            val statusAcctMgr = BOINCActivity.monitor!!.clientAcctMgrInfo
            val statusTransfers = BOINCActivity.monitor!!.transfers

            // get server / scheduler notices to display if device does not meet
            val serverNotices = BOINCActivity.monitor!!.serverNotices

            // Update Project data
            updateData(statusProjects, statusAcctMgr, serverNotices, statusTransfers)

            // Force list adapter to refresh
            listAdapter.notifyDataSetChanged()
        } catch (e: Exception) {
            // data retrieval failed, set layout to loading...
            Logging.logError(Logging.Category.GUI_VIEW, "ProjectsActivity data retrieval failed.")
        }
    }

    private fun updateData(latestRpcProjectsList: List<Project>, acctMgrInfo: AcctMgrInfo,
                           serverNotices: List<Notice>?, ongoingTransfers: List<Transfer>) {
        // ACCOUNT MANAGER
        //loop through list adapter array to find index of account manager entry (0 || 1 manager possible)
        val mgrIndex = data.indexOfFirst { it.isMgr }
        if (mgrIndex < 0) { // no manager present until now
            Logging.logVerbose(Logging.Category.GUI_VIEW, "No manager found in layout list. New entry available: " +
                    acctMgrInfo.isPresent)

            if (acctMgrInfo.isPresent) {
                // add new manager entry, at top of the list
                data.add(ProjectsListData(null, acctMgrInfo, null))

                Logging.logDebug(Logging.Category.GUI_VIEW, "New acct mgr found: " + acctMgrInfo.acctMgrName)
            }
        } else { // manager found in existing list
            Logging.logDebug(Logging.Category.GUI_VIEW, "Manager found in layout list at index: $mgrIndex")

            if (!acctMgrInfo.isPresent) {
                // manager got detached, remove from list
                data.removeAt(mgrIndex)

                Logging.logDebug(Logging.Category.GUI_VIEW, "Acct mgr removed from list.")
            }
        }

        // ATTACHED PROJECTS
        //loop through all received Result items to add new projects
        for (rpcResult in latestRpcProjectsList) {
            //check whether this project is new
            val index = data.indexOfFirst { it.id == rpcResult.masterURL }
            if (index < 0) { // Project is new, add
                Logging.logDebug(Logging.Category.GUI_VIEW, "New project found, id: " + rpcResult.masterURL +
                        ", managed: " + rpcResult.attachedViaAcctMgr)

                if (rpcResult.attachedViaAcctMgr) {
                    data.add(ProjectsListData(rpcResult, null,
                            mapTransfersToProject(rpcResult.masterURL,
                                    ongoingTransfers))) // append to end of list (after manager)
                } else {
                    data.add(0, ProjectsListData(rpcResult, null,
                            mapTransfersToProject(rpcResult.masterURL,
                                    ongoingTransfers))) // put at top of list (before manager)
                }
            } else { // Project was present before, update its data
                data[index].updateProjectData(rpcResult, null,
                        mapTransfersToProject(rpcResult.masterURL,
                                ongoingTransfers))
            }
        }

        //loop through the list adapter to find removed (ready/aborted) projects
        data.removeIf { !it.isMgr && latestRpcProjectsList.none { (masterURL) -> it.id == masterURL } }

        // SERVER NOTICES
        // loop through active projects to add/remove server notices
        if (serverNotices != null) {
            var mappedServerNotices = 0
            for (project in data) {
                if (project.isMgr) {
                    continue  // do not seek notices in manager entries (crashes)
                }
                var noticeFound = false
                for (serverNotice in serverNotices) {
                    if (project.project!!.projectName == serverNotice.projectName) {
                        project.addServerNotice(serverNotice)
                        noticeFound = true
                        mappedServerNotices++
                    }
                }
                if (!noticeFound) {
                    project.addServerNotice(null)
                }
            }
            if (mappedServerNotices != serverNotices.size) {
                Logging.logError(Logging.Category.GUI_VIEW, "could not match notice: " + mappedServerNotices + "/" + serverNotices.size)
            }
        }
    }

    // takes list of all ongoing transfers and a project id (url) and returns transfer that belong to given project
    private fun mapTransfersToProject(id: String, allTransfers: List<Transfer>): List<Transfer> {
        // project id matches url in transfer, add to list
        val projectTransfers = allTransfers.filter { it.projectUrl == id }
        Logging.logDebug(Logging.Category.GUI_VIEW, "ProjectsActivity mapTransfersToProject() mapped " + projectTransfers.size +
                " transfers to project " + id)

        return projectTransfers
    }

    // data wrapper for list view
    inner class ProjectsListData(
            // can be either project or account manager
            var project: Project?,
            var acctMgrInfo: AcctMgrInfo?,
            var projectTransfers: List<Transfer>?
    ) {
        var lastServerNotice: Notice? = null
        // == url
        @JvmField
        var id: String = ""
        @JvmField
        var isMgr = false
        private var listEntry = this

        fun updateProjectData(data: Project?, acctMgrInfo: AcctMgrInfo?, projectTransfers: List<Transfer>?) {
            if (isMgr) {
                this.acctMgrInfo = acctMgrInfo
            } else {
                project = data
                this.projectTransfers = projectTransfers
            }
        }

        fun addServerNotice(notice: Notice?) {
            lastServerNotice = notice
        }

        // handles onClick on list element, could be either project or account manager
        // sets up dialog with controls
        @JvmField
        val projectsListClickListener = View.OnClickListener {
            val dialogBinding = DialogListBinding.inflate(layoutInflater)
            dialogControls = Dialog(requireActivity()).apply {
                // layout
                requestWindowFeature(Window.FEATURE_NO_TITLE)
                setContentView(dialogBinding.root)
            }

            // add control items depending on:
            // - type, account manager vs. project
            // - client status, e.g. either suspend or resume
            // - show advanced preference
            // - project attached via account manager (e.g. hide Remove)
            val controls: MutableList<ProjectControl> = ArrayList()
            if (isMgr) {
                dialogBinding.title.setText(R.string.projects_control_dialog_title_acctmgr)
                controls.add(ProjectControl(listEntry, VISIT_WEBSITE))
                controls.add(ProjectControl(listEntry, RpcClient.MGR_SYNC))
                controls.add(ProjectControl(listEntry, RpcClient.MGR_DETACH))
            } else {
                dialogBinding.title.setText(R.string.projects_control_dialog_title)
                controls.add(ProjectControl(listEntry, VISIT_WEBSITE))
                if (!projectTransfers.isNullOrEmpty()) {
                    controls.add(ProjectControl(listEntry, RpcClient.TRANSFER_RETRY))
                }
                controls.add(ProjectControl(listEntry, RpcClient.PROJECT_UPDATE))
                if (project!!.suspendedViaGUI) {
                    controls.add(ProjectControl(listEntry, RpcClient.PROJECT_RESUME))
                } else {
                    controls.add(ProjectControl(listEntry, RpcClient.PROJECT_SUSPEND))
                }
                val isShowAdvanced = try {
                    BOINCActivity.monitor!!.showAdvanced
                } catch (e: RemoteException) {
                    false
                }
                if (isShowAdvanced) {
                    if (project!!.doNotRequestMoreWork) {
                        controls.add(ProjectControl(listEntry, RpcClient.PROJECT_ANW))
                    } else {
                        controls.add(ProjectControl(listEntry, RpcClient.PROJECT_NNW))
                    }
                    controls.add(ProjectControl(listEntry, RpcClient.PROJECT_RESET))
                }
                if (!project!!.attachedViaAcctMgr) {
                    controls.add(ProjectControl(listEntry, RpcClient.PROJECT_DETACH))
                }
            }

            // list adapter
            dialogBinding.options.adapter = ProjectControlsRecyclerViewAdapter(activity!!, controls)
            dialogBinding.options.layoutManager = LinearLayoutManager(activity)

            Logging.logDebug(Logging.Category.USER_ACTION, "dialog list adapter entries: " + controls.size)

            // buttons
            dialogBinding.cancel.setOnClickListener { dialogControls!!.dismiss() }

            // show dialog
            dialogControls!!.show()
        }

        init {
            if (project == null && acctMgrInfo != null) {
                isMgr = true
            }
            id = if (isMgr) {
                acctMgrInfo!!.acctMgrUrl
            } else {
                project!!.masterURL
            }
        }
    }

    inner class ProjectControl(var data: ProjectsListData, var operation: Int) {

        // handles onClick on list element in control dialog
        // might show confirmation dialog depending on operation type
        @JvmField
        val projectCommandClickListener = View.OnClickListener {
            //check whether command requires confirmation
            if (operation == RpcClient.PROJECT_DETACH || operation == RpcClient.PROJECT_RESET ||
                    operation == RpcClient.MGR_DETACH) {
                val dialogBinding = DialogConfirmBinding.inflate(layoutInflater)
                val dialog = Dialog(activity!!).apply {
                    requestWindowFeature(Window.FEATURE_NO_TITLE)
                    setContentView(dialogBinding.root)
                }

                // operation-dependent texts
                when (operation) {
                    RpcClient.PROJECT_DETACH -> {
                        val removeStr = getString(R.string.projects_confirm_detach_confirm)
                        dialogBinding.title.text = getString(R.string.projects_confirm_title, removeStr)
                        dialogBinding.message.text = getString(R.string.projects_confirm_message,
                            removeStr.lowercase(Locale.ROOT),
                                data.project!!.projectName + " "
                                        + getString(R.string.projects_confirm_detach_message))
                        dialogBinding.confirm.text = removeStr
                    }
                    RpcClient.PROJECT_RESET -> {
                        val resetStr = getString(R.string.projects_confirm_reset_confirm)
                        dialogBinding.title.text = getString(R.string.projects_confirm_title, resetStr)
                        dialogBinding.message.text = getString(R.string.projects_confirm_message,
                            resetStr.lowercase(Locale.ROOT),
                                data.project!!.projectName)
                        dialogBinding.confirm.text = resetStr
                    }
                    RpcClient.MGR_DETACH -> {
                        dialogBinding.title.setText(R.string.projects_confirm_remove_acctmgr_title)
                        dialogBinding.message.text = getString(R.string.projects_confirm_message,
                                getString(R.string.projects_confirm_remove_acctmgr_message),
                                data.acctMgrInfo!!.acctMgrName)
                        dialogBinding.confirm.setText(R.string.projects_confirm_remove_acctmgr_confirm)
                    }
                }
                dialogBinding.confirm.setOnClickListener {
                    lifecycleScope.launch {
                        performProjectOperationAsync(data, operation)
                    }
                    dialog.dismiss()
                    dialogControls!!.dismiss()
                }
                dialogBinding.cancel.setOnClickListener { dialog.dismiss() }
                dialog.show()
            } else if (operation == VISIT_WEBSITE) { // command does not require confirmation and is not RPC based
                dialogControls!!.dismiss()
                startActivity(Intent(Intent.ACTION_VIEW, data.id.toUri()))
            } else { // command does not require confirmation, but is RPC based
                lifecycleScope.launch {
                    performProjectOperationAsync(data, operation)
                }
                dialogControls!!.dismiss()
            }
        }
    }

    // executes project operations in new thread
    private suspend fun performProjectOperationAsync(data: ProjectsListData, operation: Int) = coroutineScope {
        val success = withContext(Dispatchers.Default) { performProjectOperation(data, operation) }

        if (success) {
            try {
                BOINCActivity.monitor!!.forceRefresh()
            } catch (e: RemoteException) {
                e.printStackTrace()
            }
        } else {
            Logging.logError(Logging.Category.USER_ACTION, "ProjectOperationAsync failed.")
        }
    }

    private fun performProjectOperation(data: ProjectsListData, operation: Int): Boolean {
        try {
            Logging.logVerbose(Logging.Category.USER_ACTION,
                    "ProjectOperationAsync isMgr: ${data.isMgr}, url: ${data.id}," +
                    " operation: $operation")

            when (operation) {
                RpcClient.PROJECT_UPDATE, RpcClient.PROJECT_SUSPEND, RpcClient.PROJECT_RESUME,
                RpcClient.PROJECT_NNW, RpcClient.PROJECT_ANW, RpcClient.PROJECT_DETACH,
                RpcClient.PROJECT_RESET -> return BOINCActivity.monitor!!.projectOp(operation, data.id)
                RpcClient.MGR_SYNC ->
                    return BOINCActivity.monitor!!.synchronizeAcctMgr(data.acctMgrInfo!!.acctMgrUrl)
                RpcClient.MGR_DETACH ->
                    return BOINCActivity.monitor!!.addAcctMgrErrorNum("", "", "")
                            .code == ERR_OK
                RpcClient.TRANSFER_RETRY ->
                    return data.projectTransfers.isNullOrEmpty() ||
                         BOINCActivity.monitor!!.transferOperation(data.projectTransfers!!, operation)
                else -> if (operation != RpcClient.TRANSFER_ABORT) {
                    Logging.logError(Logging.Category.USER_ACTION, "ProjectOperationAsync could not match operation: $operation")
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.USER_ACTION, "ProjectOperationAsync error in do in background", e)
        }
        return false
    }

    companion object {
        // operation that do not imply an RPC are defined here
        const val VISIT_WEBSITE = 100
    }
}
