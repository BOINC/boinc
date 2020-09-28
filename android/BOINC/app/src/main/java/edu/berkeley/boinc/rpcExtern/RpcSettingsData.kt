package edu.berkeley.boinc.rpcExtern

import android.os.Parcelable
import kotlinx.android.parcel.Parcelize

class RpcSettingsData {

    var validData = false
    var externEnabled = false
    var externEncryption = true
    var externPasswrd :String = ""
    var externPort : String = "31416"
    var ipAllowedList = ArrayList<String>()

    fun set(enabled : Boolean, encryption : Boolean, passwrd : String, port :String, ipAllowed1 : String, ipAllowed2 : String, ipAllowed3 : String , ipAllowed4 : String)
    {
        validData = true
        externEnabled = enabled
        externEncryption = encryption
        externPasswrd = passwrd
        externPort = port
        if (ipAllowed1.isNotEmpty()) {
            ipAllowedList.add(ipAllowed1)
        }
        if (ipAllowed2.isNotEmpty()) {
            ipAllowedList.add(ipAllowed2)
        }
        if (ipAllowed3.isNotEmpty()) {
            ipAllowedList.add(ipAllowed3)
        }
        if (ipAllowed4.isNotEmpty()) {
            ipAllowedList.add(ipAllowed4)
        }
    }
}

@Parcelize
data class RpcSettingsDataItem(
        var externEnabled:Boolean,
        var externEncryption:Boolean,
        var externPasswrd :String,
        var externPort : String,
        var ipAllowed1 : String,
        var ipAllowed2 : String,
        var ipAllowed3 : String,
        var ipAllowed4 : String,
) : Parcelable