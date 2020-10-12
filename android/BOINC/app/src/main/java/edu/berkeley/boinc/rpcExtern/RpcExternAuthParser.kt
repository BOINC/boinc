package edu.berkeley.boinc.rpcExtern
import org.xmlpull.v1.XmlPullParser
import org.xmlpull.v1.XmlPullParserFactory
import java.io.StringReader

public class RpcExternAuthParser {
    val eol : Byte = 3
    private var text: String = ""
    var nonceHash : String = ""
    var encrypted : String = ""

    fun validXml(xmlIn: String): String {
        val xml = xmlIn.replace("\\n".toRegex(), "")
        val xmlByte = xml.toByteArray()
        for (i in xmlByte.indices) {
            if (xmlByte[i] == eol) {
                xmlByte[i] = 0 // remove end marker
            }
        }
        return String(xmlByte)
    }

    fun parse(xmlIn: String)
    {
        val xml = validXml(xmlIn)
        try {
            val factory = XmlPullParserFactory.newInstance()
            factory.isNamespaceAware = true
            val parser = factory.newPullParser()
            parser.setInput(StringReader(xml))
            var eventType = parser.eventType
            while (eventType != XmlPullParser.END_DOCUMENT) {
                val tagname = parser.name
                val parserText = parser.text
                var hash = false
                when (eventType) {
                    XmlPullParser.TEXT -> text = parser.text
                    XmlPullParser.END_TAG ->
                    {
                        if (tagname.equals("nonce_hash", ignoreCase = true)){
                            nonceHash = text
                        }
                        if (tagname.equals("encrypted", ignoreCase = true)) {
                            encrypted = text
                        }
                    }
                }
                eventType = parser.next()
            }
        } catch (e: Exception)
        {
            var ii = 0  // TODO eFMer testing debugger
            ii +=1
        }
    }
}