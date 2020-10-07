/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */

package edu.berkeley.boinc.rpcExtern

import java.util.Base64
import java.security.SecureRandom
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

/*

Android <iv>,<extra>,<to_encrypt> (string encode to BASE64 string) -> Gui -> (string decode from BASE64 to plain text string)
Gui encrypt plain text to_encrypt with plain text iv en password (present in the GUI) and plain text extra)
Gui (encrypt using AES <to_encrypt> and encode result to BASE64 string) send <encrypted>
Android <to_encrypt> decode BASE64 and compare with original

string -> to BASE64 string -> send -> received (GUI) -> from BASE64 -> use GUI.
encrypted -> to BASE64 string -> send -> received (ANDROID) -> from BASE64 plain text.

GUI request
<boinc_gui_rpc_request>
<auth1/>
</boinc_gui_rpc_request>\003

BOINC for Android if encryption flag is set

Boinc for Android
<boinc_gui_rpc_reply>
    <auth_encrypted>
        <time_out>0</time_out>
        <iv></iv>
        <extra></extra>
        <to_encrypt><to_encrypt>
        <protocol>1</protocol>
    </auth_encrypted>
</boinc_gui_rpc_reply>\003

<iv> is a random 12 byte string
<extra> is a random 16 byte string
<to_encrypt> is a random 64 byte string
All 3 strings above are BASE64 encoded for transport.
<protocol> must match on of the known encryption protocols.
1 : AES/GCM

On the receiving end (GUI) <iv> and <extra> are BASE64 decoded and placed in a byte array.
The <iv> is used to encrypt the message.
The <extra> is used to add to the password (less than 16 bytes) up to a length of 16 bytes.
the iv (12 byte) and password + extra (16 byte) are used to decode the message from BOINC for Android.
<to_encryp> will be encrypted by the GUI using the iv and the combined password + extra (key)

The reply from the GUI
<boinc_gui_rpc_request>
    <auth_encrypted>
        <encrypted></encrypted>
    </auth_encrypted>
</boinc_gui_rpc_request>\003

<encrypted> is the <to_encryp> string that is encrypted by the GUI and BASE64 decoded for transport to BOINC for Android

after decryption of <encryption> it must match <to_encrypt>

After a match the reply wil be
<boinc_gui_rpc_reply>
    <auth_encrypted>
        <authorized/>
    </auth_encrypted>
</boinc_gui_rpc_reply>\003

If there is a mismatch, BOINC for Android will will wait for 5 seconds until is accepts the next request.
After 3 wrong passwords, the communication is blocked for 1 minute.
A second 3 mismatches will block for 5 minutes and increases by 5 minutes.
The timeout can be reset in the GUI settings menu.

The reply from BOINC for Android on an auth1 request during a timeout will be

<boinc_gui_rpc_reply>
    <auth_encrypted>
        <unauthorized/>
        <time_out>300</time_out> (300 seconds)
    </auth_encrypted>
</boinc_gui_rpc_reply>\003


WARNING: The <iv> must be not be reused after one request completed.


The value of nonce is to be used as a salt with the password. It is randomly generated for each request. To calculate the response, concatenate the nonce and the password (nonce first), then calculate the MD5 hash of the result, i.e: md5(nonce+password). Finally, send an <auth2> request with the calculated hash, in lowercase hexadecimal format.

<boinc_gui_rpc_request>
<auth2>
<nonce_hash>d41d8cd98f00b204e9800998ecf8427e</nonce_hash>
</auth2>
</boinc_gui_rpc_request>\003
The reply will be either <authorized/> or <unauthorized/>.
*/


// https://github.com/BOINC/boinc/issues/4044
// https://www.cryptopp.com/wiki/GCM_Mode
// https://proandroiddev.com/security-best-practices-symmetric-encryption-with-aes-in-java-7616beaaade9
// https://medium.com/@rrohaill/aes-gcm-encryption-decryption-using-kotlin-7f08884eb15b
// https://github.com/phxql/kotlin-crypto-example/blob/master/src/main/kotlin/de/mkammerer/Crypto.kt

class AES_GCM_Test()
{
    var testString = "the quick brown fox jumps over the lazy dog <> THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 1234567890."
    // key and iv must be exactly 16 bytes long
    var key = "1Hbfh667adfDEJ78"
    var plainText = ""
    fun test() : String
    {
        val aes = AES_GCM()
        var keyString = key
        var ivString = String(aes.getIv())
        var encrypted64 : String = aes.encryptGcsString(testString, keyString, ivString)
        plainText = aes.decryptGcsString(encrypted64, keyString, ivString)
        return plainText
    }
}

class AES_GCM {
    var AES_GCM_KEYLENGTH = 16 // byte
    var AES_GCM_IVLENGTH = 12
    fun encryptGcsString(plainText: String, key: String, iv: String) : String    // returns BASE64 string
    {
        var plainTextArray = plainText.toByteArray()
        var keyArray = key.toByteArray()
        var ivArray = iv.toByteArray()
        var encrypted = encryptGcm(plainTextArray, keyArray, ivArray)
        var encrypted64 : String = ""

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            encrypted64 = Base64.getEncoder().withoutPadding().encodeToString(encrypted)
        } // else take empty string
        return encrypted64
    }

    fun decryptGcsString(encrypted64: String, key: String, iv : String) : String
    {
        var encryptedArray : ByteArray = byteArrayOf(0)
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            encryptedArray = Base64.getDecoder().decode(encrypted64)
        }
        var keyArray = key.toByteArray()
        var ivArray = iv.toByteArray()
        var plainText = decryptGcm(encryptedArray, keyArray, ivArray)
        return plainText
    }

    fun encryptGcm(plaintext: ByteArray, key: ByteArray, iv : ByteArray): ByteArray {
//        val ciphertext : ByteArray
        try {
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            val keySpec = SecretKeySpec(key, "AES")

            //val nonce = getIv()
            val gcmSpec = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
                GCMParameterSpec(128, iv) // tag size 128 bit = 16 bytes
            } else {
                return byteArrayOf(0)
            } // 128 bit authentication tag

            cipher.init(Cipher.ENCRYPT_MODE, keySpec, gcmSpec)

            val cipherArray = cipher.doFinal(plaintext)
            return cipherArray
        } catch (e : Exception)
        {

        }
        return byteArrayOf(0)
    }

    fun decryptGcm(encryptedText: ByteArray, key: ByteArray, iv : ByteArray): String {
//        var decodedString = ""
        try {
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            val keySpec = SecretKeySpec(key, "AES")

            val gcmSpec = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
                // var iv = getIv()
                GCMParameterSpec(128, iv)
            } else {
                return ""
            } // 128 bit authentication tag

            cipher.init(Cipher.DECRYPT_MODE, keySpec, gcmSpec)

            val decArr = cipher.doFinal(encryptedText)
            var decodedString = String(decArr)
            return decodedString
        } catch (e : Exception)
        {
            return ""
        }
    }

    fun getIv(): ByteArray {
        val random = SecureRandom()
        val iv = ByteArray(AES_GCM_IVLENGTH) //NEVER REUSE THIS IV WITH SAME KEY
        random.nextBytes(iv)
        return iv
    }

    // Warning this must be handled exactly the same in encryption and decryption
    // for key read password
    // keyExtra must be exactly the same in encryption and decryption
    fun adjustKeyLength(key:String, keyExtra:String) : String
    {
        var newKey = key + keyExtra
        newKey.take(AES_GCM_KEYLENGTH)
        return newKey
    }
}