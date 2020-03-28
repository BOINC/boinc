/*
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
 */

package edu.berkeley.boinc.rpc;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import android.util.Log;
import android.util.Xml;

import java.util.List;

import edu.berkeley.boinc.utils.Logging;

public class CcStateParser extends BaseParser {
    static final String CLIENT_STATE_TAG = "client_state";
    static final String CORE_CLIENT_MAJOR_VERSION_TAG = "core_client_major_version";
    static final String CORE_CLIENT_MINOR_VERSION_TAG = "core_client_minor_version";
    static final String CORE_CLIENT_RELEASE_TAG = "core_client_release";

    private CcState mCcState = new CcState();
    private Project myProject = new Project();
    private VersionInfo mVersionInfo = new VersionInfo();

    private AppsParser mAppsParser = new AppsParser();
    private AppVersionsParser mAppVersionsParser = new AppVersionsParser();
    private HostInfoParser mHostInfoParser = new HostInfoParser();
    private ProjectsParser mProjectsParser = new ProjectsParser();
    private ResultsParser mResultsParser = new ResultsParser();
    private WorkunitsParser mWorkunitsParser = new WorkunitsParser();

    private boolean mInApp = false;
    private boolean mInAppVersion = false;
    private boolean mInHostInfo = false;
    private boolean mInProject = false;
    private boolean mInResult = false;
    private boolean mInWorkunit = false;

    final CcState getCcState() {
        return mCcState;
    }

    /**
     * Parse the RPC result (state) and generate vector of projects info
     *
     * @param rpcResult String returned by RPC call of core client
     * @return connected client state
     */
    public static CcState parse(String rpcResult) {
        try {
            CcStateParser parser = new CcStateParser();
            Xml.parse(rpcResult, parser);
            return parser.getCcState();
        }
        catch(SAXException e) {
            return null;
        }
    }

