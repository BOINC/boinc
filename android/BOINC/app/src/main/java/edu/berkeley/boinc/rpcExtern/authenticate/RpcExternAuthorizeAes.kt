package edu.berkeley.boinc.rpcExtern.authenticate

import edu.berkeley.boinc.rpcExtern.RpcExternString
import edu.berkeley.boinc.rpcExtern.encryption.AES_GCM
import java.util.Base64
import java.security.SecureRandom
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

class RpcExternAuthorizeAes {
    val mRpcString = RpcExternString()
    val mAes = AES_GCM()
    val mBoincEol = '\u0003'
    var mHexBytes = 64
    var mInit = false
    lateinit var mIv1 : ByteArray
    lateinit var mIv2 : ByteArray
    lateinit var mHexRandom : String

    /*
    <boinc_gui_rpc_reply>
    <authe2>
        <iv></iv>   base64
        <encrypted><encrypted>
        <protocol>1</protocol>
    </authe2>
    </boinc_gui_rpc_reply>\003
     */

    fun authe1(password : String) : String {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) { // API > 8
            try {
                mInit = true
//            var ivTest : String = "±ýðÌÐg1245fgbvfd"
//            var iv = ivTest.toByteArray();
                mIv1 = mAes.getIv()
                mIv2 = mAes.getIv()
                val iv641 = Base64.getEncoder().encodeToString(mIv1)
                val iv642 = Base64.getEncoder().encodeToString(mIv2)
//              mHexRandom = "the quick brown fox jumps over the lazy dog <> THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 1234567890."
                mHexRandom = mAes.getRandomHex(mHexBytes)

//                var pass = password
//                pass = "1234567890123456"

                val encrypted64: String = mAes.encryptGcsString(mHexRandom, password, mIv1)

                var reply = mRpcString.mRpcReplyBegin + "\n"
                reply += "<iv1>" + iv641 + "</iv1>\n"
                reply += "<iv2>" + iv642 + "</iv2>\n"
                reply += "<encrypted>" + encrypted64 + "</encrypted>\n"
                reply += "<protocol>1</protocol>\n"
                reply += mRpcString.mRpcReplyEnd + "\n" + mBoincEol
                return reply
            } catch (e : Exception)
            {
                var ii = 1
                ii =1
            }
        }
        return mRpcString.mRpcReplyBegin + "\n" + "\"<unauthorized/>\"" + mRpcString.mRpcReplyEnd + "\n" + mBoincEol
    }
    fun auth2(encrypted64 : String, password: String) : Boolean
    {
        var decrypted = ""
        try {
//            val pass = "1234567890123456"
            decrypted = mAes.decryptGcsString(encrypted64, password, mIv2)
            decrypted = decrypted.substring(2,(mHexBytes*2)+2)
        } catch ( e: Exception)
        {
            return false
        }
        if (decrypted.equals(mHexRandom))
        {
            return true
        }
        return false
    }
}