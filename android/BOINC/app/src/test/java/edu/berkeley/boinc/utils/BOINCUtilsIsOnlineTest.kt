package edu.berkeley.boinc.utils

import android.net.ConnectivityManager
import android.os.Build
import androidx.core.content.getSystemService
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import org.mockito.Mockito
import org.mockito.MockitoAnnotations
import org.mockito.Spy
import org.robolectric.RobolectricTestRunner
import org.robolectric.annotation.Config

@RunWith(RobolectricTestRunner::class)
class BOINCUtilsIsOnlineTest {
    @Spy
    private val connectivityManager = InstrumentationRegistry.getInstrumentation().context
            .getSystemService<ConnectivityManager>()!!

    @Before
    fun setUp() {
        MockitoAnnotations.initMocks(this)
    }

    @Config(minSdk = Build.VERSION_CODES.JELLY_BEAN, maxSdk = Build.VERSION_CODES.LOLLIPOP_MR1)
    @Test
    fun `Expect isOnline property to call getActiveNetworkInfo() when API level is below 23`() {
        connectivityManager.isOnline
        @Suppress("DEPRECATION")
        Mockito.verify(connectivityManager).activeNetworkInfo
    }

    @Config(minSdk = Build.VERSION_CODES.M, maxSdk = Build.VERSION_CODES.P)
    @Test
    fun `Expect isOnline property to call getActiveNetwork() when API level is 23 or higher`() {
        connectivityManager.isOnline

        Mockito.verify(connectivityManager).activeNetwork
    }
}
