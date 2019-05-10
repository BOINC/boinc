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
import edu.berkeley.boinc.rpc.AccountManager;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.AcctMgrRPCReply;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectAttachReply;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.utils.BOINCErrors;
import edu.berkeley.boinc.utils.Logging;

/**
 * Class implements RPC commands with the client
 * extends RpcClient with polling, re-try and other mechanisms
 * Most functions can block executing thread, do not call them from UI thread!
 */
public class ClientInterfaceImplementation extends RpcClient {

    // interval between polling retries in ms
    private final Integer minRetryInterval = 1000;

    /**
     * Reads authentication key from specified file path and authenticates GUI for advanced RPCs with the client
     *
     * @param authFilePath absolute path to file containing gui authentication key
     * @return success
     */
    public Boolean authorizeGuiFromFile(String authFilePath) {
        String authToken = readAuthToken(authFilePath);
        return authorize(authToken);
    }

    /**
     * Sets run mode of BOINC client
     *
     * @param mode see class BOINCDefs
     * @return success
     */
    public Boolean setRunMode(Integer mode) {
        return setRunMode(mode, 0);
    }

    /**
     * Sets network mode of BOINC client
     *
     * @param mode see class BOINCDefs
     * @return success
     */
    public Boolean setNetworkMode(Integer mode) {
        return setNetworkMode(mode, 0);
    }

    /**
     * Writes the given GlobalPreferences via RPC to the client. After writing, the active preferences are read back and written to ClientStatus.
     *
     * @param prefs new target preferences for the client
     * @return success
     */
    public Boolean setGlobalPreferences(GlobalPreferences prefs) {

        // try to get current client status from monitor
        ClientStatus status;
        try {
            status = Monitor.getClientStatus();
        } catch (Exception e) {
            if (Logging.WARNING) {
                Log.w(Logging.TAG, "Monitor.setGlobalPreferences: Could not load data, clientStatus not initialized.");
            }
            return false;
        }

        Boolean retval1 = setGlobalPrefsOverrideStruct(prefs); //set new override settings
        Boolean retval2 = readGlobalPrefsOverride(); //trigger reload of override settings
        if (!retval1 || !retval2) {
            return false;
        }
        GlobalPreferences workingPrefs = getGlobalPrefsWorkingStruct();
        if (workingPrefs != null) {
            status.setPrefs(workingPrefs);
            return true;
        }
        return false;
    }

