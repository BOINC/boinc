/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
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


package edu.berkeley.boinc.rpc;

import edu.berkeley.boinc.utils.*;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;

import android.util.Log;

/**
 * Wrapper class for MD5 hash operations for BOINC purpose.
 *
 * @author Palo M.
 */
public class Md5 {
    public static final String TAG = "Md5";

    /**
     * Hashes the input string
     *
     * @param text The text to be hashed
     * @return The hash of the input converted to string
     */
    public final static String hash(String text) {
        try {
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            md5.update(text.getBytes(StandardCharsets.ISO_8859_1), 0, text.length());
            byte[] md5hash = md5.digest();
            StringBuilder sb = new StringBuilder();
            for(byte singleMd5hash : md5hash) {
                sb.append(String.format("%02x", singleMd5hash));
            }
            return sb.toString();
        }
        catch(Exception e) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG, "Error when calculating MD5 hash");
            }
            return "";
        }
    }
}
