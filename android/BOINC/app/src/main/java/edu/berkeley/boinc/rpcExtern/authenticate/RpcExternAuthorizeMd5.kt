package edu.berkeley.boinc.rpcExtern.authenticate
import edu.berkeley.boinc.rpcExtern.RpcExternString
import java.security.MessageDigest
import kotlin.random.Random
import kotlin.random.nextUInt

// Warning MD5 is not secure, just makes it less easy to read the password

public class RpcExternAuthorizeMd5 {
    val RpcString = RpcExternString()
    val hex = Hex()
    var mNoncePasswordInHex = ""
    val eol = '\u0003'
    var mInit = false
    val mNonce = getNonce() // only once

    // Not advised, but password may be empty "", is often done in local settings.
    fun auth1(password : String) : String {
        mInit = true
        val reply = RpcString.mRpcReplyBegin + "\n<nonce>" + mNonce + "</nonce>\n" + RpcString.mRpcReplyEnd + "\n" + eol

        // generate nonce + password, to compare in auth2
        val md = MessageDigest.getInstance("MD5")
        val noncePass = mNonce + password
        val toHex = md.digest(noncePass.toByteArray())
        mNoncePasswordInHex = hex.byteToHex(toHex)
        return reply
    }
    fun auth2(hex: String) : Boolean {
        val equ = mNoncePasswordInHex.equals(hex)
        return equ
    }

    private fun getNonce() : String
    {
        // nonce must be random, but only one
        val mRandom = Random(31416)
        val randomL = mRandom.nextUInt()
        val randomR = mRandom.nextUInt()
        val randomLs = randomL.toString().take(10)
        val randomRs = randomR.toString().take(6)
        val nonce = randomLs + "." + randomRs
        return nonce
    }
}
