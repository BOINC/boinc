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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Locale;

import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;
import android.util.Log;
import android.util.Xml;
import edu.berkeley.boinc.utils.BOINCDefs;
import edu.berkeley.boinc.utils.BOINCUtils;
import edu.berkeley.boinc.utils.Logging;


/**
 * GUI RPC - the way how GUI can manage BOINC core client and retrieve the information
 * from the client.
 * This tries to be the same as original BOINC C++ GUI, but the names are rewritten
 * for the sake of naming convention. Therefore original RPC_CLIENT becomes RpcClient,
 * get_cc_status() becomes getCcStatus() etc.
 */
public class RpcClient {
	private static final int CONNECT_TIMEOUT = 30000;      // 30s
	private static final int READ_TIMEOUT = 15000;         // 15s
	private static final int READ_BUF_SIZE = 2048;
	private static final int RESULT_BUILDER_INIT_SIZE = 131072; // Yes, 128K
	private static final int REQUEST_BUILDER_INIT_SIZE = 80;
	
	public static final int SUCCESS = 0;
	public static final int ERR_RETRY = -199;
	public static final int ERR_IN_PROGRESS = -204;
	
	public static final int PROJECT_UPDATE  = 1;
	public static final int PROJECT_SUSPEND = 2;
	public static final int PROJECT_RESUME  = 3;
	public static final int PROJECT_NNW     = 4;
	public static final int PROJECT_ANW     = 5;
	public static final int PROJECT_DETACH	= 6;
	public static final int PROJECT_RESET	= 7;

	public static final int RESULT_SUSPEND  = 10;
	public static final int RESULT_RESUME   = 11;
	public static final int RESULT_ABORT    = 12;

	public static final int TRANSFER_RETRY  = 20;
	public static final int TRANSFER_ABORT  = 21;
	
	public static final int MGR_DETACH = 30;
	public static final int MGR_SYNC = 31;
	
	private Socket mSocket;
	private OutputStreamWriter mOutput;
	private InputStream mInput;
	private byte[] mReadBuffer = new byte[READ_BUF_SIZE];
	protected StringBuilder mResult = new StringBuilder(RESULT_BUILDER_INIT_SIZE);
	protected StringBuilder mRequest = new StringBuilder(REQUEST_BUILDER_INIT_SIZE);

	protected String mLastErrorMessage = null;
	
	public RpcClient() {}


	/*
	 * Private classes - Helpers
	 */

	private class Auth1Parser extends DefaultHandler {
		private StringBuilder mResult = null;
		private String mCurrentElement = null;
		private boolean mNonceParsed = false;

		public Auth1Parser(StringBuilder result) {
			mResult = result;
		}

		@Override
		public void characters(char[] ch, int start, int length) throws SAXException {
			super.characters(ch, start, length);
			// put it into StringBuilder
	        mCurrentElement = new String(ch, start, length);
		}

		@Override
		public void endElement(String uri, String localName, String qName) throws SAXException {
			super.endElement(uri, localName, qName);
			if (localName.equalsIgnoreCase("nonce") && !mNonceParsed) {
				mResult.append(mCurrentElement);
				mNonceParsed = true;
			}
			mCurrentElement = null;
		}
	}

	private class Auth2Parser extends DefaultHandler {
		private StringBuilder mResult = null;
		private boolean mParsed = false;

		public Auth2Parser(StringBuilder result) {
			mResult = result;
		}

		@Override
		public void endElement(String uri, String localName, String qName) throws SAXException {
			super.endElement(uri, localName, qName);
			if (localName.equalsIgnoreCase("authorized") && !mParsed) {
				mResult.append("authorized");
				mParsed = true;
			}
			else if (localName.equalsIgnoreCase("unauthorized") && !mParsed) {
				mResult.append("unauthorized");
				mParsed = true;
			}
		}
	}

	/*
	 * Helper methods
	 */

	private static final String modeName(int mode) {
		switch (mode) {
		case BOINCDefs.RUN_MODE_ALWAYS: return "<always/>";
		case BOINCDefs.RUN_MODE_AUTO: return "<auto/>";
		case BOINCDefs.RUN_MODE_NEVER: return "<never/>";
		case BOINCDefs.RUN_MODE_RESTORE: return "<restore/>";
		default: return "";
		}
	}

	/*
	 * Methods for connection - opening/closing/authorization/status
	 */

