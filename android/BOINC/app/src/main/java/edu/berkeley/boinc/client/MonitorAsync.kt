package edu.berkeley.boinc.client

import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.async

class MonitorAsync(val monitor :IMonitor?) {
    fun quitClientAsync(callback: ((Boolean) -> Unit)? = null) = GlobalScope.async {
        val result  = monitor?.quitClient()!!
        if (callback != null) {
            callback(result)
        }
        result
    }
}
