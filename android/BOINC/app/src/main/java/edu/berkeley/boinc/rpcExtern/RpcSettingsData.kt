package edu.berkeley.boinc.rpcExtern

import android.os.Parcel
import android.os.Parcelable
import kotlinx.android.parcel.Parcelize

class RpcSettingsDatax {
    val DEFAULT_PORT = 31416
    val data = RpcSettingsData()
    var validData = false
    var ipAllowedList = ArrayList<String>()

    fun set(enabled : Boolean, encryption : Boolean, passwrd : String, port :String, ip1 : String, ip2 : String, ip3 : String , ip4 : String)
    {
        validData = true
        data.externEnabled = enabled
        data.externEncryption = encryption
        data.externPasswrd = passwrd

        try {
            data.externPort = if (port.isNotEmpty()) port.toInt() else DEFAULT_PORT
        } catch (e: Exception) {
            data.externPort = DEFAULT_PORT
        }

        data.ipAllowed1 = ip1
        data.ipAllowed2 = ip2
        data.ipAllowed3 = ip3
        data.ipAllowed4 = ip4

        if (data.ipAllowed1.isNotEmpty()) {
            ipAllowedList.add(data.ipAllowed1)
        }
        if (data.ipAllowed2.isNotEmpty()) {
            ipAllowedList.add(data.ipAllowed2)
        }
        if (data.ipAllowed3.isNotEmpty()) {
            ipAllowedList.add(data.ipAllowed3)
        }
        if (data.ipAllowed4.isNotEmpty()) {
            ipAllowedList.add(data.ipAllowed4)
        }
    }
}

class  RpcSettingsData() : Parcelable {
    var externEnabled = false
    var externEncryption = true
    var externPasswrd = ""
    var externPort = 0
    var ipAllowed1 = ""
    var ipAllowed2 = ""
    var ipAllowed3 = ""
    var ipAllowed4 = ""
    var ipAllowedWiFi1 = ""
    var ipAllowedWiFi2 = ""
    var ipAllowedWiFi3 = ""
    var ipAllowedWiFi4 = ""

    // not Parcelable
    var ipAllowedList = ArrayList<String>()
    var ipAllowedListWiFi = ArrayList<String>()

    constructor(parcel: Parcel) : this() {
        externEnabled = parcel.readByte() != 0.toByte()
        externEncryption = parcel.readByte() != 0.toByte()
        externPasswrd = parcel.readString().toString()
        externPort = parcel.readInt()
        ipAllowed1 = parcel.readString().toString()
        ipAllowed2 = parcel.readString().toString()
        ipAllowed3 = parcel.readString().toString()
        ipAllowed4 = parcel.readString().toString()
        ipAllowedWiFi1 = parcel.readString().toString()
        ipAllowedWiFi2 = parcel.readString().toString()
        ipAllowedWiFi3 = parcel.readString().toString()
        ipAllowedWiFi4 = parcel.readString().toString()
    }

    fun setPort(port : String)
    {
        externPort = 31416
        try {
            if (port.isNotEmpty()) {
                externPort = port.toInt()
            }
        } catch (e: Exception) {
        }
    }

    fun makeList()
    {
        ipAllowedList.clear()
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

        ipAllowedListWiFi.clear()
        if (ipAllowedWiFi1.isNotEmpty()) {
            ipAllowedListWiFi.add(ipAllowedWiFi1)
        }
        if (ipAllowedWiFi2.isNotEmpty()) {
            ipAllowedListWiFi.add(ipAllowedWiFi2)
        }
        if (ipAllowedWiFi3.isNotEmpty()) {
            ipAllowedListWiFi.add(ipAllowedWiFi3)
        }
        if (ipAllowedWiFi4.isNotEmpty()) {
            ipAllowedListWiFi.add(ipAllowedWiFi4)
        }
    }

    fun matchIp(ip : String) : Boolean
    {
        if (ipAllowedList.isEmpty())
        {
            return true
        }
        var bFound = false
        for (item in ipAllowedList) {
            if (ip.startsWith(item)) {
                bFound = true
                break
            }
        }
        return bFound
    }

    fun matchIpWiFi(ip : String) : Boolean
    {
        if (ipAllowedListWiFi.isEmpty())
        {
            return true
        }
        var bFound = false
        for (item in ipAllowedListWiFi) {
            if (ip.startsWith(item)) {
                bFound = true
                break
            }
        }
        return bFound
    }

    override fun writeToParcel(parcel: Parcel, flags: Int) {
        parcel.writeByte(if (externEnabled) 1 else 0)
        parcel.writeByte(if (externEncryption) 1 else 0)
        parcel.writeString(externPasswrd)
        parcel.writeInt(externPort)
        parcel.writeString(ipAllowed1)
        parcel.writeString(ipAllowed2)
        parcel.writeString(ipAllowed3)
        parcel.writeString(ipAllowed4)
        parcel.writeString(ipAllowedWiFi1)
        parcel.writeString(ipAllowedWiFi2)
        parcel.writeString(ipAllowedWiFi3)
        parcel.writeString(ipAllowedWiFi4)
    }

    override fun describeContents(): Int {
        return 0
    }

    companion object CREATOR : Parcelable.Creator<RpcSettingsData> {
        override fun createFromParcel(parcel: Parcel): RpcSettingsData {
            return RpcSettingsData(parcel)
        }

        override fun newArray(size: Int): Array<RpcSettingsData?> {
            return arrayOfNulls(size)
        }
    }

}