	/**
	 * Connect to BOINC core client
	 * @param address Internet address of client (hostname or IP-address)
	 * @param port Port of BOINC client (default port is 31416)
	 * @return true for success, false for failure
	 */
	public boolean open(String address, int port) {
		if (isConnected()) {
			// Already connected
			if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "Attempt to connect when already connected");
			// We better close current connection and reconnect (address/port could be different)
			close();
		}
		try {
			mSocket = new Socket();
			mSocket.connect(new InetSocketAddress(address, port), CONNECT_TIMEOUT);
			mSocket.setSoTimeout(READ_TIMEOUT);
			mInput = mSocket.getInputStream();
			mOutput = new OutputStreamWriter(mSocket.getOutputStream(), "ISO8859_1");
		}
		catch (UnknownHostException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "connect failure: unknown host \"" + address + "\"", e);
			mSocket = null;
			return false;
		}
		catch (IllegalArgumentException e) {
			if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "connect failure: illegal argument", e);
			mSocket = null;
			return false;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "connect failure", e);
			mSocket = null;
			return false;
		}
		if(Logging.DEBUG) Log.d(Logging.TAG, "open(" + address + ", " + port + ") - Connected successfully");
		return true;
	}

	/**
	 * Closes the currently opened connection to BOINC core client
	 */
	public synchronized void close() {
		if (!isConnected()) {
			// Not connected - just return (can be cleanup "for sure")
			return;
		}
		try {
			mInput.close();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "input close failure", e);
		}
		try {
			mOutput.close();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "output close failure", e);
		}
		try {
			mSocket.close();
			if(Logging.DEBUG) Log.d(Logging.TAG, "close() - Socket closed");
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "socket close failure", e);
		}
		mSocket = null;
	}
	
	public String getLastErrorMessage() {
		return mLastErrorMessage;
	}

	/**
	 * Performs the BOINC authorization towards currently connected client. 
	 * The authorization uses MD5 hash of client's password and random value. 
	 * Clear-text password is never sent over network.
	 * @param password Clear text password used for authorization
	 * @return true for success, false for failure
	 */
	public synchronized boolean authorize(String password) {
		if (!isConnected()) {
			return false;
		}
		try {
			// Phase 1: get nonce
			sendRequest("<auth1/>\n");
			String auth1Rsp = receiveReply();
			mRequest.setLength(0);
			Xml.parse(auth1Rsp, new Auth1Parser(mRequest)); // get nonce value
			// Operation: combine nonce & password, make MD5 hash
			mRequest.append(password);
			String nonceHash = Md5.hash(mRequest.toString());
			// Phase 2: send hash to client
			mRequest.setLength(0);
			mRequest.append("<auth2>\n<nonce_hash>");
			mRequest.append(nonceHash);
			mRequest.append("</nonce_hash>\n</auth2>\n");
			sendRequest(mRequest.toString());
			String auth2Rsp = receiveReply();
			mRequest.setLength(0);
			Xml.parse(auth2Rsp, new Auth2Parser(mRequest));
			if (!mRequest.toString().equals("authorized")) {
				if(Logging.DEBUG) Log.d(Logging.TAG, "authorize() - Failure");
				return false;
			}
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in authorize()", e);
			return false;
		}
		catch (SAXException e) {
			Log.i(Logging.TAG, "Malformed XML received in authorize()");
			return false;
		}
		if(Logging.DEBUG) Log.d(Logging.TAG, "authorize() - Successful");
		return true;
	}

	/**
	 * Checks the current state of connection
	 * @return true if connected to BOINC core client, false if not connected
	 */
	public final boolean isConnected() {
		return (mSocket != null) ? mSocket.isConnected() : false;
	}

	/**
	 * Checks whether current connection can be used (data can be sent and received)
	 * This is achieved by sending the empty BOINC command - so in case there is
	 * socket still opened, but other side already closed connection, it will be detected.
	 * @return true if other side responds, false if data cannot be sent or received
	 */
	public synchronized boolean connectionAlive() {
		if (!isConnected()) return false;
		try {
			// We just get the status via socket and do not parse reply
			sendRequest("<get_cc_status/>\n");
			String result = receiveReply();
			if (result.length() == 0) {
				// End of stream reached and no data were received in reply
				// We assume that socket is closed on the other side,
				// most probably client shut down
				return false;
			}
			return true;
		}
		catch (IOException e) {
			return false;
		}
	}

	/*
	 * Private methods for send/receive data
	 */

	/**
	 * Send RPC request to BOINC core client (XML-formatted)
	 * @param request The request itself
	 * @throws IOException if error occurs when sending the request
	 */
	protected void sendRequest(String request) throws IOException {
		if (Logging.RPC_PERFORMANCE) if(Logging.DEBUG) Log.d(Logging.TAG, "mRequest.capacity() = " + mRequest.capacity());
		if (Logging.RPC_DATA) if(Logging.DEBUG) Log.d(Logging.TAG, "Sending request: \n" + request.toString());
		if (mOutput == null)
			return;
		mOutput.write("<boinc_gui_rpc_request>\n");
		mOutput.write(request);
		mOutput.write("</boinc_gui_rpc_request>\n\003");
		mOutput.flush();
	}

	/**
	 * Read the reply from BOINC core client
	 * @return the data read from socket
	 * @throws IOException if error occurs when reading from socket
	 */
	protected String receiveReply() throws IOException {
		mResult.setLength(0);
		if (Logging.RPC_PERFORMANCE) if(Logging.DEBUG) Log.d(Logging.TAG, "mResult.capacity() = " + mResult.capacity());

		long readStart = System.nanoTime();

		// Speed is (with large data): ~ 45 KB/s for buffer size 1024
		//                             ~ 90 KB/s for buffer size 2048
		//                             ~ 95 KB/s for buffer size 4096
		// The chosen buffer size is 2048
		int bytesRead;
		if (mInput == null)
			return mResult.toString();	// empty string
		do {
			bytesRead = mInput.read(mReadBuffer);
			if (bytesRead == -1) break;
			mResult.append(new String(mReadBuffer, 0, bytesRead));
			if (mReadBuffer[bytesRead-1] == '\003') {
				// Last read byte marks the end of transfer
				mResult.setLength(mResult.length() - 1);
				break;
			}
		} while (true);

		if (Logging.RPC_PERFORMANCE) {
			float duration = (System.nanoTime() - readStart)/1000000000.0F;
			long bytesCount = mResult.length();
			if (duration == 0) duration = 0.001F;
			if(Logging.DEBUG) Log.d(Logging.TAG, "Reading from socket took " + duration + " seconds, " + bytesCount + " bytes read (" + (bytesCount / duration) + " bytes/second)");
		}

		if (Logging.RPC_PERFORMANCE) if(Logging.DEBUG) Log.d(Logging.TAG, "mResult.capacity() = " + mResult.capacity());

		if (Logging.RPC_DATA) {
			BufferedReader dbr = new BufferedReader(new StringReader(mResult.toString()));
			String dl;
			int ln = 0;
			try {
				while ((dl = BOINCUtils.readLineLimit(dbr, 4096)) != null) {
					++ln;
					if(Logging.DEBUG) Log.d(Logging.TAG, String.format("%4d: %s", ln, dl));
				}
			}
			catch (IOException ioe) {
			}
		}
		return mResult.toString();
	}

	/*
	 * GUI RPC calls
	 */

	public synchronized VersionInfo exchangeVersions() {
		mLastErrorMessage = null;
		mRequest.setLength(0);
		mRequest.append("<exchange_versions>\n  <major>");
		mRequest.append(Boinc.MAJOR_VERSION);
		mRequest.append("</major>\n  <minor>");
		mRequest.append(Boinc.MINOR_VERSION);
		mRequest.append("</minor>\n  <release>");
		mRequest.append(Boinc.RELEASE);
		mRequest.append("</release>\n</exchange_versions>\n");
		try {
			sendRequest(mRequest.toString());
			VersionInfo versionInfo = VersionInfoParser.parse(receiveReply());
			return versionInfo;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in exchangeVersions()", e);
			return null;
		}
	}
	

	/**
	 * Performs get_cc_status RPC towards BOINC client
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized CcStatus getCcStatus() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_cc_status/>\n");
			CcStatus ccStatus = CcStatusParser.parse(receiveReply());
			return ccStatus;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getCcStatus()", e);
			return null;
		}
	}

	/**
	 * Performs get_file_transfers RPC towards BOINC client
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized ArrayList<Transfer> getFileTransfers() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_file_transfers/>\n");
			ArrayList<Transfer> transfers = TransfersParser.parse(receiveReply());
			return transfers;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getFileTransfers()", e);
			return null;
		}
	}

	/**
	 * Performs get_host_info RPC towards BOINC client
	 * 
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized HostInfo getHostInfo() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_host_info/>\n");
			HostInfo hostInfo = HostInfoParser.parse(receiveReply());
			return hostInfo;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getHostInfo()", e);
			return null;
		}
	}

	/**
	 * Performs get_message_count RPC towards BOINC client
	 * Returns highest seqNo
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized int getMessageCount() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_message_count/>\n");
			int seqNo = MessageCountParser.getSeqno(receiveReply());
			if(Logging.DEBUG) Log.d(Logging.TAG,"RpcClient.getMessageCount returning: " + seqNo);
			return seqNo;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getMessageCount()", e);
			return -1;
		}
	}

	/**
	 * Performs get_messages RPC towards BOINC client
	 * Returns client messages that are more recent than seqNo param
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized ArrayList<Message> getMessages(int seqNo) {
		mLastErrorMessage = null;
		try {
			String request;
			if (seqNo == 0) {
				// get all messages
				request = "<get_messages/>\n";
			}
			else {
				request =
					"<get_messages>\n" +
					" <seqno>" + seqNo + "</seqno>\n" +
					"</get_messages>\n";
			}
			sendRequest(request);
			ArrayList<Message> messages = MessagesParser.parse(receiveReply());
			if(messages == null) messages = new ArrayList<Message>(); // do not return null
			return messages;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getMessages()", e);
			return null;
		}
	}
	
	/**
	 * Performs get_notices PRC towards BOINC client
	 * Returns client messages that are more recent than seqNo param
	 * @return List of Notices
	 */
	public synchronized ArrayList<Notice> getNotices(int seqNo) {
		mLastErrorMessage = null;
		try {
			String request;
			if (seqNo == 0) {
				// get all notices
				request = "<get_notices/>\n";
			}
			else {
				request =
					"<get_notices>\n" +
					" <seqno>" + seqNo + "</seqno>\n" +
					"</get_notices>\n";
			}
			sendRequest(request);
			ArrayList<Notice> notices = NoticesParser.parse(receiveReply());
			if(notices == null) notices = new ArrayList<Notice>(); // do not return null
			return notices;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getMessages()", e);
			return new ArrayList<Notice>();
		}
	}

	/**
	 * Performs get_project_status RPC towards BOINC client
	 * 
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized ArrayList<Project> getProjectStatus() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_project_status/>\n");
			ArrayList<Project> projects = ProjectsParser.parse(receiveReply());
			return projects;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getProjectStatus()", e);
			return null;
		}
	}
	
	/**
	 * Performs get_results RPC towards BOINC client (only active results)
	 * 
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized ArrayList<Result> getActiveResults() {
		mLastErrorMessage = null;
		final String request =
			"<get_results>\n" +
			"<active_only>1</active_only>\n" +
			"</get_results>\n";
		try {
			sendRequest(request);
			ArrayList<Result> results = ResultsParser.parse(receiveReply());
			return results;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getActiveResults()", e);
			return null;
		}
	}

	/**
	 * Performs get_results RPC towards BOINC client (all results)
	 * 
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized ArrayList<Result> getResults() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_results/>\n");
			ArrayList<Result> results = ResultsParser.parse(receiveReply());
			return results;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getResults()", e);
			return null;
		}
	}

	/**
	 * Performs get_state RPC towards BOINC client
	 * 
	 * @return result of RPC call in case of success, null otherwise
	 */
	public synchronized CcState getState() {
		mLastErrorMessage = null;
		try {
			sendRequest("<get_state/>\n");
			CcState result = CcStateParser.parse(receiveReply());
			return result;
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getState()", e);
			return null;
		}
	}

	/**
	 * Reports the current device state to the BOINC core client,
	 * if not called frequently, BOINC core client will suspend
	 * @return true for success, false for failure
	 */
	public synchronized boolean reportDeviceStatus(DeviceStatus deviceStatus) {
		mLastErrorMessage = null;
		mRequest.setLength(0);
		mRequest.append("<report_device_status>\n <device_status>\n  <on_ac_power>");
		mRequest.append(deviceStatus.isOn_ac_power() ? 1 : 0);
		mRequest.append("</on_ac_power>\n  <on_usb_power>");
		mRequest.append(deviceStatus.isOn_usb_power() ? 1 : 0);
		mRequest.append("</on_usb_power>\n  <battery_charge_pct>");
		mRequest.append(deviceStatus.getBattery_charge_pct());
		mRequest.append("</battery_charge_pct>\n  <battery_state>");
		mRequest.append(deviceStatus.getBattery_state());
		mRequest.append("</battery_state>\n  <battery_temperature_celsius>");
		mRequest.append(deviceStatus.getBattery_temperature_celcius());
		mRequest.append("</battery_temperature_celsius>\n  <wifi_online>");
		mRequest.append(deviceStatus.isWifi_online() ? 1 : 0);
		mRequest.append("</wifi_online>\n  <user_active>");
		mRequest.append(deviceStatus.isUser_active() ? 1 : 0);
		mRequest.append("</user_active>\n </device_status>\n</report_device_status>\n");
		try {
			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in networkAvailable()", e);
			return false;
		}
	}
	
	/**
	 * Reports the Android model as host info to the client
	 * @return true for success, false for failure
	 */
	public synchronized boolean setHostInfo(String hostInfo){
		mLastErrorMessage = null;
		mRequest.setLength(0);
		mRequest.append("<set_host_info>\n <host_info>\n  <product_name>");
		mRequest.append(hostInfo);
		mRequest.append("</product_name>\n </host_info>\n</set_host_info>\n");
		try {
			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in networkAvailable()", e);
			return false;
		}
	}

	/**
	 * Tells the BOINC core client that a network connection is available,
	 * and that it should do as much network activity as it can.
	 * @return true for success, false for failure
	 */
	public synchronized boolean networkAvailable() {
		mLastErrorMessage = null;
		try {
			sendRequest("<network_available/>\n");
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in networkAvailable()", e);
			return false;
		}
	}

	/**
	 * Triggers change of state of project in BOINC core client
	 * @param operation operation to be triggered
	 * @param projectUrl master URL of project
	 * @return true for success, false for failure
	 */
	public synchronized boolean projectOp(int operation, String projectUrl) {
		try {
			String opTag;
			switch (operation) {
			case PROJECT_UPDATE:
				opTag = "project_update";
				break;
			case PROJECT_SUSPEND:
				opTag = "project_suspend";
				break;
			case PROJECT_RESUME:
				opTag = "project_resume";
				break;
			case PROJECT_NNW:
				opTag = "project_nomorework";
				break;
			case PROJECT_ANW:
				opTag = "project_allowmorework";
				break;
			case PROJECT_DETACH:
				opTag = "project_detach";
				break;
			case PROJECT_RESET:
				opTag = "project_reset";
				break;
			default:
				if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "projectOp() - unsupported operation: " + operation);
				return false;
			}
			String request =
				"<" + opTag + ">\n" +
				"<project_url>" + projectUrl + "</project_url>\n" +
				"</" + opTag + ">\n";

			sendRequest(request);
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in projectOp()", e);
			return false;
		}
	}
	
	private String getPasswdHash(String passwd, String email_addr) {
		return Md5.hash(passwd+email_addr);
	}
	
	/**
	 * Creates account
	 * @param accountIn - account info
	 * @return true for success, false for failure
	 */
	public synchronized boolean createAccount(AccountIn accountIn) {
		try {
			mRequest.setLength(0);
			mRequest.append("<create_account>\n   <url>");
			mRequest.append(accountIn.url);
			mRequest.append("</url>\n   <email_addr>");
			mRequest.append(accountIn.email_addr);
			mRequest.append("</email_addr>\n   <passwd_hash>");
			mRequest.append(getPasswdHash(accountIn.passwd, accountIn.email_addr));
			mRequest.append("</passwd_hash>\n   <user_name>");
			if (accountIn.user_name!=null)
				mRequest.append(accountIn.user_name);
			mRequest.append("</user_name>\n   <team_name>");
			if (accountIn.team_name!=null)
				mRequest.append(accountIn.team_name);
			mRequest.append("</team_name>\n<create_account>\n");
			
			sendRequest(mRequest.toString());
			
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in createAccount()", e);
			return false;
		}
	}
	
	/**
	 * polling create account
	 * @return account output
	 */
	public synchronized AccountOut createAccountPoll() {
		try {
			mRequest.setLength(0);
			mRequest.append("<create_account_poll/>");
			
			sendRequest(mRequest.toString());
			return AccountOutParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getCreateAccountPoll()", e);
			return null;
		}
	}
	
	/**
	 * Looks up account
	 * @param accountIn - account info
	 * @return true for success, false for failure
	 */
	public synchronized boolean lookupAccount(AccountIn accountIn) {
		try {
			String id;
			if(accountIn.email_addr == null || accountIn.email_addr.isEmpty()) id = accountIn.user_name;
			else id = accountIn.email_addr;
			mRequest.setLength(0);
			mRequest.append("<lookup_account>\n <url>");
			mRequest.append(accountIn.url);
			mRequest.append("</url>\n <email_addr>");
			mRequest.append(id.toLowerCase(Locale.US));
			mRequest.append("</email_addr>\n <passwd_hash>");
			mRequest.append(getPasswdHash(accountIn.passwd, id.toLowerCase(Locale.US)));
			mRequest.append("</passwd_hash>\n</lookup_account>\n");
			sendRequest(mRequest.toString());
			
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in lookupAccount()", e);
			return false;
		}
	}
	
	/**
	 * polling lookup account
	 * @return account output
	 */
	public synchronized AccountOut lookupAccountPoll() {
		try {
			mRequest.setLength(0);
			mRequest.append("<lookup_account_poll/>");
			
			sendRequest(mRequest.toString());
			return AccountOutParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getLookupAccountPoll()", e);
			return null;
		}
	}
	
	/**
	 * Attach to project 
	 * @param url project url
	 * @param authenticator account key
	 * @param name project name
	 * @return
	 */
	public synchronized boolean projectAttach(String url, String authenticator, String name) {
		try {
			mRequest.setLength(0);
			mRequest.append("<project_attach>\n   <project_url>");
			mRequest.append(url);
			mRequest.append("</project_url>\n   <authenticator>");
			mRequest.append(authenticator);
			mRequest.append("</authenticator>\n   <project_name>");
			mRequest.append(name);
			mRequest.append("</project_name>\n</project_attach>\n");
			
			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in projectAttach()", e);
			return false;
		}
	}
	
	/**
	 * polling project attach
	 * @return project attach reply
	 */
	public synchronized ProjectAttachReply projectAttachPoll() {
		try {
			mRequest.setLength(0);
			mRequest.append("<project_attach_poll/>");
			
			sendRequest(mRequest.toString());
			return ProjectAttachReplyParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in projectAttachPoll()", e);
			return null;
		}
	}
	
	/**
	 * performs acct_mgr_rpc towards client
	 * attaches account manager to client
	 * requires polling of status
	 * @param url
	 * @param name
	 * @param passwd
	 * @return success
	 */
	public synchronized boolean acctMgrRPC(String url, String name, String passwd) {
		try {
			mRequest.setLength(0);
			mRequest.append("<acct_mgr_rpc>\n   <url>");
			mRequest.append(url);
			mRequest.append("</url>\n   <name>");
			mRequest.append(name);
			mRequest.append("</name>\n   <password>");
			mRequest.append(passwd);
			mRequest.append("</password>\n</acct_mgr_rpc>\n");

			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in acctMgrRPC()", e);
			return false;
		}
	}
	
	/**
	 * performs acct_mgr_rpc with <use_config_file/> instead of login information
	 * @return success
	 */
	public synchronized boolean acctMgrRPC() {
		try {
			mRequest.setLength(0);
			mRequest.append("<acct_mgr_rpc>\n<use_config_file/>\n</acct_mgr_rpc>\n");

			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in acctMgrRPC()", e);
			return false;
		}
	}

	/**
	 * performs acct_mgr_rpc_poll towards client
	 * polls status of acct_mgr_rpc
	 * @return status class AcctMgrRPCReply
	 */
	public synchronized AcctMgrRPCReply acctMgrRPCPoll() {
		try {
			mRequest.setLength(0);
			mRequest.append("<acct_mgr_rpc_poll/>");

			sendRequest(mRequest.toString());
			return AcctMgrRPCReplyParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in acctMgrRPCPoll()", e);
			return null;
		}
	}
	
	/**
	 * performs acct_mgr_info towards client
	 * @return status class AcctMgrInfo
	 */
	public synchronized AcctMgrInfo getAcctMgrInfo() {
		mLastErrorMessage = null;
		try {
			sendRequest("<acct_mgr_info/>\n");
			return AcctMgrInfoParser.parse(receiveReply());
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getAcctMgrInfo()", e);
			return null;
		}
	}
	
	public synchronized boolean getProjectConfig(String url) {
		try {
			mRequest.setLength(0);
			mRequest.append("<get_project_config>\n   <url>");
			mRequest.append(url);
			mRequest.append("</url>\n</get_project_config>\n");
			
			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getProjectConfig()", e);
			return false;
		}
	}
	
	public synchronized ProjectConfig getProjectConfigPoll() {
		try {
			mRequest.setLength(0);
			mRequest.append("<get_project_config_poll/>");
			
			sendRequest(mRequest.toString());
			return ProjectConfigReplyParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getProjectConfigPoll()", e);
			return null;
		}
	}
	
	public synchronized ArrayList<ProjectInfo> getAllProjectsList() {
		try {
			mRequest.setLength(0);
			mRequest.append("<get_all_projects_list/>");
			
			sendRequest(mRequest.toString());
			return ProjectInfoParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getAllProjectsList()", e);
			return null;
		}
		
	}
	
	public synchronized GlobalPreferences getGlobalPrefsWorkingStruct() {
		try {
			mRequest.setLength(0);
			mRequest.append("<get_global_prefs_working/>");
			
			sendRequest(mRequest.toString());
			return GlobalPreferencesParser.parse(receiveReply());
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in globalPrefsWorking()", e);
			return null;
		}
	}
	
	public synchronized boolean setGlobalPrefsOverride(String globalPrefs) {
		try {
			mRequest.setLength(0);
			mRequest.append("<set_global_prefs_override>\n");
			mRequest.append(globalPrefs);
			mRequest.append("</set_global_prefs_override>\n");
			
			sendRequest(mRequest.toString());
			receiveReply();
			return true;
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in setGlobalPrefsOverride()", e);
			return false;
		}
	}
	
	public synchronized boolean setGlobalPrefsOverrideStruct(GlobalPreferences globalPrefs) {
		try {
			mRequest.setLength(0);
			mRequest.append("<set_global_prefs_override>\n<global_preferences>\n  <run_on_batteries>");
			mRequest.append(globalPrefs.run_on_batteries ? 1 : 0);
			mRequest.append("</run_on_batteries>\n  <battery_charge_min_pct>");
			mRequest.append(globalPrefs.battery_charge_min_pct);
			mRequest.append("</battery_charge_min_pct>\n  <battery_max_temperature>");
			mRequest.append(globalPrefs.battery_max_temperature);
			mRequest.append("</battery_max_temperature>\n  <run_gpu_if_user_active>");
			mRequest.append(globalPrefs.run_gpu_if_user_active ? 1 : 0);
			mRequest.append("</run_gpu_if_user_active>\n  <run_if_user_active>");
			mRequest.append(globalPrefs.run_if_user_active ? 1 : 0);
			mRequest.append("</run_if_user_active>\n  <idle_time_to_run>");
			mRequest.append(globalPrefs.idle_time_to_run);
			mRequest.append("</idle_time_to_run>\n  <suspend_cpu_usage>");
			mRequest.append(globalPrefs.suspend_cpu_usage);
			mRequest.append("</suspend_cpu_usage>\n  <start_hour>");
			mRequest.append(globalPrefs.cpu_times.start_hour);
			mRequest.append("</start_hour>\n  <end_hour>");
			mRequest.append(globalPrefs.cpu_times.end_hour);
			mRequest.append("</end_hour>\n  <net_start_hour>");
			mRequest.append(globalPrefs.net_times.start_hour);
			mRequest.append("</net_start_hour>\n  <net_end_hour>");
			mRequest.append(globalPrefs.net_times.end_hour);
			mRequest.append("</net_end_hour>\n  <max_ncpus_pct>");
			mRequest.append(globalPrefs.max_ncpus_pct);
			mRequest.append("</max_ncpus_pct>\n  <leave_apps_in_memory>");
			mRequest.append(globalPrefs.leave_apps_in_memory ? 1 : 0);
			mRequest.append("</leave_apps_in_memory>\n  <dont_verify_images>");
			mRequest.append(globalPrefs.dont_verify_images ? 1 : 0);
			mRequest.append("</dont_verify_images>\n  <work_buf_min_days>");
			mRequest.append(globalPrefs.work_buf_min_days);
			mRequest.append("</work_buf_min_days>\n  <work_buf_additional_days>");
			mRequest.append(globalPrefs.work_buf_additional_days);
			mRequest.append("</work_buf_additional_days>\n  <disk_interval>");
			mRequest.append(globalPrefs.disk_interval);
			mRequest.append("</disk_interval>\n  <cpu_scheduling_period_minutes>");
			mRequest.append(globalPrefs.cpu_scheduling_period_minutes);
			mRequest.append("</cpu_scheduling_period_minutes>\n  <disk_max_used_gb>");
			mRequest.append(globalPrefs.disk_max_used_gb);
			mRequest.append("</disk_max_used_gb>\n  <disk_max_used_pct>");
			mRequest.append(globalPrefs.disk_max_used_pct);
			mRequest.append("</disk_max_used_pct>\n  <disk_min_free_gb>");
			mRequest.append(globalPrefs.disk_min_free_gb);
			mRequest.append("</disk_min_free_gb>\n  <ram_max_used_busy_pct>");
			mRequest.append(globalPrefs.ram_max_used_busy_frac);
			mRequest.append("</ram_max_used_busy_pct>\n  <ram_max_used_idle_pct>");
			mRequest.append(globalPrefs.ram_max_used_idle_frac);
			mRequest.append("</ram_max_used_idle_pct>\n  <max_bytes_sec_up>");
			mRequest.append(globalPrefs.max_bytes_sec_up);
			mRequest.append("</max_bytes_sec_up>\n  <max_bytes_sec_down>");
			mRequest.append(globalPrefs.max_bytes_sec_down);
			mRequest.append("</max_bytes_sec_down>\n  <cpu_usage_limit>");
			mRequest.append(globalPrefs.cpu_usage_limit);
			mRequest.append("</cpu_usage_limit>\n  <daily_xfer_limit_mb>");
			mRequest.append(globalPrefs.daily_xfer_limit_mb);
			mRequest.append("</daily_xfer_limit_mb>\n  <daily_xfer_period_days>");
			mRequest.append(globalPrefs.daily_xfer_period_days);
			mRequest.append("</daily_xfer_period_days>\n  <network_wifi_only>");
			mRequest.append(globalPrefs.network_wifi_only ? 1 : 0);
			mRequest.append("</network_wifi_only>\n");
			
			// write days prefs
			TimePreferences.TimeSpan[] weekPrefs = globalPrefs.cpu_times.week_prefs;
			for (int i = 0; i < weekPrefs.length; i++) {
				TimePreferences.TimeSpan timeSpan = weekPrefs[i];
				if (timeSpan == null) continue;
				mRequest.append("  <day_prefs>\n    <day_of_week>");
				mRequest.append(i);
				mRequest.append("</day_of_week>\n    <start_hour>");
				mRequest.append(timeSpan.start_hour);
				mRequest.append("</start_hour>\n    <end_hour>");
				mRequest.append(timeSpan.end_hour);
				mRequest.append("</end_hour>\n  </day_prefs>\n");
			}
			
			weekPrefs = globalPrefs.net_times.week_prefs;
			for (int i = 0; i < weekPrefs.length; i++) {
				TimePreferences.TimeSpan timeSpan = weekPrefs[i];
				if (timeSpan == null) continue;
				mRequest.append("  <day_prefs>\n    <day_of_week>");
				mRequest.append(i);
				mRequest.append("</day_of_week>\n    <net_start_hour>");
				mRequest.append(timeSpan.start_hour);
				mRequest.append("</net_start_hour>\n    <net_end_hour>");
				mRequest.append(timeSpan.end_hour);
				mRequest.append("</net_end_hour>\n  </day_prefs>\n");
			}
			
			mRequest.append("</global_preferences>\n</set_global_prefs_override>\n");
			sendRequest(mRequest.toString());
			receiveReply();
			return true;
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in setGlobalPrefsOverrideStruct()", e);
			return false;
		}
	}
	
	public synchronized boolean readGlobalPrefsOverride() {
		try {
			mRequest.setLength(0);
			mRequest.append("<read_global_prefs_override/>");
			sendRequest(mRequest.toString());
			
			// TODO: handle errors
			receiveReply();
			return true;
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in setGlobalPrefsOverrideStruct()", e);
			return false;
		}
	}

	/**
	 * Tells the BOINC core client to exit. 
	 * @return true for success, false for failure
	 */
	public synchronized boolean quit() {
		try {
			sendRequest("<quit/>\n");
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in quit()", e);
			return false;
		}
	}

	/**
	 * Set the network mode
	 * @param mode 1 = always, 2 = auto, 3 = never, 4 = restore
	 * @param duration If duration is zero, mode is permanent. Otherwise revert to
	 *        last permanent mode after duration seconds elapse.
	 * @return true for success, false for failure
	 */
	public synchronized boolean setNetworkMode(int mode, double duration) {
		final String request =
			"<set_network_mode>\n" +
			modeName(mode) + "\n" +
			"<duration>" + duration + "</duration>\n" +
			"</set_network_mode>\n";
		try {
			sendRequest(request);
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in setNetworkMode()", e);
			return false;
		}
	}

	/**
	 * Set the run mode
	 * @param mode 1 = always, 2 = auto, 3 = never, 4 = restore
	 * @param duration If duration is zero, mode is permanent. Otherwise revert to
	 *        last permanent mode after duration seconds elapse.
	 * @return true for success, false for failure
	 */
	public synchronized boolean setRunMode(int mode, double duration) {
		final String request =
			"<set_run_mode>\n" +
			modeName(mode) + "\n" +
			"<duration>" + duration + "</duration>\n" +
			"</set_run_mode>\n";
		try {
			sendRequest(request);
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in setRunMode()", e);
			return false;
		}
	}

	/**
	 * Triggers operation on transfer in BOINC core client
	 * @param operation operation to be triggered
	 * @param projectUrl master URL of project
	 * @param fileName name of the file
	 * @return true for success, false for failure
	 */
	public synchronized boolean transferOp(int operation, String projectUrl, String fileName) {
		try {
			String opTag;
			switch (operation) {
			case TRANSFER_RETRY:
				opTag = "retry_file_transfer";
				break;
			case TRANSFER_ABORT:
				opTag = "abort_file_transfer";
				break;
			default:
				if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "transferOp() - unsupported operation: " + operation);
				return false;
			}
			mRequest.setLength(0);
			mRequest.append("<");
			mRequest.append(opTag);
			mRequest.append(">\n   <project_url>");
			mRequest.append(projectUrl);
			mRequest.append("</project_url>\n   <filename>");
			mRequest.append(fileName);
			mRequest.append("</filename>\n</");
			mRequest.append(opTag);
			mRequest.append(">\n");
			sendRequest(mRequest.toString());
			
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in transferOp()", e);
			return false;
		}
	}
	
	/**
	 * Triggers operation on task in BOINC core client
	 * @param operation operation to be triggered
	 * @param projectUrl master URL of project
	 * @param fileName name of the file
	 * @return true for success, false for failure
	 */
	public boolean resultOp(int operation, String projectUrl, String resultName) {
		try {
			String opTag;
			switch (operation) {
			case RESULT_SUSPEND:
				opTag = "suspend_result";
				break;
			case RESULT_RESUME:
				opTag = "resume_result";
				break;
			case RESULT_ABORT:
				opTag = "abort_result";
				break;
			default:
				if(edu.berkeley.boinc.utils.Logging.LOGLEVEL <= 4) Log.e(Logging.TAG, "resultOp() - unsupported operation: " + operation);
				return false;
			}
			mRequest.setLength(0);
			mRequest.append("<");
			mRequest.append(opTag);
			mRequest.append(">\n   <project_url>");
			mRequest.append(projectUrl);
			mRequest.append("</project_url>\n   <name>");
			mRequest.append(resultName);
			mRequest.append("</name>\n</");
			mRequest.append(opTag);
			mRequest.append(">\n");
			sendRequest(mRequest.toString());
			
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		}
		catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in transferOp()", e);
			return false;
		}
	}
	
	public synchronized boolean setCcConfig(String ccConfig) {
		final String request =
				"<set_cc_config>\n" +
						ccConfig + 
				"\n</set_cc_config>\n";
			try {
				sendRequest(request);
				SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
				if (parser == null)
					return false;
				mLastErrorMessage = parser.getErrorMessage();
				return parser.result();
			}
			catch (IOException e) {
				if(Logging.WARNING) Log.w(Logging.TAG, "error in setRunMode()", e);
				return false;
			}
		
	}
	
	public synchronized String getCcConfig() {
		//TODO: needs proper parsing
		try {
			mRequest.setLength(0);
			mRequest.append("<get_cc_config/>");
			
			sendRequest(mRequest.toString());
			String reply = receiveReply();
			Log.d(Logging.TAG, reply);
			return reply;
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in getCcConfig()", e);
			return "";
		}
	}
	
	public synchronized Boolean readCcConfig() {
		try {
			mRequest.setLength(0);
			mRequest.append("<read_cc_config/>");

			sendRequest(mRequest.toString());
			SimpleReplyParser parser = SimpleReplyParser.parse(receiveReply());
			if (parser == null)
				return false;
			mLastErrorMessage = parser.getErrorMessage();
			return parser.result();
		} catch (IOException e) {
			if(Logging.WARNING) Log.w(Logging.TAG, "error in readCcConfig()", e);
			return false;
		}
	}
}
