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

import org.junit.jupiter.api.*

@TestMethodOrder(MethodOrderer.OrderAnnotation::class)
class LoggingTest {
    @Test
    @Order(1)
    fun `Test Logging TAG`() {
        Assertions.assertEquals("BOINC_GUI", Logging.TAG)
    }

    @Test
    @Order(2)
    fun `Test Logging WAKELOCK`() {
        Assertions.assertEquals("BOINC_GUI:MyPowerLock", Logging.WAKELOCK)
    }

    @Test
    @Order(3)
    fun `Test Logging Default Log Levels`() {
        Assertions.assertEquals(-1, Logging.LOGLEVEL)
        Assertions.assertFalse(Logging.ERROR)
        Assertions.assertFalse(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(4)
    fun `Test Logging setLogLevel(-1)`() {
        Logging.setLogLevel(-1)
        Assertions.assertEquals(-1, Logging.LOGLEVEL)
        Assertions.assertFalse(Logging.ERROR)
        Assertions.assertFalse(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(5)
    fun `Test Logging setLogLevel(-10)`() {
        Logging.setLogLevel(-10)
        Assertions.assertEquals(-10, Logging.LOGLEVEL)
        Assertions.assertFalse(Logging.ERROR)
        Assertions.assertFalse(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(6)
    fun `Test Logging setLogLevel(-42)`() {
        Logging.setLogLevel(-42)
        Assertions.assertEquals(-42, Logging.LOGLEVEL)
        Assertions.assertFalse(Logging.ERROR)
        Assertions.assertFalse(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(7)
    fun `Test Logging setLogLevel(0)`() {
        Logging.setLogLevel(0)
        Assertions.assertEquals(0, Logging.LOGLEVEL)
        Assertions.assertFalse(Logging.ERROR)
        Assertions.assertFalse(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(8)
    fun `Test Logging setLogLevel(1)`() {
        Logging.setLogLevel(1)
        Assertions.assertEquals(1, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertFalse(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(9)
    fun `Test Logging setLogLevel(2)`() {
        Logging.setLogLevel(2)
        Assertions.assertEquals(2, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertFalse(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(10)
    fun `Test Logging setLogLevel(3)`() {
        Logging.setLogLevel(3)
        Assertions.assertEquals(3, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertTrue(Logging.INFO)
        Assertions.assertFalse(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(11)
    fun `Test Logging setLogLevel(4)`() {
        Logging.setLogLevel(4)
        Assertions.assertEquals(4, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertTrue(Logging.INFO)
        Assertions.assertTrue(Logging.DEBUG)
        Assertions.assertFalse(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(12)
    fun `Test Logging setLogLevel(5)`() {
        Logging.setLogLevel(5)
        Assertions.assertEquals(5, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertTrue(Logging.INFO)
        Assertions.assertTrue(Logging.DEBUG)
        Assertions.assertTrue(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(13)
    fun `Test Logging setLogLevel(6)`() {
        Logging.setLogLevel(6)
        Assertions.assertEquals(6, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertTrue(Logging.INFO)
        Assertions.assertTrue(Logging.DEBUG)
        Assertions.assertTrue(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(14)
    fun `Test Logging setLogLevel(10)`() {
        Logging.setLogLevel(10)
        Assertions.assertEquals(10, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertTrue(Logging.INFO)
        Assertions.assertTrue(Logging.DEBUG)
        Assertions.assertTrue(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }

    @Test
    @Order(15)
    fun `Test Logging setLogLevel(42)`() {
        Logging.setLogLevel(42)
        Assertions.assertEquals(42, Logging.LOGLEVEL)
        Assertions.assertTrue(Logging.ERROR)
        Assertions.assertTrue(Logging.WARNING)
        Assertions.assertTrue(Logging.INFO)
        Assertions.assertTrue(Logging.DEBUG)
        Assertions.assertTrue(Logging.VERBOSE)
        Assertions.assertFalse(Logging.RPC_PERFORMANCE)
        Assertions.assertFalse(Logging.RPC_DATA)
    }
}