/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2026 University of California
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

import android.app.Service
import android.content.Intent
import edu.berkeley.boinc.mutex.BoincMutex
import io.mockk.every
import io.mockk.justRun
import io.mockk.mockk
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.Robolectric
import org.robolectric.RobolectricTestRunner
import org.robolectric.Shadows.shadowOf
import org.robolectric.android.controller.ServiceController
import org.robolectric.annotation.Config
import org.robolectric.util.ReflectionHelpers
import java.util.Timer
import java.util.TimerTask

@RunWith(RobolectricTestRunner::class)
@Config(sdk = [26, 28])
class MonitorLifecycleTest {
    private lateinit var controller: ServiceController<Monitor>
    private lateinit var monitor: Monitor
    private lateinit var notification: ClientNotification
    private val periodicTasks = mutableListOf<TimerTask>()
    private val refreshTasks = mutableListOf<TimerTask>()
    private var allowMutex = true
    private var acquired = false

    @Before
    fun setUp() {
        controller = Robolectric.buildService(Monitor::class.java).create()
        monitor = controller.get()
        notification = ClientNotification(monitor)
        monitor.clientNotification = notification

        val mutex = mockk<BoincMutex>()
        every { mutex.acquire() } answers {
            // Foreground promotion must happen before setup can block or fail.
            assertTrue(notification.foreground)
            assertNotNull(shadowOf(monitor).lastForegroundNotification)
            acquired = allowMutex
            acquired
        }
        every { mutex.isAcquired } answers { acquired }
        justRun { mutex.release() }
        monitor.mutex = mutex

        monitor.clientInterface = mockk(relaxed = true)

        // Observe scheduling without starting the native client or a timer thread.
        val original: Timer = ReflectionHelpers.getField(monitor, "updateTimer")
        original.cancel()
        val timer = mockk<Timer>(relaxed = true)
        every { timer.schedule(capture(periodicTasks), 0L, any<Long>()) } returns Unit
        every { timer.schedule(capture(refreshTasks), 0L) } returns Unit
        ReflectionHelpers.setField(monitor, "updateTimer", timer)
    }

    private fun start(id: Int = 1) {
        assertEquals(Service.START_STICKY, monitor.onStartCommand(Intent(), 0, id))
    }

    @Test
    fun `mutex failure stops foreground and a later start retries successfully`() {
        allowMutex = false
        start()
        assertFalse(notification.foreground)
        assertTrue(periodicTasks.isEmpty())

        allowMutex = true
        start(2)
        assertTrue(notification.foreground)
        assertEquals(1, periodicTasks.size)
    }

    @Test
    fun `repeated starts refresh status without scheduling another periodic task`() {
        start()
        start(2)
        start(3)
        assertEquals(1, periodicTasks.size)
        assertEquals(2, refreshTasks.size)
        assertTrue(notification.foreground)
        assertNotNull(shadowOf(monitor).lastForegroundNotification)
    }

    @After
    fun tearDown() {
        controller.destroy()
    }
}
