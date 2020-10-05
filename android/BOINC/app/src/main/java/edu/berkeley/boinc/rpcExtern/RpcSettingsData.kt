package edu.berkeley.boinc.rpcExtern

// there should be no need to make this public but the compiler thinks otherwise
public class RpcSettingsData {

    var validData = false
    var externEnabled = false
    var externEncryption = true
    var externPasswrd :String = ""
    var externPort : String = "31416"
    var ipAllowed1 = ""
    var ipAllowed2 = ""
    var ipAllowed3 = ""
    var ipAllowed4 = ""

    var ipAllowedList = ArrayList<String>()

    fun set(enabled : Boolean, encryption : Boolean, passwrd : String, port :String, ip1 : String, ip2 : String, ip3 : String , ip4 : String)
    {
        validData = true
        externEnabled = enabled
        externEncryption = encryption
        externPasswrd = passwrd
        externPort = port

        ipAllowed1 = ip1
        ipAllowed2 = ip2
        ipAllowed3 = ip3
        ipAllowed4 = ip4

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