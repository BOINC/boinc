package edu.berkeley.boinc.mutex;

import android.net.LocalSocket;
import android.util.Log;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.IOException;

import edu.berkeley.boinc.utils.Logging;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.when;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Log.class)
public class BoincMutexTest {
    @Mock
    private LocalSocket localSocket;

    private BoincMutex boincMutex;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);

        boincMutex = new BoincMutex(localSocket);
    }

    @Test
    public void testAcquire_expectTrueWhenSocketAlreadyBound() {
        when(localSocket.isBound()).thenReturn(true);

        assertTrue(boincMutex.acquire());
    }

    @Test
    public void testAcquire_expectFalseWhenIsBoundReturnsFalse() {
        when(localSocket.isBound()).thenReturn(false);

        assertFalse(boincMutex.acquire());
    }

    @Test
    public void testAcquire_expectIOExceptionThrownByBindToBeLoggedWhenLoggingLevelIsError() throws IOException {
        PowerMockito.mockStatic(Log.class);

        doThrow(new IOException()).when(localSocket).bind(any());
        Logging.setLogLevel(1);

        boincMutex.acquire();

        PowerMockito.verifyStatic(Log.class);
        Log.e(anyString(), eq("BoincMutex.acquire error: "), any());
    }

    @Test
    public void testRelease_expectIsAcquiredToBeFalse() {
        when(localSocket.isBound()).thenReturn(true);

        boincMutex.release();

        assertFalse(boincMutex.isAcquired());
    }

    @Test
    public void testRelease_expectIOExceptionThrownByCloseToBeLoggedWhenBoundAndLoggingLevelIsError()
            throws IOException {
        PowerMockito.mockStatic(Log.class);

        when(localSocket.isBound()).thenReturn(true);
        doThrow(new IOException()).when(localSocket).close();
        Logging.setLogLevel(1);

        boincMutex.release();

        PowerMockito.verifyStatic(Log.class);
        Log.e(anyString(), eq("BoincMutex.release error: "), any());
    }
}
