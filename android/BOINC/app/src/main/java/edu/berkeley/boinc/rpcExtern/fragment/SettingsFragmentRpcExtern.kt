package edu.berkeley.boinc.rpcExtern.fragment

import android.content.*
import android.net.wifi.WifiManager
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import androidx.preference.PreferenceCategory
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.PreferenceManager
import edu.berkeley.boinc.BOINCActivity
import edu.berkeley.boinc.R
import edu.berkeley.boinc.rpcExtern.RpcSettingsData
import java.net.InetAddress
import java.net.UnknownHostException
import java.nio.ByteBuffer
import java.nio.ByteOrder


class SettingsFragmentRpcExtern : PreferenceFragmentCompat() , SharedPreferences.OnSharedPreferenceChangeListener {
    val mRpcExternBroadCastReceiver = object : BroadcastReceiver() {
        override fun onReceive(contxt: Context?, intent: Intent?) {
            try {
                var statusMsg = ""
                val connectionStatus: String? = intent!!.getStringExtra("data")
                val connectionIp: String? = intent.getStringExtra("ip")
                when (connectionStatus)
                {
                    "IPNOT" -> statusMsg = getString(R.string.status_rpc_extern_ip_not_allowed)  + " Ip: " + connectionIp
                    "CON_NOT_AUTH" -> statusMsg = getString(R.string.status_rpc_extern_password)
                    "CON_OK" ->  statusMsg = getString(R.string.status_rpc_extern_connected) + " Ip: " + connectionIp
                    "CON_NOT" -> statusMsg = getString(R.string.status_rpc_extern_not_connected)
                    "TIMEOUT" -> statusMsg = getString(R.string.status_rpc_extern_not_connected_time)
                    "CON_ASK_MD5_GET_AES" -> statusMsg = getString(R.string.status_rpc_extern_use_hash)
                    "CON_ASK_AES_GET_MD5" -> statusMsg = getString(R.string.status_rpc_extern_use_encrypted)
                    "CONIPMATCH" -> statusMsg = getString(R.string.status_rpc_extern_ip_mismatch)  + " Ip: " + connectionIp
                    "IDLE" ->  statusMsg = getString(R.string.status_rpc_extern_idle)
                    "NOWIFI" -> statusMsg = getString(R.string.status_rpc_extern_no_wifi)
                    "CLOSING" -> statusMsg = getString(R.string.status_rpc_extern_closing)
                    "START" -> statusMsg = getString(R.string.status_rpc_extern_starting)
                }
                val prefTitle = findPreference("rpcExternTitle") as PreferenceCategory?
                val prefStatus = findPreference("rpcExternStatus") as PreferenceCategory?
                var title : String = getString(R.string.rpcExternTitle)
                val status = getString(R.string.rpcExternStatus) + " " + statusMsg
                val ip = localIpAddress()
                title += ip
                prefTitle!!.title = title
                prefStatus!!.title = status
            } catch (e : Exception)
            {
            }
        }
    }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
 //     val sharedPreferences = PreferenceManager.getDefaultSharedPreferences(context)
        preferenceManager.sharedPreferences.registerOnSharedPreferenceChangeListener(this)
//      addPreferencesFromResource(R.xml.settings_rpc_extern )
        setPreferencesFromResource(R.xml.settings_rpc_extern, rootKey)

        val ip = localIpAddress()

        // add IP to preferences
        val prefCat = findPreference("rpcExternTitle") as PreferenceCategory?
        var title : String = getString(R.string.rpcExternTitle)
        title += ip
        prefCat!!.title = title
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

    override fun onSharedPreferenceChanged(sharedPreferences: SharedPreferences?, key: String?) {

        val externEnabled = sharedPreferences!!.getBoolean("rpcExternEnable", false)
        val externEncryption = sharedPreferences.getBoolean("rpcExternEncryption", true)
        val externPassword: String = sharedPreferences.getString("rpcExternPasswrd", "").toString()
        val externPort: String = sharedPreferences.getString("rpcExternPort", "31416").toString()
        val externAllowIp1 = sharedPreferences.getString("rpcExternAllowIp1", "")!!
        val externAllowIp2 = sharedPreferences.getString("rpcExternAllowIp2", "")!!
        val externAllowIp3 = sharedPreferences.getString("rpcExternAllowIp3", "")!!
        val externAllowIp4 = sharedPreferences.getString("rpcExternAllowIp4", "")!!
        sendToServiceData(externEnabled, externEncryption, externPassword, externPort, externAllowIp1, externAllowIp2, externAllowIp3, externAllowIp4)
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

    fun sendToServiceData(externEnabled :Boolean, externEncryption : Boolean, externPassword : String, externPort : String, externAllowIp1 : String, externAllowIp2 : String, externAllowIp3 : String, externAllowIp4 : String,)
    {
        val intent = Intent()
        intent.action = "RPC_EXTERN"
        intent.putExtra("ENABLED", externEnabled)
        intent.putExtra("ENCRYPTION", externEncryption)
        intent.putExtra("PASSWRD", externPassword)
        intent.putExtra("PORT", externPort)
        intent.putExtra("IP1", externAllowIp1)
        intent.putExtra("IP2", externAllowIp2)
        intent.putExtra("IP3", externAllowIp3)
        intent.putExtra("IP4", externAllowIp4)

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
            return InetAddress.getByAddress(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(ipInt).array()).hostAddress
        } catch (e: UnknownHostException) {
            return "0"
        }
    }
}
