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
package edu.berkeley.boinc.client

import android.graphics.Bitmap
import android.os.IBinder
import edu.berkeley.boinc.rpc.AccountIn
import edu.berkeley.boinc.rpc.AccountManager
import edu.berkeley.boinc.rpc.AccountOut
import edu.berkeley.boinc.rpc.AcctMgrInfo
import edu.berkeley.boinc.rpc.GlobalPreferences
import edu.berkeley.boinc.rpc.HostInfo
import edu.berkeley.boinc.rpc.ImageWrapper
import edu.berkeley.boinc.rpc.Notice
import edu.berkeley.boinc.rpc.Project
import edu.berkeley.boinc.rpc.ProjectConfig
import edu.berkeley.boinc.rpc.ProjectInfo
import edu.berkeley.boinc.rpc.Result
import edu.berkeley.boinc.rpc.Transfer
import edu.berkeley.boinc.utils.ErrorCodeDescription
import edu.berkeley.boinc.utils.TaskRunner

class MonitorAsync(monitor: IMonitor?) : IMonitor {
    val monitor = monitor!!

    fun quitClientAsync(callback: ((Boolean) -> Unit)? = null) =
            TaskRunner(callback, {quitClient()})

    fun runBenchmarksAsync(callback: ((Boolean) -> Unit)? = null) =
            TaskRunner(callback, {runBenchmarks()})

    fun setGlobalPreferencesAsync(prefs: GlobalPreferences, callback: ((Boolean) -> Unit)? = null) =
            TaskRunner(callback, {setGlobalPreferences(prefs)})

    fun setCcConfigAsync(config: String, callback: ((Boolean) -> Unit)? = null) =
            TaskRunner(callback, {setCcConfig(config)})

    fun getProjectInfoAsync(url: String, callback: ((ProjectInfo?) -> Unit)? = null) =
            TaskRunner(callback, {getProjectInfo(url)})

    fun setRunModeAsync(mode: Int, callback: ((Boolean) -> Unit)? = null) =
            TaskRunner(callback, {setRunMode(mode)})

    fun setNetworkModeAsync(mode: Int, callback: ((Boolean) -> Unit)? = null) =
            TaskRunner(callback, {setNetworkMode(mode)})

    fun getAccountManagersAsync(callback: ((List<AccountManager>) -> Unit)? = null) =
            TaskRunner(callback, {accountManagers})

    override fun asBinder(): IBinder {
        return monitor.asBinder()
    }

    override fun attachProject(url: String, projectName: String, authenticator: String): Boolean {
        return monitor.attachProject(url, projectName, authenticator)
    }

    override fun checkProjectAttached(url: String): Boolean {
        return monitor.checkProjectAttached(url)
    }

    override fun lookupCredentials(credentials: AccountIn): AccountOut {
        return monitor.lookupCredentials(credentials)
    }

    override fun projectOp(status: Int, url: String): Boolean {
        return monitor.projectOp(status, url)
    }

    override fun resultOp(op: Int, url: String, name: String): Boolean {
        return monitor.resultOp(op, url, name)
    }

    override fun createAccountPolling(information: AccountIn): AccountOut {
        return monitor.createAccountPolling(information)
    }

    override fun readAuthToken(path: String): String {
        return monitor.readAuthToken(path)
    }

    override fun getProjectConfigPolling(url: String): ProjectConfig {
        return monitor.getProjectConfigPolling(url)
    }

    override fun addAcctMgrErrorNum(url: String, userName: String, pwd: String): ErrorCodeDescription {
        return monitor.addAcctMgrErrorNum(url, userName, pwd)
    }

    override fun getAcctMgrInfo(): AcctMgrInfo {
        return monitor.acctMgrInfo
    }

    override fun synchronizeAcctMgr(url: String): Boolean {
        return monitor.synchronizeAcctMgr(url)
    }

    override fun setRunMode(mode: Int): Boolean {
        return monitor.setRunMode(mode)
    }

    override fun setNetworkMode(mode: Int): Boolean {
        return monitor.setNetworkMode(mode)
    }

    override fun getEventLogMessages(seq: Int, num: Int): List<Message> {
        return monitor.getEventLogMessages(seq, num)
    }

    override fun getMessages(seq: Int): List<Message> {
        return monitor.getMessages(seq)
    }

    override fun getNotices(seq: Int): List<Notice> {
        return monitor.getNotices(seq)
    }

    override fun setCcConfig(config: String): Boolean {
        return monitor.setCcConfig(config)
    }

    override fun setGlobalPreferences(pref: GlobalPreferences): Boolean {
        return monitor.setGlobalPreferences(pref)
    }

    override fun transferOperation(list: List<Transfer>, op: Int): Boolean {
        return monitor.transferOperation(list, op)
    }

    override fun getServerNotices(): List<Notice> {
        return monitor.serverNotices
    }

    override fun runBenchmarks(): Boolean {
        return monitor.runBenchmarks()
    }

    override fun getAttachableProjects(): List<ProjectInfo> {
        return monitor.attachableProjects
    }

    override fun getAccountManagers(): List<AccountManager> {
        return monitor.accountManagers
    }

    override fun getProjectInfo(url: String): ProjectInfo? {
        return monitor.getProjectInfo(url)
    }

