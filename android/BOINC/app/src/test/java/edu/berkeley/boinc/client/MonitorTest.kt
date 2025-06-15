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

import edu.berkeley.boinc.rpc.Transfer
import org.junit.After
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.*
import org.mockito.ArgumentMatchers.eq
import org.robolectric.Robolectric
import org.robolectric.RobolectricTestRunner
import org.robolectric.android.controller.ServiceController

@RunWith(RobolectricTestRunner::class)
class MonitorTest {
    @Mock
    private lateinit var clientStatus: ClientStatus

    @Spy
    @InjectMocks
    private lateinit var clientInterface: ClientInterfaceImplementation

    private lateinit var monitor: Monitor
    private lateinit var controller: ServiceController<Monitor>

    @Before
    fun setUp() {
        MockitoAnnotations.initMocks(this)
        controller = Robolectric.buildService(Monitor::class.java).create()
        monitor = controller.get()
        monitor.clientInterface = clientInterface
    }

    @Test
    fun `Expect default config location when getAuthFilePath() is called`() {
        Assert.assertTrue(monitor.authFilePath.endsWith("/client/gui_rpc_auth.cfg"))
    }

    @Test
    fun `Expect arm-android-linux-gnu when getBoincPlatform() is called`() {
        Assert.assertEquals("arm-android-linux-gnu", monitor.getString(monitor.boincPlatform))
    }

    @Test
    fun `Expect blank string when getBoincAltPlatform() is called`() {
        Assert.assertTrue(monitor.boincAltPlatform.isEmpty())
    }

//    TODO: Need to be fixed
//    @Test
//    fun `Expect ClientInterfaceImplementation's transferOperation() to be called when mBinding's transferOperation() is called`() {
//        monitor.mBinder.transferOperation(emptyList(), 1)
//
//        Mockito.verify(clientInterface).transferOperation(eq(emptyList<Transfer>())!!, eq(1))
//    }

    @Test(expected = NullPointerException::class)
    fun `Expect NullPointerException to be thrown when mBinder's setGlobalPreferences() is called with a null parameter`() {
        monitor.mBinder.setGlobalPreferences(null)
    }

    @Test(expected = NullPointerException::class)
    fun `Expect NullPointerException to be thrown when mBinder's readAuthToken() is called with a null parameter`() {
        monitor.mBinder.readAuthToken(null)
    }

    @Test
    fun `Expect empty string when mBinder's readAuthToken() is called with a non-existent path`() {
        Assert.assertTrue(monitor.mBinder.readAuthToken("somefile.txt").isEmpty())
    }

    @Test
    fun `Expect false when mBinder's projectOp() is called with an unsupported operation`() {
        Assert.assertFalse(monitor.mBinder.projectOp(0, ""))
    }

    @Test
    fun `Expect empty list when mBinder's getServerNotices() is called`() {
        Assert.assertTrue(monitor.mBinder.serverNotices.isEmpty())
    }

    @Test
    fun `Expect battery charge to be 0 when mBinder's getBatteryChargeStatus() is called`() {
        Assert.assertEquals(0, monitor.mBinder.batteryChargeStatus)
    }

    @Test
    fun `Expect BOINC mutex to not be acquired when mBinder's boincMutexAcquired() is called`() {
        Assert.assertFalse(monitor.mBinder.boincMutexAcquired())
    }

    @After
    fun tearDown() {
        controller.destroy()
    }
}

