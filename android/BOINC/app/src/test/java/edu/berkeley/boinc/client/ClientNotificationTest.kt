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

import android.app.Notification
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.os.Build
import androidx.test.core.app.ApplicationProvider
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpc.*
import edu.berkeley.boinc.utils.PROCESS_ABORTED
import edu.berkeley.boinc.utils.PROCESS_EXECUTING
import edu.berkeley.boinc.utils.PROCESS_SUSPENDED
import io.mockk.every
import io.mockk.justRun
import io.mockk.mockkClass
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.jupiter.api.assertDoesNotThrow
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
class ClientNotificationTest {
    private lateinit var context: Context
    private lateinit var clientNotification: ClientNotification
    private lateinit var appPreferences: AppPreferences
    private lateinit var deviceStatus: DeviceStatus
    private lateinit var deviceStatusData: DeviceStatusData

    @Before
    fun setUp() {
        context = ApplicationProvider.getApplicationContext()
        clientNotification = ClientNotification(context)
        appPreferences = mockkClass(AppPreferences::class)
        deviceStatusData = DeviceStatusData()

        deviceStatus = DeviceStatus(context, appPreferences, deviceStatusData)
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_NEVER then expect corresponding Notification title and text`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(clientStatus.currentStatusTitle, notification.extras.getString(Notification.EXTRA_TITLE))
        Assert.assertEquals(clientStatus.currentStatusDescription, notification.extras.getString(Notification.EXTRA_TEXT))
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_IDLE then expect corresponding Notification title and text`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(clientStatus.currentStatusTitle, notification.extras.getString(Notification.EXTRA_TITLE))
        Assert.assertEquals(clientStatus.currentStatusDescription, notification.extras.getString(Notification.EXTRA_TEXT))
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_SUSPENDED then expect corresponding Notification title and text`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(clientStatus.currentStatusTitle, notification.extras.getString(Notification.EXTRA_TITLE))
        Assert.assertEquals(clientStatus.currentStatusDescription, notification.extras.getString(Notification.EXTRA_TEXT))
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_COMPUTING and activeTasks is null then expect corresponding Notification title and text`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(clientStatus.currentStatusTitle, notification.extras.getString(Notification.EXTRA_TITLE))
        Assert.assertEquals(clientStatus.currentStatusDescription, notification.extras.getString(Notification.EXTRA_TEXT))
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_COMPUTING and activeTasks is empty then expect corresponding Notification title and text`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING

        val notification = clientNotification.buildNotification(clientStatus, true, listOf())
        Assert.assertEquals(clientStatus.currentStatusTitle, notification.extras.getString(Notification.EXTRA_TITLE))
        Assert.assertEquals(clientStatus.currentStatusDescription, notification.extras.getString(Notification.EXTRA_TEXT))
    }


    @Test
    fun `When ClientStatus is COMPUTING_STATUS_COMPUTING and activeTasks contains records then expect corresponding Notification title and text`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING

        val activeTasks = listOf(
            Result(project = Project(projectName = "Project 1"), app = App(name = "App Name 1")),
            Result(project = Project(projectName = "Project 2"), app = App(name = "App Name 2")),
        )

        val notification = clientNotification.buildNotification(clientStatus, true, activeTasks)
        Assert.assertEquals(clientStatus.currentStatusTitle, notification.extras.getString(Notification.EXTRA_TITLE))
        Assert.assertEquals(clientStatus.currentStatusDescription, notification.extras.getString(Notification.EXTRA_SUB_TEXT))
        Assert.assertEquals(activeTasks.size, notification.number)
        val lines = notification.extras.getCharSequenceArray(Notification.EXTRA_TEXT_LINES)
        Assert.assertNotNull(lines)
        Assert.assertEquals(activeTasks.size, lines?.size)
        Assert.assertEquals("Project 1: App Name 1", lines?.get(0)?.toString())
        Assert.assertEquals("Project 2: App Name 2", lines?.get(1)?.toString())
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_COMPUTING and activeTasks contains records with Project equal null or App equal null then expect no exception thrown`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING

        val activeTasks = listOf(
            Result(app = App(name = "App Name 1")),
            Result(project = Project(projectName = "Project 2")),
            Result()
        )

        val notification = clientNotification.buildNotification(clientStatus, true, activeTasks)
        Assert.assertEquals(activeTasks.size, notification.number)
        val lines = notification.extras.getCharSequenceArray(Notification.EXTRA_TEXT_LINES)
        Assert.assertNotNull(lines)
        Assert.assertEquals(activeTasks.size, lines?.size)
        Assert.assertEquals(": App Name 1", lines?.get(0)?.toString())
        Assert.assertEquals("Project 2: ", lines?.get(1)?.toString())
        Assert.assertEquals(": ", lines?.get(2)?.toString())
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_NEVER then expect only one Notification Action with menu_run_mode_enable title`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(1, notification.actions.size)
        Assert.assertEquals(context.getString(R.string.menu_run_mode_enable), notification.actions[0].title)
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_IDLE then expect only one Notification Action with menu_run_mode_disable title`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(1, notification.actions.size)
        Assert.assertEquals(context.getString(R.string.menu_run_mode_disable), notification.actions[0].title)
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_SUSPENDED then expect only one Notification Action with menu_run_mode_disable title`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(1, notification.actions.size)
        Assert.assertEquals(context.getString(R.string.menu_run_mode_disable), notification.actions[0].title)
    }

    @Test
    fun `When ClientStatus is COMPUTING_STATUS_COMPUTING then expect only one Notification Action with menu_run_mode_disable title`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        Assert.assertEquals(1, notification.actions.size)
        Assert.assertEquals(context.getString(R.string.menu_run_mode_disable), notification.actions[0].title)
    }

    @Test
    fun `When active is true then expect Notification priority to be high`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        val notification = clientNotification.buildNotification(clientStatus, true, null)
        @Suppress("DEPRECATION")
        Assert.assertEquals(Notification.PRIORITY_HIGH, notification.priority)
    }

    @Test
    fun `When active is false then expect Notification priority to be low`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        val notification = clientNotification.buildNotification(clientStatus, false, null)
        @Suppress("DEPRECATION")
        Assert.assertEquals(Notification.PRIORITY_LOW, notification.priority)
    }

    @Test
    fun `When foreground is false then expect it to be true after setForeground call`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }

        clientNotification.setForegroundState(monitor)
        Assert.assertTrue(clientNotification.foreground)
    }

    @Test
    fun `When updateStatus is null then expect no exception thrown`() {
        val monitor = mockkClass(Monitor::class)
        assertDoesNotThrow { clientNotification.update(null, monitor, true) }
    }

    @Test
    fun `When service is null then expect no exception thrown`() {
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER
        assertDoesNotThrow { clientNotification.update(clientStatus, null, true) }
    }

    @Test
    fun `When updateStatus is null and service is null then expect no exception thrown`() {
        assertDoesNotThrow { clientNotification.update(null, null, true) }
    }

    @Test
    fun `When active is true then expect foreground to be true`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.M)
    @Test
    fun `When active is false and showNotificationDuringSuspend is true then expect foreground to be false when API 23 or lower`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyBoolean: (Boolean) -> Boolean = { value -> value }
        @Suppress("DEPRECATION")
        justRun { monitor.stopForeground(anyBoolean(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.M)
    @Test
    fun `When active is false and showNotificationDuringSuspend is false then expect foreground to be false when API 23 or lower`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyBoolean: (Boolean) -> Boolean = { value -> value }
        @Suppress("DEPRECATION")
        justRun { monitor.stopForeground(anyBoolean(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.N, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `When active is false and showNotificationDuringSuspend is true then expect foreground to be false when API is higher than 23`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyInteger: (Int) -> Int = { value -> value }
        justRun { monitor.stopForeground(anyInteger(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.N, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `When active is false and showNotificationDuringSuspend is false then expect foreground to be false when API is higher than 23`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyInteger: (Int) -> Int = { value -> value }
        justRun { monitor.stopForeground(anyInteger(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.M)
    @Test
    fun `When active is false, showNotificationDuringSuspend is true and foreground is true then expect foreground to be false when API 23 or lower`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyBoolean: (Boolean) -> Boolean = { value -> value }
        @Suppress("DEPRECATION")
        justRun { monitor.stopForeground(anyBoolean(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.foreground = true
        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.M)
    @Test
    fun `When active is false, showNotificationDuringSuspend is false and foreground is true then expect foreground to be false when API 23 or lower`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyBoolean: (Boolean) -> Boolean = { value -> value }
        @Suppress("DEPRECATION")
        justRun { monitor.stopForeground(anyBoolean(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.foreground = true
        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.N, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `When active is false, showNotificationDuringSuspend is true and foreground is true then expect foreground to be false when API is higher than 23`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyInteger: (Int) -> Int = { value -> value }
        justRun { monitor.stopForeground(anyInteger(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.foreground = true
        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Config(minSdk = Build.VERSION_CODES.N, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `When active is false, showNotificationDuringSuspend is false and foreground is true then expect foreground to be false when API is higher than 23`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val anyInteger: (Int) -> Int = { value -> value }
        justRun { monitor.stopForeground(anyInteger(any())) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.foreground = true
        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.foreground)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_NEVER then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_IDLE then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_SUSPENDED then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_COMPUTING and executingTasks is empty then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_IDLE then expect status updated and oldActiveTasks to be empty`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_SUSPENDED then expect status updated and oldActiveTasks to be empty`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_NEVER then expect status updated and oldActiveTasks to be empty`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_COMPUTING and executingTasks is not empty then expect status updated and oldActiveTasks to be equal to executingTasks`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING
        clientStatus.setClientStatus(
            CcStatus(),
            listOf(
                Result(name = "Result 1", isActiveTask = true, activeTaskState = PROCESS_EXECUTING)
            ),
            listOf(Project()),
            listOf(Transfer()),
            HostInfo(),
            AcctMgrInfo(),
            listOf(Notice())
        )

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(1, clientNotification.mOldActiveTasks.size)
        Assert.assertEquals("Result 1", clientNotification.mOldActiveTasks[0].name)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_COMPUTING and executingTasks contains both running and not running tasks then expect status updated and oldActiveTasks to be equal to executingTasks`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING
        clientStatus.setClientStatus(
            CcStatus(),
            listOf(
                Result(name = "Result 1", isActiveTask = true, activeTaskState = PROCESS_EXECUTING),
                Result(name = "Result 2", isActiveTask = false, activeTaskState = PROCESS_SUSPENDED),
                Result(name = "Result 3", isActiveTask = false, activeTaskState = PROCESS_EXECUTING),
                Result(name = "Result 4", isActiveTask = true, activeTaskState = PROCESS_ABORTED),
            ),
            listOf(Project()),
            listOf(Transfer()),
            HostInfo(),
            AcctMgrInfo(),
            listOf(Notice())
        )

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(1, clientNotification.mOldActiveTasks.size)
        Assert.assertEquals("Result 1", clientNotification.mOldActiveTasks[0].name)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_COMPUTING and executingTasks contains not running tasks then expect status updated and oldActiveTasks to be empty`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING
        clientStatus.setClientStatus(
            CcStatus(),
            listOf(
                Result(name = "Result 1")
            ),
            listOf(Project()),
            listOf(Transfer()),
            HostInfo(),
            AcctMgrInfo(),
            listOf(Notice())
        )

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_COMPUTING, and executingTasks and oldActiveTasks are not empty then expect status updated and oldActiveTasks to be equal to executingTasks`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING
        clientStatus.setClientStatus(
            CcStatus(),
            listOf(
                Result(name = "Result 1", isActiveTask = true, activeTaskState = PROCESS_EXECUTING)
            ),
            listOf(Project()),
            listOf(Transfer()),
            HostInfo(),
            AcctMgrInfo(),
            listOf(Notice())
        )
        clientNotification.mOldActiveTasks.add(Result(name = "Result 2"))

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(1, clientNotification.mOldActiveTasks.size)
        Assert.assertEquals("Result 1", clientNotification.mOldActiveTasks[0].name)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_COMPUTING, and executingTasks and oldActiveTasks contain different number of records then expect status updated and lists to be equal`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_COMPUTING
        clientStatus.setClientStatus(
            CcStatus(),
            listOf(
                Result(name = "Result 1", isActiveTask = true, activeTaskState = PROCESS_EXECUTING)
            ),
            listOf(Project()),
            listOf(Transfer()),
            HostInfo(),
            AcctMgrInfo(),
            listOf(Notice())
        )
        clientNotification.mOldActiveTasks.add(Result(name = "Result 2"))
        clientNotification.mOldActiveTasks.add(Result(name = "Result 1"))

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(1, clientNotification.mOldActiveTasks.size)
        Assert.assertEquals("Result 1", clientNotification.mOldActiveTasks[0].name)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_IDLE, active is true and showNotificationDuringSuspend is true then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_IDLE, active is true and showNotificationDuringSuspend is false then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_IDLE, active is false and showNotificationDuringSuspend is true then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_IDLE, active is false and showNotificationDuringSuspend is false then expect notification is hidden`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_IDLE
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.notificationShown)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_SUSPENDED, active is true and showNotificationDuringSuspend is true then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_SUSPENDED, active is true and showNotificationDuringSuspend is false then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_SUSPENDED, active is false and showNotificationDuringSuspend is true then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_SUSPENDED, active is false and showNotificationDuringSuspend is false then expect notification is hidden`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_SUSPENDED
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.notificationShown)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_NEVER, active is true and showNotificationDuringSuspend is true then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_NEVER, active is true and showNotificationDuringSuspend is false then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, true)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_NEVER, active is false and showNotificationDuringSuspend is true then expect status updated`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns true

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertTrue(clientNotification.notificationShown)
        Assert.assertEquals(clientStatus.computingStatus, clientNotification.mOldComputingStatus)
        Assert.assertEquals(clientStatus.computingSuspendReason, clientNotification.mOldSuspendReason)
    }

    @Test
    fun `When updatedStatus is COMPUTING_STATUS_NEVER, active is false and showNotificationDuringSuspend is false then expect notification is hidden`() {
        val monitor = mockkClass(Monitor::class)
        justRun { monitor.startForeground(any(), any()) }
        every { monitor.appPreferences } returns appPreferences
        every { appPreferences.showNotificationDuringSuspend } returns false

        val clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        clientStatus.computingStatus = ClientStatus.COMPUTING_STATUS_NEVER
        clientNotification.mOldActiveTasks.add(Result())

        clientNotification.update(clientStatus, monitor, false)

        Assert.assertFalse(clientNotification.notificationShown)
    }

    @Test
    fun `When ClientNotification is created then expect default values to be set`() {
        Assert.assertEquals(-1, clientNotification.mOldComputingStatus)
        Assert.assertEquals(-1, clientNotification.mOldSuspendReason)
        Assert.assertTrue(clientNotification.mOldActiveTasks.isEmpty())
        Assert.assertFalse(clientNotification.notificationShown)
        Assert.assertFalse(clientNotification.foreground)
    }

    @Test
    fun `Check equality of Intent constants`() {
        Assert.assertEquals(Intent.FLAG_ACTIVITY_CLEAR_TOP, PendingIntent.FLAG_IMMUTABLE)
        Assert.assertEquals(Intent.FLAG_ACTIVITY_NEW_TASK, PendingIntent.FLAG_CANCEL_CURRENT)
    }
}
