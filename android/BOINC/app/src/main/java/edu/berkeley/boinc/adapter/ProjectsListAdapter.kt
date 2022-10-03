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
package edu.berkeley.boinc.adapter

import android.R.color
import android.app.Activity
import android.graphics.Bitmap
import android.text.format.DateUtils
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.ImageView
import android.widget.ListView
import android.widget.RelativeLayout
import android.widget.TextView
import androidx.appcompat.content.res.AppCompatResources
import androidx.core.content.ContextCompat
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.ProjectsFragment.ProjectsListData
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.TransferStatus.ERR_GIVEUP_DOWNLOAD
import edu.berkeley.boinc.rpc.TransferStatus.ERR_GIVEUP_UPLOAD
import edu.berkeley.boinc.utils.Logging.Category.GUI_VIEW
import edu.berkeley.boinc.utils.Logging.Category.MONITOR
import edu.berkeley.boinc.utils.Logging.logException
import java.text.DecimalFormat
import java.text.NumberFormat
import java.time.Duration
import java.time.Instant
import kotlin.math.roundToInt
import org.apache.commons.lang3.StringUtils

class ProjectsListAdapter(
    private val activity: Activity,
    listView: ListView,
    textViewResourceId: Int,
    private val entries: List<ProjectsListData>
) :
    ArrayAdapter<ProjectsListData>(activity, textViewResourceId, entries) {
    init {
        listView.adapter = this
    }

    override fun getCount(): Int {
        return entries.size
    }

    override fun getItem(position: Int): ProjectsListData {
        return entries[position]
    }

    override fun getItemId(position: Int): Long {
        return position.toLong()
    }

    fun getDiskUsage(position: Int): String {
        val diskUsage = entries[position].project!!.diskUsage
        val df = DecimalFormat("#.##")
        return df.format(diskUsage / (1024 * 1024))
    }

    fun getName(position: Int): String {
        return entries[position].project!!.projectName
    }

    private fun getUser(position: Int): String {
        val user = entries[position].project!!.userName
        val team = entries[position].project!!.teamName
        return if (team.isNotEmpty()) {
            "$user ($team)"
        } else user
    }

    private fun getIcon(position: Int): Bitmap? {
        // try to get current client status from monitor
        return try {
            BOINCActivity.monitor!!.getProjectIcon(entries[position].id)
        } catch (e: Exception) {
            logException(
                MONITOR,
                "ProjectsListAdapter: Could not load data, clientStatus not initialized.",
                e
            )
            null
        }
    }

    override fun getView(position: Int, convertView: View?, parent: ViewGroup): View {
        val data = entries[position]
        val isAcctMgr = data.isMgr
        var vi = convertView
        // setup new view, if:
        // - view is null, has not been here before
        // - view has different id
        var setup = false
        if (vi == null) {
            setup = true
        } else {
            val viewId = vi.tag?.toString().orEmpty()
            if (!StringUtils.equals(data.id, viewId)) {
                setup = true
            }
        }
        if (setup) {
            val layoutInflater = ContextCompat.getSystemService(
                activity,
                LayoutInflater::class.java
            )!!
            vi = if (isAcctMgr) {
                layoutInflater.inflate(R.layout.projects_layout_listitem_acctmgr, null)
            } else {
                layoutInflater.inflate(R.layout.projects_layout_listitem, null)
            }
            vi!!.setOnClickListener(entries[position].projectsListClickListener)
            vi.tag = data.id
        }
        if (isAcctMgr) {
            // element is account manager

            // populate name
            val tvName = vi!!.findViewById<TextView>(R.id.name)
            tvName.text = data.acctMgrInfo!!.acctMgrName

            // populate url
            val tvUrl = vi.findViewById<TextView>(R.id.url)
            tvUrl.text = data.acctMgrInfo!!.acctMgrUrl
        } else {
            // element is project
            // set data of standard elements
            val tvName = vi!!.findViewById<TextView>(R.id.project_name)
            tvName.text = getName(position)
            val tvUser = vi.findViewById<TextView>(R.id.project_user)
            val userText = getUser(position)
            if (userText.isEmpty()) {
                tvUser.visibility = View.GONE
            } else {
                tvUser.visibility = View.VISIBLE
                tvUser.text = userText
            }
            var statusText = ""
            try {
                statusText = BOINCActivity.monitor!!.getProjectStatus(data.project!!.masterURL)
            } catch (e: Exception) {
                logException(GUI_VIEW, "ProjectsListAdapter.getView error: ", e)
            }
            val tvStatus = vi.findViewById<TextView>(R.id.project_status)
            if (statusText.isEmpty()) {
                tvStatus.visibility = View.GONE
            } else {
                tvStatus.visibility = View.VISIBLE
                tvStatus.text = statusText
            }
            val ivIcon = vi.findViewById<ImageView>(R.id.project_icon)
            val finalIconId = ivIcon.tag?.toString().orEmpty()
            if (!StringUtils.equals(finalIconId, data.id)) {
                val icon = getIcon(position)
                // if available set icon, if not boinc logo
                if (icon == null) {
                    // BOINC logo
                    ivIcon.setImageResource(R.drawable.ic_boinc)
                } else {
                    // project icon
                    ivIcon.setImageBitmap(icon)
                    // mark as final
                    ivIcon.tag = data.id
                }
            }

            // transfers
            val numberTransfers = data.projectTransfers!!.size
            val tvTransfers = vi.findViewById<TextView>(R.id.project_transfers)
            var transfersString = ""
            if (numberTransfers > 0) { // ongoing transfers
                // summarize information for compact representation
                var numberTransfersUpload = 0
                var uploadsPresent = false
                var numberTransfersDownload = 0
                var downloadsPresent = false
                var transfersActive = false // true if at least one transfer is active
                var nextRetryS: Long = 0
                var transferStatus = 0

                for (trans in data.projectTransfers!!) {
                    if (trans.isUpload) {
                        numberTransfersUpload++
                        uploadsPresent = true
                    } else {
                        numberTransfersDownload++
                        downloadsPresent = true
                    }
                    if (trans.isTransferActive) {
                        transfersActive = true
                    } else if (trans.nextRequestTime < nextRetryS || nextRetryS == 0L) {
                        nextRetryS = trans.nextRequestTime
                        transferStatus = trans.status
                    }
                }

                var numberTransfersString = "(" // will never be empty
                if (downloadsPresent) {
                    numberTransfersString += "$numberTransfersDownload " + activity.resources.getString(
                        R.string.trans_download
                    )
                }
                if (downloadsPresent && uploadsPresent) {
                    numberTransfersString += " / "
                }
                if (uploadsPresent) {
                    numberTransfersString += "$numberTransfersUpload " + activity.resources.getString(
                        R.string.trans_upload
                    )
                }
                numberTransfersString += ")"
                var activityStatus = "" // will never be empty
                var activityExplanation = ""
                if (nextRetryS > Instant.now().epochSecond) {
                    activityStatus += activity.resources.getString(R.string.trans_pending)
                    val retryInSeconds = Duration.between(
                        Instant.now(),
                        Instant.ofEpochSecond(nextRetryS)
                    ).seconds
                    // if timestamp is in the past, do not write anything
                    if (retryInSeconds >= 0) {
                        val formattedTime = DateUtils.formatElapsedTime(retryInSeconds)
                        activityExplanation += activity.resources.getString(
                            R.string.trans_retry_in,
                            formattedTime
                        )
                    }
                } else if (ERR_GIVEUP_DOWNLOAD.status == transferStatus || ERR_GIVEUP_UPLOAD.status == transferStatus) {
                    activityStatus += activity.resources.getString(R.string.trans_failed)
                } else {
                    activityStatus += if (BOINCActivity.monitor!!.networkSuspendReason != 0) {
                        activity.resources.getString(R.string.trans_suspended)
                    } else {
                        if (transfersActive) {
                            activity.resources.getString(R.string.trans_active)
                        } else {
                            activity.resources.getString(R.string.trans_pending)
                        }
                    }
                }
                transfersString += activity.resources.getString(R.string.tab_transfers) + " " + activityStatus + " " +
                        numberTransfersString + " " + activityExplanation
                tvTransfers.visibility = View.VISIBLE
                tvTransfers.text = transfersString
            } else { // no ongoing transfers
                tvTransfers.visibility = View.GONE
            }

            // credits
            val userCredit = data.project!!.userTotalCredit.roundToInt()
            val hostCredit = data.project!!.hostTotalCredit.roundToInt()
            (vi.findViewById<View>(R.id.project_credits) as TextView).text =
                if (hostCredit == userCredit) NumberFormat.getIntegerInstance()
                    .format(hostCredit) else activity.getString(
                    R.string.projects_credits_host_and_user,
                    hostCredit,
                    userCredit
                )
            val tvDiskUsage = vi.findViewById<TextView>(R.id.project_disk_usage)
            val diskUsage = getDiskUsage(position)
            tvDiskUsage.text =
                activity.getString(R.string.projects_disk_usage_with_unit, diskUsage)

            // server notice
            val notice = data.lastServerNotice
            val tvNotice = vi.findViewById<TextView>(R.id.project_notice)
            if (notice == null) {
                tvNotice.visibility = View.GONE
            } else {
                tvNotice.visibility = View.VISIBLE
                val noticeText = notice.description.trim { it <= ' ' }
                tvNotice.text = noticeText
            }

            // icon background
            val iconBackground = vi.findViewById<RelativeLayout>(R.id.icon_background)
            if (data.project!!.attachedViaAcctMgr) {
                val background = AppCompatResources.getDrawable(
                    activity.applicationContext,
                    R.drawable.shape_boinc_icon_light_blue_background
                )
                iconBackground.background = background
            } else {
                iconBackground.setBackgroundColor(
                    ContextCompat.getColor(
                        activity.applicationContext,
                        color.transparent
                    )
                )
            }
        }
        return vi
    }
}
