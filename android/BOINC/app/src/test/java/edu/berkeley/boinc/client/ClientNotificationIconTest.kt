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
package edu.berkeley.boinc.client

import android.os.Build
import androidx.test.core.app.ApplicationProvider.getApplicationContext
import edu.berkeley.boinc.R
import org.junit.Assert
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
class ClientNotificationIconTest {
    private lateinit var clientNotification: ClientNotification

    @Before
    fun setUp() {
        clientNotification = ClientNotification(getApplicationContext())
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_paused_white when ClientStatus is COMPUTING_STATUS_NEVER, isSmall is true and API lower than 21`() {
        Assert.assertEquals(R.mipmap.ic_boinc_paused_white, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_NEVER, true))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_paused_white when ClientStatus is COMPUTING_STATUS_SUSPENDED, isSmall is true and API lower than 21`() {
        Assert.assertEquals(R.mipmap.ic_boinc_paused_white, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_SUSPENDED, true))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_paused_white when ClientStatus is COMPUTING_STATUS_IDLE, isSmall is true and API lower than 21`() {
        Assert.assertEquals(R.mipmap.ic_boinc_paused_white, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_IDLE, true))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_NEVER, isSmall is false and API lower than 21`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_NEVER, false))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_SUSPENDED, isSmall is false and API lower than 21`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_SUSPENDED, false))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_IDLE, isSmall is false and API lower than 21`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_IDLE, false))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_NEVER, isSmall is true and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_NEVER, true))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_SUSPENDED, isSmall is true and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_SUSPENDED, true))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_IDLE, isSmall is true and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_IDLE, true))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_NEVER, isSmall is false and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_NEVER, false))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_SUSPENDED, isSmall is false and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_SUSPENDED, false))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_paused when ClientStatus is COMPUTING_STATUS_IDLE, isSmall is false and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc_paused, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_IDLE, false))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_boinc_white when ClientStatus is COMPUTING_STATUS_COMPUTING, isSmall is true and API lower than 21`() {
        Assert.assertEquals(R.mipmap.ic_boinc_white, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_COMPUTING, true))
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.KITKAT_WATCH)
    @Test
    fun `Expect icon_boinc when ClientStatus is COMPUTING_STATUS_COMPUTING, isSmall is false and API lower than 21`() {
        Assert.assertEquals(R.drawable.ic_boinc, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_COMPUTING, false))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_boinc when ClientStatus is COMPUTING_STATUS_COMPUTING, isSmall is true and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_COMPUTING, true))
    }

    @Config(minSdk = Build.VERSION_CODES.LOLLIPOP, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect icon_boinc when ClientStatus is COMPUTING_STATUS_COMPUTING, isSmall is false and API is 21 or higher`() {
        Assert.assertEquals(R.drawable.ic_boinc, clientNotification.getIcon(ClientStatus.COMPUTING_STATUS_COMPUTING, false))
    }
}
