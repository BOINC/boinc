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
package edu.berkeley.boinc.utils

import android.util.Log
import io.mockk.Called
import io.mockk.confirmVerified
import io.mockk.every
import io.mockk.mockkStatic
import io.mockk.verify
import org.junit.Assert
import org.junit.Before
import org.junit.FixMethodOrder
import org.junit.Test
import org.junit.runners.MethodSorters

@FixMethodOrder(MethodSorters.NAME_ASCENDING)
class LoggingTest {
    @Before
    fun setUp() {
        mockkStatic(Log::class)
        every { Log.e(any(), any()) } returns 0
        every { Log.e(any(), any(), any()) } returns 0
        every { Log.w(any(), any<String>()) } returns 0
        every { Log.i(any(), any()) } returns 0
        every { Log.d(any(), any()) } returns 0
        every { Log.v(any(), any()) } returns 0
    }

    @Test
    fun `Test_01 Logging TAG`() {
        Assert.assertEquals("BOINC_GUI", Logging.TAG)
    }

    @Test
    fun `Test_02 Logging WAKELOCK`() {
        Assert.assertEquals("BOINC_GUI:MyPowerLock", Logging.WAKELOCK)
    }

    @Test
    fun `Test_03 Logging Default Log Levels`() {
        Assert.assertEquals(-1, Logging.getLogLevel())
        Assert.assertFalse(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))

