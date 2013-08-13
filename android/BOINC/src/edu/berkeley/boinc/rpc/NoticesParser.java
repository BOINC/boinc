package edu.berkeley.boinc.rpc;

import java.util.ArrayList;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

public class NoticesParser extends BaseParser {
	
	private Notice mNotice = null;
	private ArrayList<Notice> mNotices = new ArrayList<Notice>();

	public final ArrayList<Notice> getNotices() {
		return mNotices;
	}

	public static ArrayList<Notice> parse(String rpcResult) {
		try {
			NoticesParser parser = new NoticesParser();
			
			Log.d("blub", rpcResult);
			Xml.parse(rpcResult.replace("&", "&amp;"), parser);
			return parser.getNotices();
		}
		catch (SAXException e) {
			Log.d("NoticesParser","SAXException " + e.getMessage() + e.getException());
			return new ArrayList<Notice>();
		}
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		super.startElement(uri, localName, qName, attributes);
		if (localName.equalsIgnoreCase("notice")) {
			mNotice = new Notice();
		} else {
			// primitive
			mElementStarted = true;
			mCurrentElement.setLength(0);
		}
	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		super.endElement(uri, localName, qName);
		try {
			if (mNotice != null) {
				// inside <notice>
				if (localName.equalsIgnoreCase("notice")) {
					// Closing tag
					if (mNotice.seqno != -1) {
						// seqno is a must
						mNotices.add(mNotice);
					} 
					mNotice = null;
				}
				else {
					// decode inner tags
					if (localName.equalsIgnoreCase("seqno")) {
						mNotice.seqno = Integer.parseInt(mCurrentElement.toString());
					} else if (localName.equalsIgnoreCase("title")) {
						mNotice.title = mCurrentElement.toString();
					} else if (localName.equalsIgnoreCase("description")) {
						String current = mCurrentElement.toString();
						if (current.startsWith("<![CDATA["))
							mNotice.description = current.substring(8, current.length()-3);
						else
							mNotice.description = current;
					} else if (localName.equalsIgnoreCase("create_time")) {
						mNotice.create_time = Double.parseDouble(mCurrentElement.toString());
					} else if (localName.equalsIgnoreCase("arrival_time")) {
						mNotice.arrival_time = Double.parseDouble(mCurrentElement.toString());
					} else if (localName.equalsIgnoreCase("category")) {
						mNotice.category = mCurrentElement.toString();
						if(mNotice.category.equals("server")) mNotice.isServerNotice = true;
						if(mNotice.category.equals("scheduler")) mNotice.isServerNotice = true;
						if(mNotice.category.equals("client")) mNotice.isClientNotice = true;
					} else if (localName.equalsIgnoreCase("link")) {
						mNotice.link = mCurrentElement.toString();
					} else if (localName.equalsIgnoreCase("project_name")) {
						mNotice.project_name = mCurrentElement.toString();
					}
				}
			}
			mElementStarted = false;
		}
		catch (NumberFormatException e) {
			Log.d("NoticesParser","NumberFormatException " + localName + " " + mCurrentElement.toString());
		}
	}
}