    /**
     * Reads authentication token for GUI RPC authentication from file
     *
     * @param authFilePath absolute path to file containing GUI RPC authentication
     * @return GUI RPC authentication code
     */
    public String readAuthToken(String authFilePath) {
        StringBuilder fileData = new StringBuilder(100);
        char[] buf = new char[1024];
        int read;
        try {
            File authFile = new File(authFilePath);
            BufferedReader br = new BufferedReader(new FileReader(authFile));
            while ((read = br.read(buf)) != -1) {
                String readData = String.valueOf(buf, 0, read);
                fileData.append(readData);
                buf = new char[1024];
            }
            br.close();
        } catch (FileNotFoundException fnfe) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "auth file not found", fnfe);
            }
        } catch (IOException ioe) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "ioexception", ioe);
            }
        }

        String authKey = fileData.toString();
        if (Logging.DEBUG) {
            Log.d(Logging.TAG, "authentication key acquired. length: " + authKey.length());
        }
        return authKey;
    }

    /**
     * Reads project configuration for specified master URL.
     *
     * @param url master URL of the project
     * @return project configuration information
     */
    public ProjectConfig getProjectConfigPolling(String url) {
        ProjectConfig config = null;

        Boolean success = getProjectConfig(url); //asynchronous call
        if (success) { //only continue if attach command did not fail
            // verify success of getProjectConfig with poll function
            Boolean loop = true;
            while (loop) {
                loop = false;
                try {
                    Thread.sleep(minRetryInterval);
                } catch (Exception ignored) {
                }
                config = getProjectConfigPoll();
                if (config == null) {
                    if (Logging.ERROR) {
                        Log.e(Logging.TAG, "ClientInterfaceImplementation.getProjectConfigPolling: returned null.");
                    }
                    return null;
                }
                if (config.error_num == BOINCErrors.ERR_IN_PROGRESS) {
                    loop = true; //no result yet, keep looping
                } else {
                    //final result ready
                    if (config.error_num == 0) {
                        if (Logging.DEBUG) {
                            Log.d(Logging.TAG,
                                    "ClientInterfaceImplementation.getProjectConfigPolling: ProjectConfig retrieved: " +
                                            config.name);
                        }
                    } else {
                        if (Logging.DEBUG) {
                            Log.d(Logging.TAG,
                                    "ClientInterfaceImplementation.getProjectConfigPolling: final result with error_num: " +
                                            config.error_num);
                        }
                    }
                }
            }
        }
        return config;
    }

    /**
     * Attaches project, requires authenticator
     *
     * @param url           URL of project to be attached, either masterUrl(HTTP) or webRpcUrlBase(HTTPS)
     * @param projectName   name of project as shown in the manager
     * @param authenticator user authentication key, has to be obtained first
     * @return success
     */

    public Boolean attachProject(String url, String projectName, String authenticator) {
        Boolean success = projectAttach(url, authenticator, projectName); //asynchronous call to attach project
        if (success) {
            // verify success of projectAttach with poll function
            ProjectAttachReply reply = projectAttachPoll();
            while (reply != null && reply.error_num ==
                    BOINCErrors.ERR_IN_PROGRESS) { // loop as long as reply.error_num == BOINCErrors.ERR_IN_PROGRESS
                try {
                    Thread.sleep(minRetryInterval);
                } catch (Exception ignored) {
                }
                reply = projectAttachPoll();
            }
            return (reply != null && reply.error_num == BOINCErrors.ERR_OK);
        } else if (Logging.DEBUG) {
            Log.d(Logging.TAG, "rpc.projectAttach failed.");
        }
        return false;
    }

    /**
     * Checks whether project of given master URL is currently attached to BOINC client
     *
     * @param url master URL of the project
     * @return true if attached
     */

    public Boolean checkProjectAttached(String url) {
        try {
            ArrayList<Project> attachedProjects = getProjectStatus();
            for (Project project : attachedProjects) {
                if (Logging.DEBUG) {
                    Log.d(Logging.TAG, project.master_url + " vs " + url);
                }
                if (project.master_url.equals(url)) {
                    return true;
                }
            }
        } catch (Exception e) {
            if (Logging.ERROR) {
                Log.e(Logging.TAG, "ClientInterfaceImplementation.checkProjectAttached() error: ", e);
            }
        }
        return false;
    }

    /**
     * Looks up account credentials for given user data.
     * Contains authentication key for project attachment.
     *
     * @param credentials account credentials
     * @return account credentials
     */

    public AccountOut lookupCredentials(AccountIn credentials) {
        AccountOut auth = null;
        Boolean success = lookupAccount(credentials); //asynch
        if (success) {
            // get authentication token from lookupAccountPoll
            Boolean loop = true;
            while (loop) {
                loop = false;
                try {
                    Thread.sleep(minRetryInterval);
                } catch (Exception ignored) {
                }
                auth = lookupAccountPoll();
                if (auth == null) {
                    if (Logging.ERROR) {
                        Log.e(Logging.TAG, "ClientInterfaceImplementation.lookupCredentials: returned null.");
                    }
                    return null;
                }
                if (auth.error_num == BOINCErrors.ERR_IN_PROGRESS) {
                    loop = true; //no result yet, keep looping
                } else {
                    //final result ready
                    if (auth.error_num == 0) {
                        if (Logging.DEBUG) {
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.lookupCredentials: authenticator retrieved.");
                        }
                    } else {
                        if (Logging.DEBUG) {
                            Log.d(Logging.TAG,
                                    "ClientInterfaceImplementation.lookupCredentials: final result with error_num: " +
                                            auth.error_num);
                        }
                    }
                }
            }
        } else if (Logging.DEBUG) {
            Log.d(Logging.TAG, "rpc.lookupAccount failed.");
        }
        return auth;
    }

    /**
     * Sets cc_config.xml entries and triggers activation in BOINC client.
     * Used to set debug log flags.
     *
     * @param ccConfig string of all cc_config flags
     */
    public void setCcConfigAndActivate(String ccConfig) {
        if (Logging.DEBUG)
            Log.d(Logging.TAG, "Monitor.setCcConfig: current cc_config: " + getCcConfig());
        if (Logging.DEBUG)
            Log.d(Logging.TAG, "Monitor.setCcConfig: setting new cc_config: " + ccConfig);
        setCcConfig(ccConfig);
        readCcConfig();
    }

    /**
     * Runs transferOp for a list of given transfers.
     * E.g. batch pausing of transfers
     *
     * @param transfers list of transfered operation gets executed for
     * @param operation see BOINCDefs
     * @return success
     */

    public Boolean transferOperation(ArrayList<Transfer> transfers, int operation) {
        Boolean success = true;
        for (Transfer transfer : transfers) {
            success = success && transferOp(operation, transfer.project_url, transfer.name);
            if (Logging.DEBUG) Log.d(Logging.TAG, "transfer: " + transfer.name + " " + success);
        }
        return success;
    }

    /**
     * Creates account for given user information and returns account credentials if successful.
     *
     * @param information account credentials
     * @return account credentials (see status inside, to check success)
     */

    public AccountOut createAccountPolling(AccountIn information) {
        AccountOut auth = null;

        Boolean success = createAccount(information); //asynchronous call to attach project
        if (success) {
            Boolean loop = true;
            while (loop) {
                loop = false;
                try {
                    Thread.sleep(minRetryInterval);
                } catch (Exception ignored) {
                }
                auth = createAccountPoll();
                if (auth == null) {
                    if (Logging.ERROR)
                        Log.e(Logging.TAG, "ClientInterfaceImplementation.createAccountPolling: returned null.");
                    return null;
                }
                if (auth.error_num == BOINCErrors.ERR_IN_PROGRESS) {
                    loop = true; //no result yet, keep looping
                } else {
                    //final result ready
                    if (auth.error_num == 0) {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.createAccountPolling: authenticator retrieved.");
                    } else {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.createAccountPolling: final result with error_num: " + auth.error_num);
                    }
                }
            }
        } else {
            if (Logging.DEBUG) Log.d(Logging.TAG, "rpc.createAccount returned false.");
        }
        return auth;
    }

    /**
     * Adds account manager to BOINC client.
     * There can only be a single acccount manager be active at a time.
     *
     * @param url      URL of account manager
     * @param userName user name
     * @param pwd      password
     * @return status of attachment
     */

    public AcctMgrRPCReply addAcctMgr(String url, String userName, String pwd) {
        AcctMgrRPCReply reply = null;
        Boolean success = acctMgrRPC(url, userName, pwd);
        if (success) {
            Boolean loop = true;
            while (loop) {
                reply = acctMgrRPCPoll();
                if (reply == null || reply.error_num != BOINCErrors.ERR_IN_PROGRESS) {
                    loop = false;
                    //final result ready
                    if (reply == null) {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.addAcctMgr: failed, reply null.");
                    } else {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.addAcctMgr: returned " + reply.error_num);
                    }
                } else {
                    try {
                        Thread.sleep(minRetryInterval);
                    } catch (Exception ignored) {
                    }
                }
            }
        } else {
            if (Logging.DEBUG) Log.d(Logging.TAG, "rpc.acctMgrRPC returned false.");
        }
        return reply;
    }


    /**
     * Synchronized BOINC client projects with information of account manager.
     * Sequence copied from BOINC's desktop manager.
     *
     * @param url URL of account manager
     * @return success
     */
    public Boolean synchronizeAcctMgr(String url) {

        // 1st get_project_config for account manager url
        Boolean success = getProjectConfig(url);
        ProjectConfig reply;
        if (success) {
            Boolean loop = true;
            while (loop) {
                loop = false;
                try {
                    Thread.sleep(minRetryInterval);
                } catch (Exception ignored) {
                }
                reply = getProjectConfigPoll();
                if (reply == null) {
                    if (Logging.ERROR)
                        Log.e(Logging.TAG, "ClientInterfaceImplementation.synchronizeAcctMgr: getProjectConfigreturned null.");
                    return null;
                }
                if (reply.error_num == BOINCErrors.ERR_IN_PROGRESS) {
                    loop = true; //no result yet, keep looping
                } else {
                    //final result ready
                    if (reply.error_num == 0) {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.synchronizeAcctMgr: project config retrieved.");
                    } else {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.synchronizeAcctMgr: final result with error_num: " + reply.error_num);
                    }
                }
            }
        } else {
            if (Logging.DEBUG) Log.d(Logging.TAG, "rpc.getProjectConfig returned false.");
        }

        // 2nd acct_mgr_rpc with <use_config_file/>
        AcctMgrRPCReply reply2;
        success = acctMgrRPC(); //asynchronous call to synchronize account manager
        if (success) {
            Boolean loop = true;
            while (loop) {
                loop = false;
                try {
                    Thread.sleep(minRetryInterval);
                } catch (Exception ignored) {
                }
                reply2 = acctMgrRPCPoll();
                if (reply2 == null) {
                    if (Logging.ERROR)
                        Log.e(Logging.TAG, "ClientInterfaceImplementation.synchronizeAcctMgr: acctMgrRPCPoll returned null.");
                    return null;
                }
                if (reply2.error_num == BOINCErrors.ERR_IN_PROGRESS) {
                    loop = true; //no result yet, keep looping
                } else {
                    //final result ready
                    if (reply2.error_num == 0) {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.synchronizeAcctMgr: acct mngr reply retrieved.");
                    } else {
                        if (Logging.DEBUG)
                            Log.d(Logging.TAG, "ClientInterfaceImplementation.synchronizeAcctMgr: final result with error_num: " + reply2.error_num);
                    }
                }
            }
        } else {
            if (Logging.DEBUG) Log.d(Logging.TAG, "rpc.acctMgrRPC returned false.");
        }

        return true;
    }

    @Override
    public boolean setCcConfig(String ccConfig) {
        // set CC config and trigger re-read.
        super.setCcConfig(ccConfig);
        return super.readCcConfig();
    }

    /**
     * Returns List of event log messages
     *
     * @param seqNo  lower bound of sequence number
     * @param number number of messages returned max, can be less
     * @return list of messages
     */

    // returns given number of client messages, older than provided seqNo
    // if seqNo <= 0 initial data retrieval
    public ArrayList<Message> getEventLogMessages(int seqNo, int number) {
        // determine oldest message seqNo for data retrieval
        int lowerBound;
        if (seqNo > 0) lowerBound = seqNo - number - 2;
        else
            lowerBound = getMessageCount() - number - 1; // can result in >number results, if client writes message btwn. here and rpc.getMessages!

        // less than desired number of messsages available, adapt lower bound
        if (lowerBound < 0) lowerBound = 0;
        ArrayList<Message> msgs = getMessages(lowerBound); // returns ever messages with seqNo > lowerBound
        if (msgs == null)
            msgs = new ArrayList<>(); // getMessages might return null in case of parsing or IO error

        if (seqNo > 0) {
            // remove messages that are >= seqNo
            Iterator<Message> it = msgs.iterator();
            while (it.hasNext()) {
                Message tmp = it.next();
                if (tmp.seqno >= seqNo) it.remove();
            }
        }

        if (!msgs.isEmpty())
            if (Logging.DEBUG)
                Log.d(Logging.TAG, "getEventLogMessages: returning array with " + msgs.size() + " entries. for lowerBound: " + lowerBound + " at 0: " + msgs.get(0).seqno + " at " + (msgs.size() - 1) + ": " + msgs.get(msgs.size() - 1).seqno);
            else if (Logging.DEBUG)
                Log.d(Logging.TAG, "getEventLogMessages: returning empty array for lowerBound: " + lowerBound);
        return msgs;
    }

    /**
     * Returns list of projects from all_projects_list.xml that...
     * - support Android
     * - support CPU architecture
     * - are not yet attached
     *
     * @return list of attachable projects
     */
    public ArrayList<ProjectInfo> getAttachableProjects(String boincPlatformName, String boincAltPlatformName) {
        if (Logging.DEBUG)
            Log.d(Logging.TAG, "getAttachableProjects for platform: " + boincPlatformName + " or " + boincAltPlatformName);

        ArrayList<ProjectInfo> allProjectsList = getAllProjectsList(); // all_proejcts_list.xml
        ArrayList<Project> attachedProjects = getState().projects; // currently attached projects

        ArrayList<ProjectInfo> attachableProjects = new ArrayList<>(); // array to be filled and returned

        if (allProjectsList == null || attachedProjects == null) return null;

        //filter projects that do not support Android
        for (ProjectInfo candidate : allProjectsList) {
            // check whether already attached
            Boolean alreadyAttached = false;
            for (Project attachedProject : attachedProjects) {
                if (attachedProject.master_url.equals(candidate.url)) {
                    alreadyAttached = true;
                    break;
                }
            }
            if (alreadyAttached) continue;

            // project is not yet attached, check whether it supports CPU architecture
            for (String supportedPlatform : candidate.platforms) {
                if (supportedPlatform.contains(boincPlatformName) ||
                   (!boincAltPlatformName.isEmpty() && supportedPlatform.contains(boincAltPlatformName))) {
                    // project is not yet attached and does support platform
                    // add to list, if not already in it
                    if (!attachableProjects.contains(candidate)) attachableProjects.add(candidate);
                    break;
                }
            }
        }

        if (Logging.DEBUG)
            Log.d(Logging.TAG, "getAttachableProjects: number of candidates found: " + attachableProjects.size());
        return attachableProjects;
    }

    /**
     * Returns list of account managers from all_projects_list.xml
     *
     * @return list of account managers
     */
    public ArrayList<AccountManager> getAccountManagers() {
        ArrayList<AccountManager> accountManagers = getAccountManagersList(); // from all_proejcts_list.xml

        if (Logging.DEBUG)
            Log.d(Logging.TAG, "getAccountManagers: number of account managers found: " + accountManagers.size());
        return accountManagers;
    }

    public ProjectInfo getProjectInfo(String url) {
        ArrayList<ProjectInfo> allProjectsList = getAllProjectsList(); // all_proejcts_list.xml
        for (ProjectInfo tmp : allProjectsList) {
            if (tmp.url.equals(url)) return tmp;
        }
        if (Logging.ERROR) Log.e(Logging.TAG, "getProjectInfo: could not find info for: " + url);
        return null;
    }

    public boolean setDomainName(String deviceName) {
        boolean success = setDomainNameRpc(deviceName);
        if (Logging.DEBUG) Log.d(Logging.TAG, "setDomainName: success " + success);
        return success;
    }

}