        Logging.setLogCategory("DEVICE", true)

        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))

    }

    @Test
    fun `Test_04 Logging setLogLevel(-1)`() {
        Logging.setLogLevel(-1)
        Assert.assertEquals(-1, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_05 Logging setLogLevel(-10)`() {
        Logging.setLogLevel(-10)
        Assert.assertEquals(-10, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_06 Logging setLogLevel(-42)`() {
        Logging.setLogLevel(-42)
        Assert.assertEquals(-42, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_07 Logging setLogLevel(0)`() {
        Logging.setLogLevel(0)
        Assert.assertEquals(0, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_08 Logging setLogLevel(1)`() {
        Logging.setLogLevel(1)
        Assert.assertEquals(1, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_09 Logging setLogLevel(2)`() {
        Logging.setLogLevel(2)
        Assert.assertEquals(2, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_10 Logging setLogLevel(3)`() {
        Logging.setLogLevel(3)
        Assert.assertEquals(3, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_11 Logging setLogLevel(4)`() {
        Logging.setLogLevel(4)
        Assert.assertEquals(4, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_12 Logging setLogLevel(5)`() {
        Logging.setLogLevel(5)
        Assert.assertEquals(5, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_13 Logging setLogLevel(6)`() {
        Logging.setLogLevel(6)
        Assert.assertEquals(6, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_14 Logging setLogLevel(10)`() {
        Logging.setLogLevel(10)
        Assert.assertEquals(10, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_15 Logging setLogLevel(42)`() {
        Logging.setLogLevel(42)
        Assert.assertEquals(42, Logging.getLogLevel())
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertTrue(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test
    fun `Test_16 Logging after category remove`() {
        Logging.setLogCategory("DEVICE", false)

        Assert.assertFalse(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.ERROR, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.WARNING, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.INFO, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.DEBUG, Logging.Category.DEVICE))
        Assert.assertFalse(Logging.isLoggable(Logging.Level.VERBOSE, Logging.Category.DEVICE))
    }

    @Test(expected = Test.None::class)
    fun `Test_17 Logging not fail on double add or double remove`() {
        Logging.setLogCategory("RPC", true)
        Logging.setLogCategory("RPC", true)
        Logging.setLogCategory("RPC", false)
        Logging.setLogCategory("RPC", false)
    }

    @Test(expected = Test.None::class)
    fun `Test_18 Logging not fail when non existing category is provided`() {
        Logging.setLogCategory("TEST_CATEGORY", true)
        Logging.setLogCategory("TEST_CATEGORY", false)
    }

    @Test
    fun `Test_19 Logging after categories list set`() {
        Logging.setLogCategories(listOf("DEVICE", "RPC"))
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.DEVICE))
        Assert.assertTrue(Logging.getLogCategories().contains(Logging.Category.RPC))
        Logging.setLogCategory("DEVICE", false)
        Logging.setLogCategory("RPC", false)
    }

    @Test
    fun `Test_20 only error and exception are logged when logLevel equals ERROR`() {
        Logging.setLogLevel(Logging.Level.ERROR.logLevel)
        Logging.setLogCategory("DEVICE", true)

        Logging.logException(Logging.Category.DEVICE, "TestException", Exception("TestException"))
        Logging.logError(Logging.Category.DEVICE, "TestError")
        Logging.logWarning(Logging.Category.DEVICE, "TestWarning")
        Logging.logInfo(Logging.Category.DEVICE, "TestInfo")
        Logging.logDebug(Logging.Category.DEVICE, "TestDebug")
        Logging.logVerbose(Logging.Category.DEVICE, "TestVerbose")

        verify(exactly = 2) { Log.e(any(), any(), any()) }
        verify(exactly = 0) { Log.w(any(), any<String>()) }
        verify(exactly = 0)  { Log.i(any(), any()) }
        verify(exactly = 0)  { Log.d(any(), any()) }
        verify(exactly = 0)  { Log.v(any(), any()) }

        Logging.setLogCategory("DEVICE", false)
    }

    @Test
    fun `Test_21 only warning, error and exception are logged when logLevel equals WARNING`() {
        Logging.setLogLevel(Logging.Level.WARNING.logLevel)
        Logging.setLogCategory("DEVICE", true)

        Logging.logException(Logging.Category.DEVICE, "TestException", Exception("TestException"))
        Logging.logError(Logging.Category.DEVICE, "TestError")
        Logging.logWarning(Logging.Category.DEVICE, "TestWarning")
        Logging.logInfo(Logging.Category.DEVICE, "TestInfo")
        Logging.logDebug(Logging.Category.DEVICE, "TestDebug")
        Logging.logVerbose(Logging.Category.DEVICE, "TestVerbose")

        verify(exactly = 2) { Log.e(any(), any(), any()) }
        verify(exactly = 1) { Log.w(any(), any<String>()) }
        verify(exactly = 0)  { Log.i(any(), any()) }
        verify(exactly = 0)  { Log.d(any(), any()) }
        verify(exactly = 0)  { Log.v(any(), any()) }

        Logging.setLogCategory("DEVICE", false)
    }

    @Test
    fun `Test_22 only info, warning, error and exception are logged when logLevel equals INFO`() {
        Logging.setLogLevel(Logging.Level.INFO.logLevel)
        Logging.setLogCategory("DEVICE", true)

        Logging.logException(Logging.Category.DEVICE, "TestException", Exception("TestException"))
        Logging.logError(Logging.Category.DEVICE, "TestError")
        Logging.logWarning(Logging.Category.DEVICE, "TestWarning")
        Logging.logInfo(Logging.Category.DEVICE, "TestInfo")
        Logging.logDebug(Logging.Category.DEVICE, "TestDebug")
        Logging.logVerbose(Logging.Category.DEVICE, "TestVerbose")

        verify(exactly = 2) { Log.e(any(), any(), any()) }
        verify(exactly = 1) { Log.w(any(), any<String>()) }
        verify(exactly = 1)  { Log.i(any(), any()) }
        verify(exactly = 0)  { Log.d(any(), any()) }
        verify(exactly = 0)  { Log.v(any(), any()) }

        Logging.setLogCategory("DEVICE", false)
    }

    @Test
    fun `Test_23 only debug, info, warning, error and exception are logged when logLevel equals DEBUG`() {
        Logging.setLogLevel(Logging.Level.DEBUG.logLevel)
        Logging.setLogCategory("DEVICE", true)

        Logging.logException(Logging.Category.DEVICE, "TestException", Exception("TestException"))
        Logging.logError(Logging.Category.DEVICE, "TestError")
        Logging.logWarning(Logging.Category.DEVICE, "TestWarning")
        Logging.logInfo(Logging.Category.DEVICE, "TestInfo")
        Logging.logDebug(Logging.Category.DEVICE, "TestDebug")
        Logging.logVerbose(Logging.Category.DEVICE, "TestVerbose")

        verify(exactly = 2) { Log.e(any(), any(), any()) }
        verify(exactly = 1) { Log.w(any(), any<String>()) }
        verify(exactly = 1)  { Log.i(any(), any()) }
        verify(exactly = 1)  { Log.d(any(), any()) }
        verify(exactly = 0)  { Log.v(any(), any()) }

        Logging.setLogCategory("DEVICE", false)
    }

    @Test
    fun `Test_24 verbose, debug, info, warning, error and exception are logged when logLevel equals VERBOSE`() {
        Logging.setLogLevel(Logging.Level.VERBOSE.logLevel)
        Logging.setLogCategory("DEVICE", true)

        Logging.logException(Logging.Category.DEVICE, "TestException", Exception("TestException"))
        Logging.logError(Logging.Category.DEVICE, "TestError")
        Logging.logWarning(Logging.Category.DEVICE, "TestWarning")
        Logging.logInfo(Logging.Category.DEVICE, "TestInfo")
        Logging.logDebug(Logging.Category.DEVICE, "TestDebug")
        Logging.logVerbose(Logging.Category.DEVICE, "TestVerbose")

        verify(exactly = 2) { Log.e(any(), any(), any()) }
        verify(exactly = 1) { Log.w(any(), any<String>()) }
        verify(exactly = 1)  { Log.i(any(), any()) }
        verify(exactly = 1)  { Log.d(any(), any()) }
        verify(exactly = 1)  { Log.v(any(), any()) }

        Logging.setLogCategory("DEVICE", false)
    }
}
