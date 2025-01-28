// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


#ifndef BOINC_EVENTS_H
#define BOINC_EVENTS_H

// Common Events across GUIs
#define ID_CLOSEWINDOW                          6100
#define ID_CHANGEGUI                            6101
#define ID_OPENWEBSITE                          6102
#define ID_OPENBOINCMANAGER                     6103
#define ID_PERIODICRPCTIMER                     6104
#define ID_DOCUMENTPOLLTIMER                    6105
#define ID_ALERTPOLLTIMER                       6106
#define ID_REFRESHSTATETIMER                    6107
#define ID_WIZARDATTACHPROJECT                  6108
#define ID_WIZARDATTACHACCOUNTMANAGER           6109
#define ID_WIZARDUPDATE                         6110
#define ID_WIZARDDETACH                         6111


//
// Advanced GUI
//

// Advanced Frame
#define ID_ADVANCEDFRAME                        6000
#define ID_STATUSBAR                            6001
#define ID_FRAMENOTEBOOK                        6002
#define ID_FRAMERENDERTIMER                     6004
#define ID_FRAMETASKRENDERTIMER                 6005

// File Menu
//#define ID_CLOSEWINDOW
//#define wxID_EXIT

// View Menu
#define ID_ADVVIEWBASE                          6125
#define ID_ADVNOTICESVIEW                       6125
#define ID_ADVPROJECTSVIEW                      6126
#define ID_ADVTASKSVIEW                         6127
#define ID_ADVTRANSFERSVIEW                     6128
#define ID_ADVSTATISTICSVIEW                    6129
#define ID_ADVRESOURCEUSAGEVIEW                 6130

// Tools Menu
//#define ID_ATTACHWIZARD
//#define ID_WIZARDUPDATE
//#define ID_WIZARDDETACH

// Activity Menu
#define ID_ADVACTIVITYRUNALWAYS                 6010
#define ID_ADVACTIVITYRUNBASEDONPREPERENCES     6011
#define ID_ADVACTIVITYSUSPEND                   6012
#define ID_MENUSEPARATOR1                       6013
#define ID_ADVACTIVITYGPUALWAYS                 6014
#define ID_ADVACTIVITYGPUBASEDONPREPERENCES     6015
#define ID_ADVACTIVITYGPUSUSPEND                6016
#define ID_MENUSEPARATOR2                       6017
#define ID_ADVNETWORKRUNALWAYS                  6018
#define ID_ADVNETWORKRUNBASEDONPREPERENCES      6019
#define ID_ADVNETWORKSUSPEND                    6020

// Advanced Menu
#define ID_OPTIONS                              6050
#define ID_PREFERENCES                          6051
#define ID_SELECTCOMPUTER                       6052
#define ID_SHUTDOWNCORECLIENT                   6053
#define ID_RUNBENCHMARKS                        6054
#define ID_RETRYCOMMUNICATIONS                  6055
#define ID_READCONFIG                           6056
#define ID_READPREFERENCES                      6057
#define ID_EVENTLOG                             6058
#define ID_LAUNCHNEWINSTANCE                    6059
#define ID_DIAGNOSTICLOGFLAGS                   6060
#define ID_SELECTCOLUMNS                        6061
#define ID_EXCLUSIVE_APPS                       6063

// Help Menu
#define ID_HELPBOINC                            6035  // Locked: Used by manager_links.php
#define ID_HELPBOINCWEBSITE                     6024  // Locked: Used by manager_links.php
#define ID_HELPBOINCMANAGER                     6025  // Locked: Used by manager_links.php
#define ID_CHECK_VERSION                        6026
#define ID_REPORT_BUG                           6027
//#define wxID_ABOUT

