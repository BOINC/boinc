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

package edu.berkeley.boinc.rpcExtern.encryption

import edu.berkeley.boinc.rpcExtern.authenticate.Hex
import java.security.SecureRandom
import java.util.*
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

class AES_GCM_Test()
{
    var testString = "the quick brown fox jumps over the lazy dog <> THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG 1234567890."
    // key and iv must be exactly 16 / 16 bytes long

    var plainText = ""
    fun test() : String
    {
        val key = "1Hbfh667adfDEJ78"
        val aes = AES_GCM()
        val keyString = key
        val iv =aes.getIv()
        val encrypted64 : String = aes.encryptGcsString(testString, keyString, iv)
        plainText = aes.decryptGcsString(encrypted64, keyString, iv)
        return plainText
    }
}

class AES_GCM {
    var AES_GCM_KEYLENGTH = 16 // byte
    var AES_GCM_IVLENGTH = 16
    fun encryptGcsString(plainText: String, key: String, ivArray: ByteArray) : String    // returns BASE64 string
    {
        try {
            val plainTextArray = plainText.toByteArray()
            val keyArray = key.toByteArray()
            if (keyArray.size != AES_GCM_KEYLENGTH) return ""
            if (ivArray.size != AES_GCM_KEYLENGTH) return ""
//        var ivArray = iv.toByteArray()
            val encrypted = encryptGcm(plainTextArray, keyArray, ivArray)
            var encrypted64 = ""

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                encrypted64 = Base64.getEncoder().withoutPadding().encodeToString(encrypted)
            } // else take empty string
            return encrypted64
        } catch (e: Exception)
        {
            return ""
        }
    }

    fun decryptGcsString(encrypted64: String, key: String, ivArray: ByteArray) : String {
        try {
            var encryptedArray: ByteArray = byteArrayOf(0)
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                encryptedArray = Base64.getDecoder().decode(encrypted64)
            }
            val keyArray = key.toByteArray()
            if (keyArray.size != AES_GCM_KEYLENGTH) return ""
            if (ivArray.size != AES_GCM_KEYLENGTH) return ""
            val plainText = decryptGcm(encryptedArray, keyArray, ivArray)
            return plainText
        } catch (e: Exception)
        {
            return ""
        }
    }

    fun encryptGcm(plaintext: ByteArray, key: ByteArray, iv: ByteArray): ByteArray {
//        val ciphertext : ByteArray
        try {
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            val keySpec = SecretKeySpec(key, "AES")
            val gcmSpec = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
                GCMParameterSpec(128, iv) // tag size 128 bit = 16 bytes
            } else {
                return byteArrayOf(0)
            } // 128 bit authentication tag

            cipher.init(Cipher.ENCRYPT_MODE, keySpec, gcmSpec)

            val cipherArray = cipher.doFinal(plaintext)
            return cipherArray
        } catch (e: Exception)
        {
            var ii = 1
            ii += 1
        }
        return byteArrayOf(0)
    }

    fun decryptGcm(encryptedText: ByteArray, key: ByteArray, iv: ByteArray): String {
//        var decodedString = ""
        try {
            val cipher = Cipher.getInstance("AES/GCM/NoPadding")
            val keySpec = SecretKeySpec(key, "AES")

            val gcmSpec = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
                GCMParameterSpec(128, iv)
            } else {
                return ""
            } // 128 bit authentication tag

            cipher.init(Cipher.DECRYPT_MODE, keySpec, gcmSpec)

            val decArr = cipher.doFinal(encryptedText)
            val decodedString = String(decArr)
            return decodedString
        } catch (e: Exception)
        {
            return ""
        }
    }

    fun getIv(): ByteArray {
        val random = SecureRandom()
        val bytes = ByteArray(AES_GCM_IVLENGTH)
        random.nextBytes(bytes)
        return bytes
    }

    fun getRandomHex(bytes: Int): String
    {
        val hex = Hex()
        val random = SecureRandom()
        val randomHex = ByteArray(bytes)
        random.nextBytes(randomHex)
        return hex.byteToHex(randomHex)
    }
}