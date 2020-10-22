package edu.berkeley.boinc.rpcExtern.fragment

import android.content.*
import android.net.wifi.WifiManager
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.edit
import androidx.preference.*
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpcExtern.RpcSettingsData
import java.net.InetAddress
import java.net.UnknownHostException
import java.nio.ByteBuffer
import java.nio.ByteOrder


class SettingsFragmentRpcExtern : PreferenceFragmentCompat() , SharedPreferences.OnSharedPreferenceChangeListener {
    var mLocalIp = ""
    val mRpcExternBroadCastReceiver = object : BroadcastReceiver() {
        override fun onReceive(contxt: Context?, intent: Intent?) {
            try {
                var statusMsg = ""
                val connectionStatus: String? = intent!!.getStringExtra("data")
                val connectionIp: String? = intent.getStringExtra("ip")
                when (connectionStatus)
                {
                    "IPNOT" -> statusMsg = getString(R.string.status_rpc_extern_ip_not_allowed) + " Ip: " + connectionIp
                    "WIFI_NOT_ALLOWED" -> statusMsg = getString(R.string.status_rpc_extern_wifi_ip_not_allowed) + " Ip: " + connectionIp
                    "CON_NOT_AUTH" -> statusMsg = getString(R.string.status_rpc_extern_password)
                    "CON_OK" -> statusMsg = getString(R.string.status_rpc_extern_connected) + " Ip: " + connectionIp
                    "CON_NOT" -> statusMsg = getString(R.string.status_rpc_extern_not_connected)
                    "TIMEOUT" -> statusMsg = getString(R.string.status_rpc_extern_not_connected_time)
                    "CON_ASK_MD5_GET_AES" -> statusMsg = getString(R.string.status_rpc_extern_use_hash)
                    "CON_ASK_AES_GET_MD5" -> statusMsg = getString(R.string.status_rpc_extern_use_encrypted)
                    "CONIPMATCH" -> statusMsg = getString(R.string.status_rpc_extern_ip_mismatch) + " Ip: " + connectionIp
                    "IDLE" -> statusMsg = getString(R.string.status_rpc_extern_idle)
                    "NOWIFI" -> statusMsg = getString(R.string.status_rpc_extern_no_wifi)
                    "CLOSING" -> statusMsg = getString(R.string.status_rpc_extern_closing)
                    "START" -> statusMsg = getString(R.string.status_rpc_extern_starting)
                }
                val prefTitle = findPreference("rpcExternTitle") as PreferenceCategory?
                val prefStatus = findPreference("rpcExternStatus") as PreferenceCategory?
                var title : String = getString(R.string.rpcExternTitle)
                val status = getString(R.string.rpcExternStatus) + " " + statusMsg
                mLocalIp = localIpAddress()
                title += mLocalIp
                prefTitle!!.title = title
                prefStatus!!.title = status
            } catch (e: Exception)
            {
            }
        }
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        setPreferencesFromResource(R.xml.settings_rpc_extern, rootKey)
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)

        mLocalIp = localIpAddress()