    @Override
    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        super.startElement(uri, localName, qName, attributes);
        if(localName.equalsIgnoreCase(CLIENT_STATE_TAG)) {
            //Starting the query, clear mCcState
            mCcState.clearArrays();
        }
        if(localName.equalsIgnoreCase(HostInfoParser.HOST_INFO_TAG)) {
            // Just stepped inside <host_info>
            mInHostInfo = true;
        }
        if(mInHostInfo) {
            mHostInfoParser.startElement(uri, localName, qName, attributes);
        }
        if(localName.equalsIgnoreCase(ProjectsParser.PROJECT_TAG)) {
            // Just stepped inside <project>
            mInProject = true;
        }
        if(mInProject) {
            mProjectsParser.startElement(uri, localName, qName, attributes);
        }
        if(localName.equalsIgnoreCase(AppsParser.APP_TAG)) {
            // Just stepped inside <app>
            mInApp = true;
        }
        if(mInApp) {
            mAppsParser.startElement(uri, localName, qName, attributes);
        }
        if(localName.equalsIgnoreCase(AppVersionsParser.APP_VERSION_TAG)) {
            // Just stepped inside <app_version>
            mInAppVersion = true;
        }
        if(mInAppVersion) {
            mAppVersionsParser.startElement(uri, localName, qName, attributes);
        }
        if(localName.equalsIgnoreCase(WorkunitsParser.WORKUNIT_TAG)) {
            // Just stepped inside <workunit>
            mInWorkunit = true;
        }
        if(mInWorkunit) {
            mWorkunitsParser.startElement(uri, localName, qName, attributes);
        }
        if(localName.equalsIgnoreCase(ResultsParser.RESULT_TAG)) {
            // Just stepped inside <result>
            mInResult = true;
        }
        if(mInResult) {
            mResultsParser.startElement(uri, localName, qName, attributes);
        }
        if(localName.equalsIgnoreCase(CORE_CLIENT_MAJOR_VERSION_TAG) ||
           localName.equalsIgnoreCase(CORE_CLIENT_MINOR_VERSION_TAG) ||
           localName.equalsIgnoreCase(CORE_CLIENT_RELEASE_TAG) ||
           localName.equalsIgnoreCase(CcState.Fields.HAVE_ATI) ||
           localName.equalsIgnoreCase(CcState.Fields.HAVE_CUDA)) {
            // VersionInfo elements
            mElementStarted = true;
            mCurrentElement.setLength(0);
        }
    }

    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        super.characters(ch, start, length);
        if(mInHostInfo) {
            // We are inside <host_info>
            mHostInfoParser.characters(ch, start, length);
        }
        if(mInProject) {
            // We are inside <project>
            mProjectsParser.characters(ch, start, length);
        }
        if(mInApp) {
            // We are inside <project>
            mAppsParser.characters(ch, start, length);
        }
        if(mInAppVersion) {
            // We are inside <project>
            mAppVersionsParser.characters(ch, start, length);
        }
        if(mInWorkunit) {
            // We are inside <workunit>
            mWorkunitsParser.characters(ch, start, length);
        }
        if(mInResult) {
            // We are inside <result>
            mResultsParser.characters(ch, start, length);
        }
        // VersionInfo elements are handled in super.characters()
    }

    @Override
    public void endElement(String uri, String localName, String qName) throws SAXException {
        super.endElement(uri, localName, qName);
        try {
            if(mInHostInfo) {
                // We are inside <host_info>
                // parse it by sub-parser in any case (to parse also closing element)
                mHostInfoParser.endElement(uri, localName, qName);
                mCcState.setHostInfo(mHostInfoParser.getHostInfo());
                if(localName.equalsIgnoreCase(HostInfoParser.HOST_INFO_TAG)) {
                    mInHostInfo = false;
                }
            }
            if(mInProject) {
                // We are inside <project>
                // parse it by sub-parser in any case (must parse also closing element!)
                mProjectsParser.endElement(uri, localName, qName);
                if(localName.equalsIgnoreCase(ProjectsParser.PROJECT_TAG)) {
                    // Closing tag of <project>
                    mInProject = false;
                    final List<Project> projects = mProjectsParser.getProjects();

                    if(!projects.isEmpty()) {
                        myProject = projects.get(projects.size() - 1);
                        mCcState.getProjects().add(myProject);
                    }
                }
            }
            if(mInApp) {
                // We are inside <app>
                // parse it by sub-parser in any case (must parse also closing element!)
                mAppsParser.endElement(uri, localName, qName);
                if(localName.equalsIgnoreCase(AppsParser.APP_TAG)) {
                    // Closing tag of <app>
                    mInApp = false;
                    final List<App> apps = mAppsParser.getApps();

                    if(!apps.isEmpty()) {
                        final App myApp = apps.get(apps.size() - 1);
                        myApp.setProject(myProject);
                        mCcState.getApps().add(myApp);
                    }
                }
            }
            if(mInAppVersion) {
                // We are inside <app_version>
                // parse it by sub-parser in any case (must also parse closing element!)
                mAppVersionsParser.endElement(uri, localName, qName);
                if(localName.equalsIgnoreCase(AppVersionsParser.APP_VERSION_TAG)) {
                    // Closing tag of <app_version>
                    mInAppVersion = false;
                    final List<AppVersion> appVersions = mAppVersionsParser.getAppVersions();

                    if(!appVersions.isEmpty()) {
                        final AppVersion myAppVersion = appVersions.get(appVersions.size() - 1);
                        myAppVersion.project = myProject;
                        myAppVersion.app = mCcState.lookupApp(myProject, myAppVersion.app_name);
                        mCcState.getAppVersions().add(myAppVersion);
                    }
                }
            }
            if(mInWorkunit) {
                // We are inside <workunit>
                // parse it by sub-parser in any case (must parse also closing element!)
                mWorkunitsParser.endElement(uri, localName, qName);
                if(localName.equalsIgnoreCase(WorkunitsParser.WORKUNIT_TAG)) {
                    // Closing tag of <workunit>
                    mInWorkunit = false;
                    final List<Workunit> workunits = mWorkunitsParser.getWorkunits();

                    if(!workunits.isEmpty()) {
                        final Workunit myWorkunit = workunits.get(workunits.size() - 1);
                        myWorkunit.project = myProject;
                        myWorkunit.app = mCcState.lookupApp(myProject, myWorkunit.app_name);
                        mCcState.getWorkunits().add(myWorkunit);
                    }
                }
            }
            if(mInResult) {
                // We are inside <result>
                // parse it by sub-parser in any case (must parse also closing element!)
                mResultsParser.endElement(uri, localName, qName);
                if(localName.equalsIgnoreCase(ResultsParser.RESULT_TAG)) {
                    // Closing tag of <result>
                    mInResult = false;
                    final List<Result> results = mResultsParser.getResults();

                    if(!results.isEmpty()) {
                        final Result myResult = results.get(results.size() - 1);
                        myResult.project = myProject;
                        myResult.wup = mCcState.lookupWorkUnit(myProject, myResult.wu_name);
                        if(myResult.wup != null) {
                            myResult.app = myResult.wup.app;
                            myResult.avp =
                                    mCcState.lookupAppVersion(myProject, myResult.app,
                                                              myResult.version_num,
                                                              myResult.plan_class);

                        }
                        mCcState.getResults().add(myResult);
                    }
                }
            }
            if(mElementStarted) {
                trimEnd();
                // VersionInfo?
                if(localName.equalsIgnoreCase(CORE_CLIENT_MAJOR_VERSION_TAG)) {
                    mVersionInfo.major = Integer.parseInt(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(CORE_CLIENT_MINOR_VERSION_TAG)) {
                    mVersionInfo.minor = Integer.parseInt(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(CORE_CLIENT_RELEASE_TAG)) {
                    mVersionInfo.release = Integer.parseInt(mCurrentElement.toString());
                }
                else if(localName.equalsIgnoreCase(CcState.Fields.HAVE_ATI)) {
                    mCcState.setHaveAti(!mCurrentElement.toString().equals("0"));
                }
                else if(localName.equalsIgnoreCase(CcState.Fields.HAVE_CUDA)) {
                    mCcState.setHaveCuda(!mCurrentElement.toString().equals("0"));
                }
                mElementStarted = false;
                mCcState.setVersionInfo(mVersionInfo);
            }
        }
        catch(NumberFormatException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "CcStateParser.endElement error: ", e);
            }
        }
    }
}
