package edu.berkeley.boinc.rpcExtern.authenticate

class Hex {
    val hexArray = "0123456789abcdef".toCharArray()
    fun HexToByte(hexChars: String) : ByteArray {
        val length = hexChars.length
        val result = ByteArray(length / 2)
        for (i in 0 until length step 2) {
            val firstIndex = hexArray.indexOf(hexChars[i])
            val secondIndex = hexArray.indexOf(hexChars[i + 1])
            val octet = firstIndex.shl(4).or(secondIndex)
            result.set(i.shr(1), octet.toByte())
        }
        return result
    }

    fun byteToHex(bytes: ByteArray): String {
        val hexChars = CharArray(bytes.size * 2)
        for (j in bytes.indices) {
            var v = bytes[j].toInt() and 0xFF
            hexChars[j * 2] = hexArray[v ushr 4]
            hexChars[j * 2 + 1] = hexArray[v and 0x0F]
        }
        return String(hexChars)
    }
}