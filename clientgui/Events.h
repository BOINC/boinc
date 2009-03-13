// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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


#ifndef _EVENTS_H_
#define _EVENTS_H_

#define ID_ADVANCEDFRAME                        6000
#define ID_STATUSBAR                            6001
#define ID_FRAMENOTEBOOK                        6002
#define ID_REFRESHSTATETIMER                    6003
#define ID_FRAMERENDERTIMER                     6004
#define ID_FRAMETASKRENDERTIMER                 6005
#define ID_DOCUMENTPOLLTIMER                    6007
#define ID_ALERTPOLLTIMER                       6009
#define ID_FILEACTIVITYRUNALWAYS                6010
#define ID_FILEACTIVITYRUNBASEDONPREPERENCES    6011
#define ID_FILEACTIVITYSUSPEND                  6012
#define ID_FILENETWORKRUNALWAYS                 6013
#define ID_FILENETWORKRUNBASEDONPREPERENCES     6014
#define ID_FILENETWORKSUSPEND                   6015
#define ID_COMMANDSRETRYCOMMUNICATIONS          6016
#define ID_FILERUNBENCHMARKS                    6017
#define ID_FILESELECTCOMPUTER                   6018
#define ID_PROJECTSATTACHACCOUNTMANAGER         6019
#define ID_PROJECTSATTACHPROJECT                6020
#define ID_READ_PREFS                           6021
#define ID_READ_CONFIG                          6022
#define ID_OPTIONSOPTIONS                       6023
#define ID_HELPBOINCWEBSITE                     6024
#define ID_HELPBOINCMANAGER                     6025
#define ID_TOOLSAMUPDATENOW                     6026
#define ID_ADVANCEDAMDEFECT                     6027
#define ID_OPENWEBSITE                          6028
#define ID_FILESWITCHGUI                        6029
#define ID_ACTIVITYMENUSEPARATOR                6031
#define ID_FILECLOSEWINDOW                      6032
#define ID_ADVPREFSDLG                          6033
#define ID_SHUTDOWNCORECLIENT                   6034
#define ID_HELPBOINC                            6035
#define ID_VIEWLIST                             6036
#define ID_PERIODICRPCTIMER                     6050
#define ID_SIMPLEFRAME                          6100
#define ID_SIMPLEMESSAGECHECKTIMER              6101
#define ID_SIMPLE_ATTACHTOPROJECT               6600
#define ID_SIMPLE_HELP                          6601
#define ID_SIMPLE_MESSAGES                      6602
#define ID_SIMPLE_MESSAGES_ALERT                6603
#define ID_SIMPLE_SUSPEND                       6604
#define ID_SIMPLE_RESUME                        6605
#define ID_SIMPLE_PREFERENCES                   6606
#define ID_SIMPLE_MESSAGESVIEW                  6607
#define ID_SIMPLE_SYNCHRONIZE                   6608
#define ID_TB_SUSPEND                           6801
#define ID_LIST_BASE                            7000
#define ID_LIST_PROJECTSVIEW                    7000
#define ID_LIST_WORKVIEW                        7001
#define ID_LIST_TRANSFERSVIEW                   7002
#define ID_LIST_MESSAGESVIEW                    7003
#define ID_PIECTRL_RESOURCEUTILIZATIONVIEW      7004
#define ID_PIECTRL_RESOURCEUTILIZATIONVIEWTOTAL	7005
#define ID_LIST_STATISTICSVIEW                  7006
#define ID_HTML_NEWSVIEW                        7007
#define ID_TASK_BASE                            8000
#define ID_TASK_PROJECTSVIEW                    8000
#define ID_TASK_WORKVIEW                        8001
#define ID_TASK_TRANSFERSVIEW                   8002
#define ID_TASK_MESSAGESVIEW                    8003
#define ID_TASK_STATISTICSVIEW                  8004
#define ID_TASK_RESOURCEUTILIZATIONVIEW         8005
#define ID_TASK_NEWSVIEW                        8006
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
#define ID_TASK_WORK_SUSPEND                    9200
#define ID_TASK_WORK_RESUME                     9201
#define ID_TASK_WORK_SHOWGRAPHICS               9202
#define ID_TASK_WORK_ABORT                      9203
#define ID_TASK_SHOW_PROPERTIES                 9204
#define ID_TASK_TRANSFERS_RETRYNOW              9300
#define ID_TASK_TRANSFERS_ABORT                 9301
#define ID_TASK_MESSAGES_COPYALL                9400
#define ID_TASK_MESSAGES_COPYSELECTED           9401
#define ID_TASK_MESSAGES_FILTERBYPROJECT        9402
#define ID_TASK_STATISTICS_USERTOTAL            9500
#define ID_TASK_STATISTICS_USERAVERAGE          9501
#define ID_TASK_STATISTICS_HOSTTOTAL            9502
#define ID_TASK_STATISTICS_HOSTAVERAGE          9503
#define ID_TASK_STATISTICS_NEXTPROJECT          9601
#define ID_TASK_STATISTICS_PREVPROJECT          9602
#define ID_TASK_STATISTICS_MODEVIEW0            9610
#define ID_TASK_STATISTICS_MODEVIEW1            9611
#define ID_TASK_STATISTICS_MODEVIEW2            9612
#define ID_TASK_NEWS_BOINC                      9700
#define ID_TASK_NEWS_BOINCWEBSITE               9701
#define ID_ANYDIALOG                            10000
#endif

