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
import android.content.Context
import androidx.test.core.app.ApplicationProvider
import edu.berkeley.boinc.rpc.DeviceStatusData
import edu.berkeley.boinc.rpc.Notice
import io.mockk.mockkClass
import org.hamcrest.CoreMatchers.instanceOf
import org.hamcrest.MatcherAssert.assertThat
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner

@RunWith(RobolectricTestRunner::class)
class NoticeNotificationTest {
    private lateinit var context: Context
    private lateinit var noticeNotification: NoticeNotification
    private lateinit var clientStatus: ClientStatus
    private lateinit var appPreferences: AppPreferences
    private lateinit var deviceStatus: DeviceStatus
    private lateinit var deviceStatusData: DeviceStatusData

    @Before
    fun setUp() {
        context = ApplicationProvider.getApplicationContext()
        appPreferences = mockkClass(AppPreferences::class)
        deviceStatusData = DeviceStatusData()
        deviceStatus = DeviceStatus(context, appPreferences, deviceStatusData)
        clientStatus = ClientStatus(context, appPreferences, deviceStatus)
        val persistentStorage = PersistentStorage(ApplicationProvider.getApplicationContext())
        noticeNotification = NoticeNotification(ApplicationProvider.getApplicationContext(), clientStatus, persistentStorage)
        noticeNotification.currentlyNotifiedNotices.add(Notice(projectName = "testProject"))
    }

    @Test
    fun `Check update() returns not null`() {
        val notices = listOf<Notice>()
        val isPreferenceEnabled = true
        Assert.assertNotNull(noticeNotification.update(notices, isPreferenceEnabled))
        Assert.assertNotNull(noticeNotification.update(notices, false))
    }

    @Test
    fun `Check buildNoticeNotification() returns List of Notification`() {
        assertThat(noticeNotification.buildNoticeNotifications(), instanceOf(List::class.java))
    }

    @Test
    fun `Check buildSummaryNotification() returns Notification`() {
        assertThat(noticeNotification.buildSummaryNotification(), instanceOf(Notification::class.java))
    }

    @Test
    fun `Check getLargeProjectIcon() returns Bitmap`() {
        assertThat(noticeNotification.getLargeProjectIcon(ApplicationProvider.getApplicationContext(), "testProject"), instanceOf(android.graphics.Bitmap::class.java))
    }
}
