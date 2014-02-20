package edu.berkeley.boinc.client;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import android.util.Log;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.AcctMgrRPCReply;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectAttachReply;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.utils.Logging;

/**
 * Class implements RPC commands with the client
 * extends RpcClient with polling, re-try and other mechanisms
 * Most functions can block executing thread, do not call them from UI thread!
 */
public class ClientInterfaceImplementation extends RpcClient{

	// maximum polling retry duration in ms
	private final Integer maxRetryDuration = 3000;
	// interval between polling retries in ms
	private final Integer minRetryInterval = 500;
    
    /**
     * Reads authentication key from specified file path and authenticates GUI for advanced RPCs with the client
     * @param authFilePath absolute path to file containing gui authentication key
     * @return success
     */
    public Boolean authorizeGuiFromFile(String authFilePath) {
    	String authToken = readAuthToken(authFilePath);
		return authorize(authToken); 
    }

    /**
     * Sets run mode of BOINC client
     * @param mode see class BOINCDefs
     * @return success
     */
	public Boolean setRunMode(Integer mode) {
		return setRunMode(mode, 0);
	}
	
    /**
     * Sets network mode of BOINC client
     * @param mode see class BOINCDefs
     * @return success
     */
	public Boolean setNetworkMode(Integer mode) {
		return setNetworkMode(mode, 0);
	}
	
	/**
	 * Writes the given GlobalPreferences via RPC to the client. After writing, the active preferences are read back and written to ClientStatus.
	 * @param prefs new target preferences for the client
	 * @return success
	 */
	public Boolean setGlobalPreferences(GlobalPreferences prefs) {

		// try to get current client status from monitor
		ClientStatus status = null;
		try{
			status  = Monitor.getClientStatus();
		} catch (Exception e){
			if(Logging.WARNING) Log.w(Logging.TAG,"Monitor.setGlobalPreferences: Could not load data, clientStatus not initialized.");
			return false;
		}

		Boolean retval1 = setGlobalPrefsOverrideStruct(prefs); //set new override settings
		Boolean retval2 = readGlobalPrefsOverride(); //trigger reload of override settings
		if(!retval1 || !retval2) {
			return false;
		}
		GlobalPreferences workingPrefs = getGlobalPrefsWorkingStruct();
		if(workingPrefs != null){
			status.setPrefs(workingPrefs);
			return true;
		}
		return false;
	}

	/**
	 * Reads authentication token for GUI RPC authentication from file
	 * @param authFilePath absolute path to file containing GUI RPC authentication
	 * @return GUI RPC authentication code
	 */
	public String readAuthToken(String authFilePath) {
    	StringBuffer fileData = new StringBuffer(100);
    	char[] buf = new char[1024];
    	int read = 0;
    	try{
    		File authFile = new File(authFilePath);
    		BufferedReader br = new BufferedReader(new FileReader(authFile));
    		while((read=br.read(buf)) != -1){
    	    	String readData = String.valueOf(buf, 0, read);
    	    	fileData.append(readData);
    	    	buf = new char[1024];
    	    }
    		br.close();
    	}
    	catch (FileNotFoundException fnfe) {
    		if(Logging.ERROR) Log.e(Logging.TAG, "auth file not found",fnfe);
    	}
    	catch (IOException ioe) {
    		if(Logging.ERROR) Log.e(Logging.TAG, "ioexception",ioe);
    	}

		String authKey = fileData.toString();
		if(Logging.DEBUG) Log.d(Logging.TAG, "authentication key acquired. length: " + authKey.length());
		return authKey;
	}
	