// Views
#define ID_LIST_BASE                            7000
#define ID_LIST_PROJECTSVIEW                    7000
#define ID_LIST_WORKVIEW                        7001
#define ID_LIST_TRANSFERSVIEW                   7002
#define ID_LIST_MESSAGESVIEW                    7003
#define ID_PIECTRL_RESOURCEUTILIZATIONVIEW      7004
#define ID_PIECTRL_RESOURCEUTILIZATIONVIEWTOTAL	7005
#define ID_LIST_STATISTICSVIEW                  7006
#define ID_LIST_NOTIFICATIONSVIEW               7007
#define ID_LIST_RELOADNOTICES                   7008
#define ID_TASK_BASE                            8000
#define ID_TASK_NOTIFICATIONSVIEW               8000
#define ID_TASK_PROJECTSVIEW                    8001
#define ID_TASK_WORKVIEW                        8002
#define ID_TASK_TRANSFERSVIEW                   8003
#define ID_TASK_STATISTICSVIEW                  8004
#define ID_TASK_RESOURCEUTILIZATIONVIEW         8005
#define ID_TASK_PROJECT_UPDATE                  9000
#define ID_TASK_PROJECT_SUSPEND                 9002
#define ID_TASK_PROJECT_RESUME                  9003
#define ID_TASK_PROJECT_NONEWWORK               9004
#define ID_TASK_PROJECT_ALLOWNEWWORK            9005
#define ID_TASK_PROJECT_RESET                   9006
#define ID_TASK_PROJECT_DETACH                  9007
#define ID_TASK_PROJECT_SHOW_PROPERTIES         9008
#define ID_TASK_PROJECT_WEB_PROJDEF_MIN         9100
#define ID_TASK_PROJECT_WEB_PROJDEF_MAX         9150
#define ID_TASK_ACTIVE_ONLY                     9200
#define ID_TASK_WORK_SUSPEND                    9201
#define ID_TASK_WORK_SHOWGRAPHICS               9202
#define ID_TASK_WORK_ABORT                      9203
#define ID_TASK_SHOW_PROPERTIES                 9204
#define ID_TASK_WORK_VMCONSOLE                  9205
#define ID_TASK_TRANSFERS_RETRYNOW              9300
#define ID_TASK_TRANSFERS_ABORT                 9301
#define ID_TASK_MESSAGES_COPYALL                9400
#define ID_TASK_MESSAGES_COPYSELECTED           9401
#define ID_TASK_MESSAGES_FILTERBYPROJECT        9402
#define ID_TASK_MESSAGES_FILTERBYERROR          9403
#define ID_TASK_STATISTICS_USERTOTAL            9500
#define ID_TASK_STATISTICS_USERAVERAGE          9501
#define ID_TASK_STATISTICS_HOSTTOTAL            9502
#define ID_TASK_STATISTICS_HOSTAVERAGE          9503
#define ID_TASK_STATISTICS_NEXTPROJECT          9601
#define ID_TASK_STATISTICS_PREVPROJECT          9602
#define ID_TASK_STATISTICS_HIDEPROJLIST         9603
#define ID_TASK_STATISTICS_MODEVIEWALLSEPARATE  9610
#define ID_TASK_STATISTICS_MODEVIEWONEPROJECT   9611
#define ID_TASK_STATISTICS_MODEVIEWALLTOGETHER  9612
#define ID_TASK_STATISTICS_MODEVIEWSUM          9613
#define ID_TASK_NEWS_BOINC                      9700
#define ID_TASK_NEWS_BOINCWEBSITE               9701

// Shortcuts
#define ID_SELECTALL                            9800

//
// Simple GUI
//
#define ID_SIMPLEFRAME                          6400
#define ID_SIMPLEMESSAGECHECKTIMER              6401
#define ID_SIMPLE_HELP                          6402
#define ID_SIMPLE_MESSAGESVIEW                  6403
#define ID_SGTASKSELECTOR                       6404
#define ID_TASKSCOMMANDBUTTON                   6405
#define ID_ADDROJECTBUTTON                      6406
#define ID_SGPROJECTSELECTOR                    6407
#define ID_PROJECTWEBSITESBUTTON                6408
#define ID_PROJECTCOMMANDBUTTON                 6409
#define ID_SGNOTICESBUTTON                      6410
#define ID_SGSUSPENDRESUMEBUTTON                6411
#define ID_SGOPTIONS                            6412
#define ID_SGSKINSELECTOR                       6413
#define ID_SGPROJECTDESCRIPTION                 6414
#define ID_SGDIAGNOSTICLOGFLAGS                 6415
#define ID_SGDEFAULTSKINSELECTOR                6500
#define ID_SGFIRSTSKINSELECTOR                  6501
// 6501-6599 Reserved for Skin Selection
#define ID_LASTSGSKINSELECTOR                   6999

#define ID_CHANGE_SLIDE_TIMER                   6600
#define WEBSITE_URL_MENU_ID_REMOVE_PROJECT      6610
#define WEBSITE_URL_MENU_ID_HOMEPAGE            6620
#define WEBSITE_URL_MENU_ID                     6630





//
// Taskbar/System Tray
//
#define ID_TB_SUSPEND                           6301
#define ID_TB_SUSPEND_GPU                       6302

//
// Dialogs
//
#define ID_ANYDIALOG                            10000

#endif
