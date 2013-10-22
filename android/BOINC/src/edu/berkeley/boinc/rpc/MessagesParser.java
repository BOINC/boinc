/*******************************************************************************
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
 ******************************************************************************/

package edu.berkeley.boinc.rpc;

import java.util.ArrayList;

public class MessagesParser {

	private ArrayList<Message> mMessages = new ArrayList<Message>();
	//private Message mMessage = null;


	public final ArrayList<Message> getMessages() {
		return mMessages;
	}

	/**
	 * Parse the RPC result (app_version) and generate corresponding vector
	 * @param rpcResult String returned by RPC call of core client
	 * @return vector of application version
	 */
	public static ArrayList<Message> parse(String rpcResult) {
		MessagesParser parser = new MessagesParser();
		try {
			parser.parseMessages(rpcResult);
		} catch(Exception ex) {
			return null;
		}
		return parser.getMessages();
	}
	
	public void parseMessages(String xml) {
		int pos = 0;
		int end = xml.length();
		boolean inMsgs = false;
		
		int newPos;
		Message message = null;
		
		try {
			while (pos < end) {
				/* skip spaces */
				while (pos < end) {
					if (!Character.isSpace(xml.charAt(pos)))
						break;
					pos++;
				}
				if (!inMsgs) {
					newPos = xml.indexOf("<msgs>");
					if (newPos == -1)
						throw new RuntimeException("Cant parse messages");
					pos = newPos + 6;
					inMsgs = true;
				} else if (inMsgs && message == null && xml.startsWith("<msg>", pos)) {
					pos += 5;
					message = new Message();
				} else if (message != null && xml.startsWith("</msg>", pos)) {
					mMessages.add(message);
					message = null;
					pos += 6;
				} else if (inMsgs && xml.startsWith("</msgs>", pos)) {
					break; // end
				} else if (message != null) {
					if (xml.startsWith("<project>", pos)) {
						pos += 9;
						newPos = xml.indexOf("</project>", pos);
						if (newPos == -1)
							throw new RuntimeException("Cant parse messages");
						message.project = xml.substring(pos, newPos).trim();
						pos = newPos+10;
					} else if (xml.startsWith("<seqno>", pos)) {
						pos += 7;
						newPos = xml.indexOf("</seqno>", pos);
						if (newPos == -1)
							throw new RuntimeException("Cant parse messages");
						message.seqno = Integer.parseInt(xml.substring(pos, newPos).trim());
						pos = newPos+8;
					} else if (xml.startsWith("<pri>", pos)) {
						pos += 5;
						newPos = xml.indexOf("</pri>", pos);
						if (newPos == -1)
							throw new RuntimeException("Cant parse messages");
						message.priority = Integer.parseInt(xml.substring(pos, newPos).trim());
						pos = newPos+6;
					} else if (xml.startsWith("<time>", pos)) {
						pos += 6;
						newPos = xml.indexOf("</time>", pos);
						if (newPos == -1)
							throw new RuntimeException("Cant parse messages");
						message.timestamp = (long)Double.parseDouble(xml.substring(pos, newPos).trim());
						pos = newPos+7;
					} else if (xml.startsWith("<body>", pos)) {
						pos += 6;
						newPos = xml.indexOf("</body>", pos);
						if (newPos == -1)
							throw new RuntimeException("Cant parse messages");
						message.body = xml.substring(pos, newPos).trim();
						pos = newPos+7;
					} else {
						// skip unknown tag
						pos++;
						int endTagIndex = xml.indexOf(">", pos);
						if (endTagIndex == -1)
							throw new RuntimeException("Cant parse messages");
						String tag = xml.substring(pos, endTagIndex);
						pos = endTagIndex+1;
						String endString = "</"+tag+">";
						newPos = xml.indexOf(endString, pos);
						if (newPos == -1)
							throw new RuntimeException("Cant parse messages");
						pos = newPos + endString.length();
					}
				} 
			}
		} catch (NumberFormatException e) {
		}
	}
}