        try {
            // add IP to preferences
            val prefCat = findPreference("rpcExternTitle") as PreferenceCategory?
            var title: String = getString(R.string.rpcExternTitle)
            title += mLocalIp
            prefCat!!.title = title

            val button = findPreference("rpcExternAllowWfIpSet") as Preference?
            title = getString(R.string.settings_rpc_set_wifi_button)
            title += mLocalIp
            button!!.title = title
        } catch (e: Exception)
        {

        }
    }

    override fun onResume() {
        super.onResume()
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)
        BOINCActivity.appContext?.registerReceiver(mRpcExternBroadCastReceiver, IntentFilter("RPC_EXTERN_FROM_CONNECTION"))
        sendToServiceString("START_UPDATE")
    }

    override fun onPause() {
        super.onPause()
        preferenceManager.sharedPreferences.unregisterOnSharedPreferenceChangeListener(this)
        BOINCActivity.appContext?.unregisterReceiver(mRpcExternBroadCastReceiver)
        sendToServiceString("STOP_UPDATE")
    }

    override fun onDestroy() {
        super.onDestroy()
//        BOINCActivity.appContext?.unregisterReceiver(mRpcExternBroadCastReceiver) not here already done in onPause
    }

    fun sendStatus(status: String?)
    {
        val prefCat = findPreference("rpcExternTitle") as PreferenceCategory?
        prefCat!!.title = status
    }

    override fun onPreferenceTreeClick(preference: Preference): Boolean {
        try {
            if (preference.key == "rpcExternAllowWfIpSet") {
                val sp = getPreferenceManager().getSharedPreferences()
                // set any empty one to local IP if it's not there already
                // first find a match
                // the match from the start, an entry 192.168.10.7 matches 192.168.10.70
                if (addMatch(sp,"rpcExternAllowWfIp1", mLocalIp)) return true
                if (addMatch(sp,"rpcExternAllowWfIp2", mLocalIp)) return true
                if (addMatch(sp,"rpcExternAllowWfIp3", mLocalIp)) return true
                if (addMatch(sp,"rpcExternAllowWfIp4", mLocalIp)) return true
                // no match, so add it to an empty one
                if (addMatch(sp,"rpcExternAllowWfIp1", mLocalIp,true)) return true
                if (addMatch(sp,"rpcExternAllowWfIp2", mLocalIp,true)) return true
                if (addMatch(sp,"rpcExternAllowWfIp3", mLocalIp,true)) return true
                if (addMatch(sp,"rpcExternAllowWfIp4", mLocalIp, true)) return true
            }
        } catch (e : Exception)
        {
        }
        return false
    }

    fun addMatch(sp : SharedPreferences, key: String, wifi: String, add: Boolean = false) : Boolean {
        try {
            val ip = sp.getString(key, "")
            if (ip == null) return false
            if (ip.isNotEmpty() && wifi.startsWith(ip)) {
                return true
            }
            if (ip.isNotEmpty()) return false
            if (add) {
                val editTp = findPreference(key) as EditTextPreference?
                editTp!!.text = wifi
                return true
            }
        } catch (e : Exception)
        {
        }
        return false
    }

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences?, key: String?) {
        val data = RpcSettingsData()
        data.externEnabled = sharedPreferences!!.getBoolean("rpcExternEnable", false)
        data.externEncryption = sharedPreferences.getBoolean("rpcExternEncryption", true)
        data.externPasswrd = sharedPreferences.getString("rpcExternPasswrd", "").toString()
        data.setPort(sharedPreferences.getString("rpcExternPort", "31416").toString())
        data.ipAllowed1 = sharedPreferences.getString("rpcExternAllowIp1", "")!!
        data.ipAllowed2 = sharedPreferences.getString("rpcExternAllowIp2", "")!!
        data.ipAllowed3 = sharedPreferences.getString("rpcExternAllowIp3", "")!!
        data.ipAllowed4 = sharedPreferences.getString("rpcExternAllowIp4", "")!!
        data.ipAllowedWiFi1 = sharedPreferences.getString("rpcExternAllowWfIp1", "")!!
        data.ipAllowedWiFi2 = sharedPreferences.getString("rpcExternAllowWfIp2", "")!!
        data.ipAllowedWiFi3 = sharedPreferences.getString("rpcExternAllowWfIp3", "")!!
        data.ipAllowedWiFi4 = sharedPreferences.getString("rpcExternAllowWfIp4", "")!!

        sendToServiceData(data)
    }

    fun sendToServiceString(dataItem: String)
    {
        val intent = Intent()
        intent.action = "RPC_EXTERN"
        intent.putExtra("STRING", dataItem)
        intent.flags = Intent.FLAG_INCLUDE_STOPPED_PACKAGES
        // make sure this is only send locally but the LocalBroadcast seems to fail
        BOINCActivity.appContext?.sendBroadcast(intent)
//      LocalBroadcastManager.getInstance(BOINCActivity.appContext!!).sendBroadcast(intent)
    }

    fun sendToServiceData(data: RpcSettingsData)
    {
        val intent = Intent()
        intent.action = "RPC_EXTERN"
        val bundle = Bundle()
        bundle.putParcelable("settings_parcel", data)
        intent.putExtra("settings_bundle", bundle)
        intent.flags = Intent.FLAG_INCLUDE_STOPPED_PACKAGES
        // TODO eFMer  make sure this is only send locally but LocalBroadcast is deprecated
        BOINCActivity.appContext?.sendBroadcast(intent)
//        LocalBroadcastManager.getInstance(BOINCActivity.appContext!!).sendBroadcast(intent)
    }

    fun localIpAddress(): String {
        try {
            val wifiManager: WifiManager = BOINCActivity.appContext!!.applicationContext.getSystemService(AppCompatActivity.WIFI_SERVICE) as WifiManager
            val wifiInfo = wifiManager.connectionInfo
            val ipInt = wifiInfo.ipAddress
            if (ipInt == 0) return ""
            return InetAddress.getByAddress(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(ipInt).array()).hostAddress
        } catch (e: UnknownHostException) {
            return ""
        }
    }


}

