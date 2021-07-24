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
package edu.berkeley.boinc.mutex

import android.net.LocalSocket
import android.util.Log
import edu.berkeley.boinc.utils.Logging.setLogLevel
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.ArgumentMatchers
import org.mockito.Mock
import org.mockito.Mockito
import org.mockito.MockitoAnnotations
import org.powermock.api.mockito.PowerMockito
import org.powermock.core.classloader.annotations.PrepareForTest
import org.powermock.modules.junit4.PowerMockRunner
import java.io.IOException

@RunWith(PowerMockRunner::class)
@PrepareForTest(Log::class)
class BoincMutexTest {
    @Mock
    private val localSocket: LocalSocket? = null
    private var boincMutex: BoincMutex? = null
    @Before
    fun setUp() {
        MockitoAnnotations.initMocks(this)
        boincMutex = BoincMutex(localSocket!!)
    }

    @Test
    fun `Acquire() Expect True when socket is already bound`() {
        Mockito.`when`(localSocket!!.isBound).thenReturn(true)
        Assert.assertTrue(boincMutex!!.acquire())
    }

    @Test
    fun `Acquire() Expect False when isBound() returns false`() {
        Mockito.`when`(localSocket!!.isBound).thenReturn(false)
        Assert.assertFalse(boincMutex!!.acquire())
    }

    @Test
    fun `Release() Expect isAcquired to be False`() {
        Mockito.`when`(localSocket!!.isBound).thenReturn(true)
        boincMutex!!.release()
        Assert.assertFalse(boincMutex!!.isAcquired)
    }
}
