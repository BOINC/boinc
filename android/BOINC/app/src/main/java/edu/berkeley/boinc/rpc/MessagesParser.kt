/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2021 University of California
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

import android.util.Xml
import edu.berkeley.boinc.utils.Logging
import org.xml.sax.Attributes
import org.xml.sax.SAXException

class MessagesParser : BaseParser() {
    val messages: MutableList<Message> = mutableListOf()
    private lateinit var message: Message

    @Throws(SAXException::class)
    override fun startElement(uri: String?, localName: String, qName: String?, attributes: Attributes?) {
        super.startElement(uri, localName, qName, attributes)
        if (localName.equals(MESSAGE, ignoreCase = true) && !this::message.isInitialized) {
            message = Message()
        } else {
            mElementStarted = true
            mCurrentElement.setLength(0)
        }
    }

    @Throws(SAXException::class)
    override fun endElement(uri: String?, localName: String, qName: String?) {
        super.endElement(uri, localName, qName)
        try {
            if (localName.equals(MESSAGE, ignoreCase = true)) {
                if (message.seqno != -1) {
                    messages.add(message)
                }
                message = Message()
                mElementStarted = false
            } else {
                when {
                    localName.equals(Message.Fields.BODY, ignoreCase = true) -> {
                        message.body = mCurrentElement.toString()
                    }
                    localName.equals(Message.Fields.PRIORITY, ignoreCase = true) -> {
                        message.priority = mCurrentElement.toInt()
                    }
                    localName.equals(PROJECT, ignoreCase = true) -> {
                        message.project = mCurrentElement.toString()
                    }
                    localName.equals(Message.Fields.TIMESTAMP, ignoreCase = true) -> {
                        message.timestamp = mCurrentElement.toDouble().toLong()
                    }
                    localName.equals(SEQNO, ignoreCase = true) -> {
                        message.seqno = mCurrentElement.toInt()
                    }
                }
            }
        } catch (e: Exception) {
            Logging.logException(Logging.Category.XML, "MessagesParser.endElement error: ", e)
        }
    }

    companion object {
        const val MESSAGE = "msg"
        /**
         * Parse the RPC result (messages) and generate corresponding list.
         *
         * @param rpcResult String returned by RPC call of core client
         * @return list of messages
         */
        @JvmStatic
        fun parse(rpcResult: String): List<Message> {
            return try {
                // Replace 0x03 character in the rpcResult string
                val rpcResultReplaced = rpcResult.replace("\u0003", "")
                val parser = MessagesParser()
                Xml.parse(rpcResultReplaced, parser)
                parser.messages
            } catch (e: SAXException) {
                Logging.logException(Logging.Category.RPC, "MessagesParser: malformed XML ", e)
                Logging.logDebug(Logging.Category.XML, "MessagesParser: $rpcResult")

                emptyList()
            }
        }
    }
}