    override fun setDomainName(deviceName: String): Boolean {
        return monitor.setDomainName(deviceName)
    }

    override fun boincMutexAcquired(): Boolean {
        return monitor.boincMutexAcquired()
    }

    override fun forceRefresh() {
        return monitor.forceRefresh()
    }

    override fun isStationaryDeviceSuspected(): Boolean {
        return monitor.isStationaryDeviceSuspected
    }

    override fun getBatteryChargeStatus(): Int {
        return monitor.batteryChargeStatus
    }

    override fun getAuthFilePath(): String {
        return monitor.authFilePath
    }

    override fun getBoincPlatform(): Int {
        return monitor.boincPlatform
    }

    override fun cancelNoticeNotification() {
        return monitor.cancelNoticeNotification()
    }

    override fun setShowNotificationDuringSuspend(isShow: Boolean) {
        monitor.showNotificationDuringSuspend = isShow
    }

    override fun getShowNotificationDuringSuspend(): Boolean {
        return monitor.showNotificationDuringSuspend
    }

    override fun quitClient(): Boolean {
        return monitor.quitClient()
    }

    override fun getAcctMgrInfoPresent(): Boolean {
        return monitor.acctMgrInfoPresent
    }

    override fun getSetupStatus(): Int {
        return monitor.setupStatus
    }

    override fun getComputingStatus(): Int {
        return monitor.computingStatus
    }

    override fun getComputingSuspendReason(): Int {
        return monitor.computingSuspendReason
    }

    override fun getNetworkSuspendReason(): Int {
        return monitor.networkSuspendReason
    }

    override fun getCurrentStatusTitle(): String {
        return monitor.currentStatusTitle
    }

    override fun getCurrentStatusDescription(): String {
        return monitor.currentStatusDescription
    }

    override fun getHostInfo(): HostInfo {
        return monitor.hostInfo
    }

    override fun getPrefs(): GlobalPreferences {
        return monitor.prefs
    }

    override fun getProjects(): List<Project> {
        return monitor.projects
    }

    override fun getClientAcctMgrInfo(): AcctMgrInfo {
        return monitor.clientAcctMgrInfo
    }

    override fun getTransfers(): List<Transfer> {
        return monitor.transfers
    }

    override fun getTasks(start: Int, count: Int, isActive: Boolean): List<Result> {
        return monitor.getTasks(start, count, isActive)
    }

    override fun getTasksCount(): Int {
        return monitor.tasksCount
    }

    override fun getProjectIconByName(name: String): Bitmap? {
        return monitor.getProjectIconByName(name)
    }

    override fun getProjectIcon(id: String): Bitmap? {
        return monitor.getProjectIcon(id)
    }

    override fun getProjectStatus(url: String): String {
        return monitor.getProjectStatus(url)
    }

    override fun getRssNotices(): List<Notice> {
        return monitor.rssNotices
    }

    override fun getSlideshowForProject(url: String): List<ImageWrapper> {
        return monitor.getSlideshowForProject(url)
    }

    override fun setAutostart(isAutoStart: Boolean) {
        monitor.autostart = isAutoStart
    }

    override fun setShowNotificationForNotices(isShow: Boolean) {
        monitor.showNotificationForNotices = isShow
    }

    override fun getShowAdvanced(): Boolean {
        return monitor.showAdvanced
    }

    override fun getIsRemote(): Boolean {
        return monitor.isRemote
    }

    override fun getAutostart(): Boolean {
        return monitor.autostart
    }

    override fun getShowNotificationForNotices(): Boolean {
        return monitor.showNotificationForNotices
    }

    override fun getLogLevel(): Int {
        return monitor.logLevel
    }

    override fun setLogLevel(level: Int) {
        monitor.logLevel = level
    }

    override fun getLogCategories(): List<String> {
        return monitor.logCategories
    }

    override fun setLogCategories(categories: List<String>) {
        monitor.logCategories = categories
    }

    override fun setPowerSourceAc(src: Boolean) {
        monitor.powerSourceAc = src
    }

    override fun setPowerSourceUsb(src: Boolean) {
        monitor.powerSourceUsb = src
    }

    override fun setPowerSourceWireless(src: Boolean) {
        monitor.powerSourceWireless = src
    }

    override fun getStationaryDeviceMode(): Boolean {
        return monitor.stationaryDeviceMode
    }

    override fun getPowerSourceAc(): Boolean {
        return monitor.powerSourceAc
    }

    override fun getPowerSourceUsb(): Boolean {
        return monitor.powerSourceUsb
    }

    override fun getPowerSourceWireless(): Boolean {
        return monitor.powerSourceWireless
    }

    override fun setShowAdvanced(isShow: Boolean) {
        monitor.showAdvanced = isShow
    }

    override fun setIsRemote(isRemote: Boolean) {
        monitor.isRemote = isRemote
    }

    override fun setStationaryDeviceMode(mode: Boolean) {
        monitor.stationaryDeviceMode = mode
    }

    override fun getSuspendWhenScreenOn(): Boolean {
        return monitor.suspendWhenScreenOn
    }

    override fun setSuspendWhenScreenOn(swso: Boolean) {
        monitor.suspendWhenScreenOn = swso
    }
}
