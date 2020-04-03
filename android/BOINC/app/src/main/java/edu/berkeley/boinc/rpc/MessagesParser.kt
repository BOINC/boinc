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
package edu.berkeley.boinc.rpc

import android.util.Log
import edu.berkeley.boinc.utils.Logging

class MessagesParser {
    private val messages: MutableList<Message> = mutableListOf()

    @Throws(MessageParseException::class)
    private fun parseMessages(xml: String) {
        // Removes whitespace at the start of the string.
        val xml = xml.trimStart()
        var pos = 0
        val end = xml.length
        var inMsgs = false
        var newPos: Int
        var message: Message? = null
        try {
            while (pos < end) {
                if (!inMsgs) {
                    newPos = xml.indexOf("<msgs>")
                    if (newPos == -1) {
                        throw MessageParseException()
                    }
                    pos = newPos + 6
                    inMsgs = true
                } else if (message == null && xml.startsWith("<msg>", pos)) {
                    pos += 5
                    message = Message()
                } else if (message != null && xml.startsWith("</msg>", pos)) {
                    this.messages.add(message)
                    message = null
                    pos += 6
                } else if (xml.startsWith("</msgs>", pos)) {
                    break // end
                } else if (message != null) {
                    when {
                        xml.startsWith("<project>", pos) -> {
                            pos += 9
                            newPos = xml.indexOf("</project>", pos)
                            if (newPos == -1) {
                                throw MessageParseException()
                            }
                            message.project = xml.substring(pos, newPos).trim { it <= ' ' }
                            pos = newPos + 10
                        }
                        xml.startsWith("<seqno>", pos) -> {
                            pos += 7
                            newPos = xml.indexOf("</seqno>", pos)
                            if (newPos == -1) {
                                throw MessageParseException()
                            }
                            message.seqno = xml.substring(pos, newPos).trim { it <= ' ' }.toInt()
                            pos = newPos + 8
                        }
                        xml.startsWith("<pri>", pos) -> {
                            pos += 5
                            newPos = xml.indexOf("</pri>", pos)
                            if (newPos == -1) {
                                throw MessageParseException()
                            }
                            message.priority = xml.substring(pos, newPos).trim { it <= ' ' }.toInt()
                            pos = newPos + 6
                        }
                        xml.startsWith("<time>", pos) -> {
                            pos += 6
                            newPos = xml.indexOf("</time>", pos)
                            if (newPos == -1) {
                                throw MessageParseException()
                            }
                            message.timestamp =
                                    xml.substring(pos, newPos).trim { it <= ' ' }.toDouble().toLong()
                            pos = newPos + 7
                        }
                        xml.startsWith("<body>", pos) -> {
                            pos += 6
                            newPos = xml.indexOf("</body>", pos)
                            if (newPos == -1) {
                                throw MessageParseException()
                            }
                            message.body = xml.substring(pos, newPos).trim { it <= ' ' }
                            pos = newPos + 7
                        }
                        else -> { // skip unknown tag
                            pos++
                            val endTagIndex = xml.indexOf('>', pos)
                            if (endTagIndex == -1) {
                                throw MessageParseException()
                            }
                            val tag = xml.substring(pos, endTagIndex)
                            pos = endTagIndex + 1
                            val endString = "</$tag>"
                            newPos = xml.indexOf(endString, pos)
                            if (newPos == -1) {
                                throw MessageParseException()
                            }
                            pos = newPos + endString.length
                        }
                    }
                }
            }
        } catch (e: NumberFormatException) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "MessagesParser.parseMessages error: ", e)
            }
        }
    }

    companion object {
        /**
         * Parse the RPC result (messages) and generate corresponding list.
         *
         * @param rpcResult String returned by RPC call of core client
         * @return list of messages
         */
        @JvmStatic
        fun parse(rpcResult: String): List<Message> {
            val parser = MessagesParser()
            return try {
                parser.parseMessages(rpcResult)
                parser.messages
            } catch (ex: MessageParseException) {
                emptyList()
            }
        }
    }
}

/**
 * Exception to be thrown by MessagesParser if an issue occurs while parsing.
 */
internal class MessageParseException : Exception("Can't parse messages")
