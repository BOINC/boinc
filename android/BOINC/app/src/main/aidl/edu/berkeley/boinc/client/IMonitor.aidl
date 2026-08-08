/*******************************************************************************
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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
package edu.berkeley.boinc.client;

import java.util.List;
import edu.berkeley.boinc.rpc.AccountOut;
import edu.berkeley.boinc.rpc.AccountManager;
import edu.berkeley.boinc.rpc.AccountIn;
import edu.berkeley.boinc.rpc.ProjectConfig;
import edu.berkeley.boinc.rpc.AcctMgrInfo;
import edu.berkeley.boinc.rpc.Message;
import edu.berkeley.boinc.rpc.Notice;
import edu.berkeley.boinc.rpc.Transfer;
import edu.berkeley.boinc.rpc.GlobalPreferences;
import edu.berkeley.boinc.rpc.HostInfo;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.Result;
import edu.berkeley.boinc.rpc.ImageWrapper;
import edu.berkeley.boinc.utils.ErrorCodeDescription;

interface IMonitor {
/////// client interface //////////////////////////////////////////
// Data flow: IMonitor -> Monitor -> ClientInterfaceImplementation -> RpcClient
// Eg.: IMonitor.setDomainName() -> Monitor.setDomainName() -> ClientInterfaceImplementation.setDomainName() -> RpcClient.setDomainNameRpc()
boolean attachProject(in String url, in String projectName, in String authenticator); // implement: call clientInterface.attachProject(url, projectName, authenticator);
boolean checkProjectAttached(in String url);       // implement: call clientInterface.checkProjectAttached(url);
AccountOut lookupCredentials(in AccountIn credentials);  // implement: call clientInterface.lookupCredentials(credentials);
boolean projectOp(in int status, String url);             // implement: call clientInterface.projectOp(RpcClient.PROJECT_DETACH,url);
boolean resultOp(in int op, in String url, in String name);      // implement: call clientInterface.resultOp(int, String, String);
AccountOut createAccountPolling(in AccountIn information);  // implement: call clientInterface.createAccountPolling(information);
String readAuthToken(in String path);               // implement: call clientInterface.readAuthToken(String);
ProjectConfig getProjectConfigPolling(in String url);    // implement: call clientInterface.getProjectConfigPolling(url);
ErrorCodeDescription addAcctMgrErrorNum(in String url, in String userName, in String pwd);  // implement: return clientInterface.addAcctMgr(url, userName, pwd).error_num; check return null!=clientInterface.addAcctMgr(url, userName, pwd)
AcctMgrInfo getAcctMgrInfo();               // implement: call clientInterface.getAcctMgrInfo();
boolean synchronizeAcctMgr(in String url);         // implement: call clientInterface.synchronizeAcctMgr(String);
boolean setRunMode(in int mode);                // implement: call clientInterface.setRunMode(Integer);
boolean setNetworkMode(in int mode);            // implement: call clientInterface.setNetworkMode(Integer);
List<edu.berkeley.boinc.rpc.Message> getEventLogMessages(in int seq, in int num);  // implement: call clientInterface.getEventLogMessages(int, Integer);
List<edu.berkeley.boinc.rpc.Message> getMessages(in int seq);        // implement: call clientInterface.getMessages(Integer);
List<Notice> getNotices(in int seq);          // implement: call clientInterface.getNotices(int);
boolean setCcConfig(in String config);                // implement: call clientInterface.setCcConfig(String);
boolean setGlobalPreferences(in GlobalPreferences pref);   // implement: call clientInterface.setGlobalPreferences(GlobalPreferences);
boolean transferOperation(in List<Transfer> list, in int op);  // implement: call clientInterface.transferOperation(ArrayList<transfer>, int);
List<Notice> getServerNotices();        // implement: call clientInterface.getServerNotices()
boolean runBenchmarks();
List<ProjectInfo> getAttachableProjects();  // clientInterface.getAttachableProjects();
List<AccountManager> getAccountManagers();  // clientInterface.getAccountManagers();
ProjectInfo getProjectInfo(String url);  // clientInterface.getProjectInfo(String url);
boolean setDomainName(in String deviceName);            // clientInterface.setDomainName(String deviceName);

/////// general //////////////////////////////////////////
boolean boincMutexAcquired();				// implement: call Monitor.boincMutexAcquired();
void forceRefresh();                        // implement: call Monitor.forceRefresh();
boolean isStationaryDeviceSuspected();               // implement: call Monitor.getDeviceStatus().isStationaryDevice();
int getBatteryChargeStatus();           // implement: return getDeviceStatus().getStatus().battery_charge_pct;
String getAuthFilePath();               // implement: return Monitor.getAuthFilePath();
int getBoincPlatform();                        // should be not necessary to be implemented as monitor interface
void cancelNoticeNotification();
boolean quitClient();
/////// client status //////////////////////////////////////////
boolean getAcctMgrInfoPresent();  // clientStatus.getAcctMgrInfo().present;
int getSetupStatus();         // clientStatus.setupStatus;
int getComputingStatus();     // clientStatus.computingStatus;
int getComputingSuspendReason(); // clientStatus.computingSuspendReason;
int getNetworkSuspendReason();   // clientStatus.networkSuspendReason;
String getCurrentStatusTitle(); // status.getCurrentStatusTitle()
String getCurrentStatusDescription(); // status.getCurrentStatusDescription()
HostInfo getHostInfo();            // clientStatus.getHostInfo()
GlobalPreferences getPrefs();        // clientStatus.getPrefs()
List<Project> getProjects();    // clientStatus.getProjects();
AcctMgrInfo getClientAcctMgrInfo();   // clientStatus.getAcctMgrInfo();
List<Transfer> getTransfers();   // clientStatus.getTransfers();
List<Result> getTasks(in int start, in int count, in boolean isActive);          // clientStatus.getTasks(int, int, boolean);
int getTasksCount(); // clientStatus.getTasksCount();
Bitmap getProjectIconByName(in String name);  // clientStatus.getProjectIconByName(entries.get(position).project_name);
Bitmap getProjectIcon(in String id);        // clientStatus.getProjectIcon(entries.get(position).id);
String getProjectStatus(in String url);   // clientStatus.getProjectStatus(url);
List<Notice> getRssNotices();             // clientStatus.getRssNotices();
List<ImageWrapper> getSlideshowForProject(in String url);   // clientStatus.getSlideshowForProject(url);

////// app preference ////////////////////////////////////////////
void setAutostart(in boolean isAutoStart);          // Monitor.getAppPrefs().setAutostart(boolean);
void setShowNotificationForNotices(in boolean isShow);   // Monitor.getAppPrefs().setShowNotificationForNotices(boolean);
void setShowNotificationDuringSuspend(in boolean isShow);   // Monitor.getAppPrefs().setShowNotificationDuringSuspend(boolean);
boolean getShowAdvanced();           // Monitor.getAppPrefs().getShowAdvanced();
boolean getIsRemote();              // Monitor.getAppPrefs().getIsRemote();
boolean getAutostart();              // Monitor.getAppPrefs().getAutostart();
boolean getShowNotificationForNotices();       // Monitor.getAppPrefs().getShowNotificationForNotices();
boolean getShowNotificationDuringSuspend();       // Monitor.getAppPrefs().getShowNotificationDuringSuspend();
int getLogLevel();                   // Monitor.getAppPrefs().getLogLevel();
void setLogLevel(in int level);               // Monitor.getAppPrefs().setLogLevel(int);
List<String> getLogCategories();
void setLogCategories(in List<String> categories);
void setPowerSourceAc(in boolean src);      // Monitor.getAppPrefs().setPowerSourceAc(boolean);
void setPowerSourceUsb(in boolean src);     // Monitor.getAppPrefs().setPowerSourceUsb(boolean);
void setPowerSourceWireless(in boolean src); // Monitor.getAppPrefs().setPowerSourceWireless(boolean);
boolean getStationaryDeviceMode();           // Monitor.getAppPrefs().getStationaryDeviceMode();
boolean getPowerSourceAc();
boolean getPowerSourceUsb();
boolean getPowerSourceWireless();
void setShowAdvanced(in boolean isShow);
void setIsRemote(in boolean isRemote);
void setStationaryDeviceMode(in boolean mode);
boolean getSuspendWhenScreenOn();
void setSuspendWhenScreenOn(in boolean swso);
}
