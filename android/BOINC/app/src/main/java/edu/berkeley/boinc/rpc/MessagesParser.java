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

import android.util.Log;

import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import edu.berkeley.boinc.utils.Logging;

public class MessagesParser {
    private List<Message> mMessages = new ArrayList<>();

    private List<Message> getMessages() {
        return mMessages;
    }

    /**
     * Parse the RPC result (messages) and generate corresponding list.
     *
     * @param rpcResult String returned by RPC call of core client
     * @return list of messages
     */
    public static List<Message> parse(String rpcResult) {
        MessagesParser parser = new MessagesParser();
        try {
            parser.parseMessages(rpcResult);
            return parser.getMessages();
        }
        catch (MessageParseException ex) {
            return Collections.emptyList();
        }
    }

    private void parseMessages(String xml) throws MessageParseException {
        // Removes whitespace at the start of the string.
        xml = StringUtils.stripStart(xml, null);

        int pos = 0;
        int end = xml.length();
        boolean inMsgs = false;

        int newPos;
        Message message = null;

        try {
            while(pos < end) {
                if(!inMsgs) {
                    newPos = xml.indexOf("<msgs>");
                    if(newPos == -1) {
                        throw new MessageParseException();
                    }
                    pos = newPos + 6;
                    inMsgs = true;
                }
                else if(message == null && xml.startsWith("<msg>", pos)) {
                    pos += 5;
                    message = new Message();
                }
                else if(message != null && xml.startsWith("</msg>", pos)) {
                    mMessages.add(message);
                    message = null;
                    pos += 6;
                }
                else if(xml.startsWith("</msgs>", pos)) {
                    break; // end
                }
                else if(message != null) {
                    if(xml.startsWith("<project>", pos)) {
                        pos += 9;
                        newPos = xml.indexOf("</project>", pos);
                        if(newPos == -1) {
                            throw new MessageParseException();
                        }
                        message.project = xml.substring(pos, newPos).trim();
                        pos = newPos + 10;
                    }
                    else if(xml.startsWith("<seqno>", pos)) {
                        pos += 7;
                        newPos = xml.indexOf("</seqno>", pos);
                        if(newPos == -1) {
                            throw new MessageParseException();
                        }
                        message.seqno = Integer.parseInt(xml.substring(pos, newPos).trim());
                        pos = newPos + 8;
                    }
                    else if(xml.startsWith("<pri>", pos)) {
                        pos += 5;
                        newPos = xml.indexOf("</pri>", pos);
                        if(newPos == -1) {
                            throw new MessageParseException();
                        }
                        message.priority = Integer.parseInt(xml.substring(pos, newPos).trim());
                        pos = newPos + 6;
                    }
                    else if(xml.startsWith("<time>", pos)) {
                        pos += 6;
                        newPos = xml.indexOf("</time>", pos);
                        if(newPos == -1) {
                            throw new MessageParseException();
                        }
                        message.timestamp = (long) Double.parseDouble(
                                xml.substring(pos, newPos).trim());
                        pos = newPos + 7;
                    }
                    else if(xml.startsWith("<body>", pos)) {
                        pos += 6;
                        newPos = xml.indexOf("</body>", pos);
                        if(newPos == -1) {
                            throw new MessageParseException();
                        }
                        message.body = xml.substring(pos, newPos).trim();
                        pos = newPos + 7;
                    }
                    else {
                        // skip unknown tag
                        pos++;
                        int endTagIndex = xml.indexOf('>', pos);
                        if(endTagIndex == -1) {
                            throw new MessageParseException();
                        }
                        String tag = xml.substring(pos, endTagIndex);
                        pos = endTagIndex + 1;
                        String endString = "</" + tag + ">";
                        newPos = xml.indexOf(endString, pos);
                        if(newPos == -1) {
                            throw new MessageParseException();
                        }
                        pos = newPos + endString.length();
                    }
                }
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR.equals(Boolean.TRUE)) {
                Log.e(Logging.TAG, "MessagesParser.parseMessages error: ", e);
            }
        }
    }
}
