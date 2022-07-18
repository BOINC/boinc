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

import android.content.Intent
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.adapter.AttachActivityItem
import edu.berkeley.boinc.adapter.AttachActivityItemAdapter
import edu.berkeley.boinc.adapter.AttachActivityItemType
import edu.berkeley.boinc.databinding.ActivityAttachBinding
import edu.berkeley.boinc.utils.Logging

class AttachActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val binding = ActivityAttachBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // If AccountManager is already connected, user should not be able to connect more AMs
        // Hide 'Use Account Manager' option
        var statusAcctMgrPresent = false
        try {
            val statusAcctMgr = BOINCActivity.monitor!!.clientAcctMgrInfo
            statusAcctMgrPresent = statusAcctMgr.isPresent
        } catch (e: Exception) {
            // data retrieval failed, continue...
            Logging.logException(Logging.Category.MONITOR, "AcctMgrInfo data retrieval failed.", e)
        }

        val attachActivityItemList = listOf(
                AttachActivityItem(
                        AttachActivityItemType.ALL_PROJECTS,
                        getString(R.string.attachproject_attach_projects_header),
                        getString(R.string.attachproject_attach_projects_desc))
        ).toMutableList()

        if (!statusAcctMgrPresent) {
            attachActivityItemList.add(AttachActivityItem(
                    AttachActivityItemType.ACCOUNT_MANAGER,
                    getString(R.string.attachproject_acctmgr_header),
                    getString(R.string.attachproject_acctmgr_list_desc)
            ))
        }

        binding.itemsRecyclerView.adapter = AttachActivityItemAdapter(attachActivityItemList, this)
        binding.itemsRecyclerView.layoutManager = LinearLayoutManager(this)

        binding.cancelButton.setOnClickListener {
            // go to projects screen and clear history
            startActivity(Intent(this@AttachActivity, BOINCActivity::class.java).apply {
                // add flags to return to main activity and clearing all others and clear the back stack
                addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                putExtra("targetFragment", R.string.tab_projects) // make activity display projects fragment
            })
        }
    }

    fun startSelectionProjectActivity() =
            startActivity(Intent(this@AttachActivity, SelectionListActivity::class.java))

    fun startAccountManagerActivity() =
            startActivity(Intent(this@AttachActivity, AttachAccountManagerActivity::class.java))
}