	/**
	 * Reads project configuration for specified master URL.
	 * @param url master URL of the project
	 * @return project configuration information
	 */
	public ProjectConfig getProjectConfigPolling(String url) {
		ProjectConfig config = null;
		
    	Boolean success = getProjectConfig(url); //asynchronous call
    	if(success) { //only continue if attach command did not fail
    		// verify success of getProjectConfig with poll function
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			config = getProjectConfigPoll();
    			if(config==null) {
    				return null;
    			}
    			if (config.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			} else {
    				//final result ready
    				if(config.error_num == 0) { 
        				if(Logging.DEBUG) Log.d(Logging.TAG, "ProjectConfig retrieved: " + config.name);
    				} else {
    					if(Logging.DEBUG) Log.d(Logging.TAG, "final result with error_num: " + config.error_num);
    				}
    			}
    		}
    	}
		return config;
	}
	
	/**
	 * Attaches project, requires authenticator
	 * @param url URL of project to be attached, either masterUrl(HTTP) or webRpcUrlBase(HTTPS)
	 * @param projectName name of project as shown in the manager
	 * @param authenticator user authentication key, has to be obtained first
	 * @return success
	 */
	
	public Boolean attachProject(String url, String projectName, String authenticator) {
    	Boolean success = false;
    	success = projectAttach(url, authenticator, projectName); //asynchronous call to attach project
    	if(success) { //only continue if attach command did not fail
    		// verify success of projectAttach with poll function
    		success = false;
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		while(!success && (counter < maxLoops)) {
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			ProjectAttachReply reply = projectAttachPoll();
    			if(reply != null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG, "rpc.projectAttachPoll reply error_num: " + reply.error_num);
    				if(reply.error_num == 0) success = true;
    			}
    		}
    	} else if(Logging.DEBUG) Log.d(Logging.TAG, "rpc.projectAttach failed.");
    	return success;
    }
	
	/**
	 * Checks whether project of given master URL is currently attached to BOINC client
	 * @param url master URL of the project
	 * @return true if attached
	 */
	
	public Boolean checkProjectAttached(String url) {
		Boolean match = false;
		try{
			ArrayList<Project> attachedProjects = getProjectStatus();
			for (Project project: attachedProjects) {
				if(Logging.DEBUG) Log.d(Logging.TAG, project.master_url + " vs " + url);
				if(project.master_url.equals(url)) {
					match = true;
					continue;
				}
			}
		} catch(Exception e){}
		return match;
	}
	
	/**
	 * Looks up account credentials for given user data.
	 * Contains authentication key for project attachment.
	 * @param url URL of project, either masterUrl(HTTP) or webRpcUrlBase(HTTPS)
	 * @param id user ID, can be either name or eMail, see usesName
	 * @param pwd password
	 * @param usesName if true, id represents a user name, if not, the user's email address
	 * @return account credentials
	 */
	
	public AccountOut lookupCredentials(String url, String id, String pwd, Boolean usesName) {
    	AccountOut auth = null;
    	AccountIn credentials = new AccountIn();
    	if(usesName) credentials.user_name = id;
    	else credentials.email_addr = id;
    	credentials.passwd = pwd;
    	credentials.url = url;
    	Boolean success = lookupAccount(credentials); //asynch
    	if(success) { //only continue if lookupAccount command did not fail
    		//get authentication token from lookupAccountPoll
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			auth = lookupAccountPoll();
    			if(auth==null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.lookupAccountPoll.");
    				return null;
    			}
    			if (auth.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(auth.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "credentials verification result, retrieved authenticator.");
    				else Log.d(Logging.TAG, "credentials verification result, error: " + auth.error_num);
    			}
    		}
    	} else if(Logging.DEBUG) Log.d(Logging.TAG, "rpc.lookupAccount failed.");
    	return auth;
    }
	
	/**
	 * Sets cc_config.xml entries and triggers activation in BOINC client.
	 * Used to set debug log flags.
	 * @param ccConfig string of all cc_config flags
	 */
	public void setCcConfigAndActivate(String ccConfig) {
		if(Logging.DEBUG) Log.d(Logging.TAG, "Monitor.setCcConfig: current cc_config: " + getCcConfig());
		if(Logging.DEBUG) Log.d(Logging.TAG, "Monitor.setCcConfig: setting new cc_config: " + ccConfig);
		setCcConfig(ccConfig);
		readCcConfig();
	}
	
	/**
	 * Runs transferOp for a list of given transfers.
	 * E.g. batch pausing of transfers
	 * @param transfers list of transfered operation gets executed for
	 * @param operation see BOINCDefs
	 * @return success
	 */
	
	public Boolean transferOperation(ArrayList<Transfer> transfers, int operation) {
		Boolean success = true;
		for (Transfer transfer: transfers) {
			success = success && transferOp(operation, transfer.project_url, transfer.name);
			if(Logging.DEBUG) Log.d(Logging.TAG, "transfer: " + transfer.name + " " + success);
		}
		return success;
	}
	
	/**
	 * Creates account for given user information and returns account credentials if successful.
	 * @param url master URL of project
	 * @param email email address of user
	 * @param userName user name of user
	 * @param pwd password
	 * @param teamName name of team, account shall get associated to
	 * @return account credentials (see status inside, to check success)
	 */
	
	public AccountOut createAccountPolling(String url, String email, String userName, String pwd, String teamName) {
		AccountIn information = new AccountIn();
		information.url = url;
		information.email_addr = email;
		information.user_name = userName;
		information.passwd = pwd;
		information.team_name = teamName;
		
		AccountOut auth = null;
		
    	Boolean success = createAccount(information); //asynchronous call to attach project
    	if(success) { //only continue if attach command did not fail
    		// verify success of projectAttach with poll function
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			auth = createAccountPoll();
    			if(auth==null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.createAccountPoll.");
    				return null;
    			}
    			if (auth.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(auth.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "account creation result, retrieved authenticator.");
    				else if(Logging.DEBUG) Log.d(Logging.TAG, "account creation result, error: " + auth.error_num);
    			}
    		}
    	} else {if(Logging.DEBUG) Log.d(Logging.TAG,"rpc.createAccount returned false.");}
    	return auth;
	}
	
	/**
	 * Adds account manager to BOINC client.
	 * There can only be a single acccount manager be active at a time.
	 * @param url URL of account manager
	 * @param userName
	 * @param pwd
	 * @return status of attachment
	 */
	
	public AcctMgrRPCReply addAcctMgr(String url, String userName, String pwd) {
		AcctMgrRPCReply reply = null;
    	Boolean success = acctMgrRPC(url, userName, pwd); //asynchronous call to attach account manager
    	if(success) { //only continue if rpc command did not fail
    		// verify success of acctMgrRPC with poll function
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			reply = acctMgrRPCPoll();
    			if(reply == null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.addAcctMgr.");
    				return null;
    			}
    			if (reply.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(reply.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "account manager attach successful.");
    				else if(Logging.DEBUG) Log.d(Logging.TAG, "account manager attach failed, error: " + reply.error_num);
    			}
    		}
    	} else {if(Logging.DEBUG) Log.d(Logging.TAG,"rpc.acctMgrRPC returned false.");}
    	return reply;
	}
	
	
	/**
	 * Synchronized BOINC client projects with information of account manager.
	 * Sequence copied from BOINC's desktop manager.
	 * @param url URL of account manager
	 * @return success
	 */
	public Boolean synchronizeAcctMgr(String url) {

	// 1st get_project_config for account manager url
		Boolean success = false;
		ProjectConfig reply = null;
		success = getProjectConfig(url);
    	if(success) { //only continue if rpc command did not fail
    		// verify success of getProjectConfig with poll function
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			reply = getProjectConfigPoll();
    			if(reply == null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.getProjectConfig.");
    				return false;
    			}
    			if (reply.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(reply.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "getting project config for account manager synchronization successful.");
    				else  {
    					if(Logging.DEBUG) Log.d(Logging.TAG, "getting project config for account manager synchronization failed, error: " + reply.error_num);
    					return false;
    				}
    			}
    		}
    	} else {if(Logging.DEBUG) Log.d(Logging.TAG,"rpc.getProjectConfig returned false.");}
		
    // 2nd acct_mgr_rpc with <use_config_file/>
		success = false;
		AcctMgrRPCReply reply2 = null;
    	success = acctMgrRPC(); //asynchronous call to synchronize account manager
    	if(success) { //only continue if rpc command did not fail
    		// verify success of acctMgrRPC with poll function
    		Integer counter = 0;
    		Integer maxLoops = maxRetryDuration / minRetryInterval;
    		Boolean loop = true;
    		while(loop && (counter < maxLoops)) {
    			loop = false;
    			try {
    				Thread.sleep(minRetryInterval);
    			} catch (Exception e) {}
    			counter ++;
    			reply2 = acctMgrRPCPoll();
    			if(reply2 == null) {
    				if(Logging.DEBUG) Log.d(Logging.TAG,"error in rpc.addAcctMgr.");
    				return false;
    			}
    			if (reply2.error_num == -204) {
    				loop = true; //no result yet, keep looping
    			}
    			else {
    				//final result ready
    				if(reply2.error_num == 0) if(Logging.DEBUG) Log.d(Logging.TAG, "account manager synchronization successful.");
    				else {
    					if(Logging.DEBUG) Log.d(Logging.TAG, "account manager synchronization failed, error: " + reply2.error_num);
    					return false;
    				}
    			}
    		}
    	} else {if(Logging.DEBUG) Log.d(Logging.TAG,"rpc.acctMgrRPC returned false.");}
		
		return true;
	}
	
	@Override
	public synchronized boolean setCcConfig(String ccConfig) {
		// set CC config and trigger re-read.
		super.setCcConfig(ccConfig);
		return super.readCcConfig();
	}

	/**
	 * Returns List of event log messages
	 * @param seqNo lower bound of sequence number
	 * @param number number of messages returned max, can be less
	 * @return list of messages
	 */
	
	// returns given number of client messages, older than provided seqNo
	// if seqNo <= 0 initial data retrieval
	public ArrayList<Message> getEventLogMessages(int seqNo, int number) {
		// determine oldest message seqNo for data retrieval
		int lowerBound = 0;
		if(seqNo > 0) lowerBound = seqNo - number - 2;
		else lowerBound = getMessageCount() - number - 1; // can result in >number results, if client writes message btwn. here and rpc.getMessages!
		
		// less than desired number of messsages available, adapt lower bound
		if(lowerBound < 0) lowerBound = 0;
		ArrayList<Message> msgs= getMessages(lowerBound); // returns ever messages with seqNo > lowerBound
		if(msgs == null) msgs = new ArrayList<Message>(); // getMessages might return null in case of parsing or IO error
		
		if(seqNo > 0) {
			// remove messages that are >= seqNo
			Iterator<Message> it = msgs.iterator();
			while(it.hasNext()) {
				Message tmp = it.next();
				if (tmp.seqno >= seqNo) it.remove();
			}
		}
		
		if(!msgs.isEmpty()) 
			if(Logging.DEBUG) Log.d(Logging.TAG,"getEventLogMessages: returning array with " + msgs.size() + " entries. for lowerBound: " + lowerBound + " at 0: " + msgs.get(0).seqno + " at " + (msgs.size()-1) + ": " + msgs.get(msgs.size()-1).seqno);
		else 
			if(Logging.DEBUG) Log.d(Logging.TAG,"getEventLogMessages: returning empty array for lowerBound: " + lowerBound);
		return msgs;
	}
}
