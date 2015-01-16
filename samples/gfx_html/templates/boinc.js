// This file is part of BOINC.
// http://boinc.berkeley.edu
//

var BOINC = function () {
    var updatedTime;
    var updatedStateTime;
    var userName;
    var teamName;
    var wuName;
    var resultName;
    var authenticator;
    var userId;
    var teamId;
    var hostId;
    var userCreditTotal;
    var userCreditAverage;
    var hostCreditTotal;
    var hostCreditAverage;
    var exit;
    var exit_timeout;
    var vboxJob;
    var webAPIPort;
    var remoteDesktopPort;
    var elapsedTime;
    var cpuTime;
    var fractionDone;
    var suspended;
    var networkSuspended;
    var abortRequested;
    var quitRequested;
    var stateFileUpdated;

    this.poll();
};

BOINC.prototype.createRequest = function () {
    var xmlHttp = null;
    var XMLHttpFactories = [
        function () { return new XMLHttpRequest() },
        function () { return new ActiveXObject('Msxml2.XMLHTTP') },
        function () { return new ActiveXObject('Msxml3.XMLHTTP') },
        function () { return new ActiveXObject('Microsoft.XMLHTTP') }
    ];

    for (var i = 0; i < XMLHttpFactories.length; i++) {
        try {
            xmlHttp = XMLHttpFactories[i]();
        } catch (e) {
            continue;
        }
        break;
    }
    return xmlHttp;
}

BOINC.prototype.sendRequest = function (url) {
    var req = this.createRequest();
    var response = null;
    req.open('GET', url, false);
    req.send();
    response = req.responseXML;
    req = null;
    return response;
}

BOINC.prototype.poll = function () {
    var xmlGraphicsStatusDoc;
    var xmlInitDataDoc;

    xmlGraphicsStatusDoc = this.sendRequest('/api/getGraphicsStatus');
    this.updatedTime = parseFloat(xmlGraphicsStatusDoc.getElementsByTagName('updated_time')[0].childNodes[0].nodeValue);
    this.fractionDone = parseFloat(xmlGraphicsStatusDoc.getElementsByTagName('fraction_done')[0].childNodes[0].nodeValue);
    this.elapsedTime = parseFloat(xmlGraphicsStatusDoc.getElementsByTagName('elapsed_time')[0].childNodes[0].nodeValue);
    this.cpuTime = parseFloat(xmlGraphicsStatusDoc.getElementsByTagName('cpu_time')[0].childNodes[0].nodeValue);
    this.suspended = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('suspended')[0].childNodes[0].nodeValue);
    this.networkSuspended = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('network_suspended')[0].childNodes[0].nodeValue);
    this.abortRequested = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('abort_request')[0].childNodes[0].nodeValue);
    this.quitRequested = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('quit_request')[0].childNodes[0].nodeValue);
    this.stateFileUpdated = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('reread_init_data_file')[0].childNodes[0].nodeValue);
    this.exit = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('suspended')[0].childNodes[0].nodeValue);
    this.exit_timeout = parseFloat(xmlGraphicsStatusDoc.getElementsByTagName('cpu_time')[0].childNodes[0].nodeValue);

    if (this.stateFileUpdated || (this.updatedStateTime == undefined)) {
        this.updatedStateTime = this.updatedTime;

        this.vboxJob = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('vbox_job')[0].childNodes[0].nodeValue);
        this.webAPIPort = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('webapi_port')[0].childNodes[0].nodeValue);
        this.remoteDesktopPort = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('remote_desktop_port')[0].childNodes[0].nodeValue);

        xmlInitDataDoc = this.sendRequest('/api/getInitData');
        this.userName = xmlInitDataDoc.getElementsByTagName('user_name')[0].childNodes[0].nodeValue;
        this.wuName = xmlInitDataDoc.getElementsByTagName('wu_name')[0].childNodes[0].nodeValue;
        this.resultName = xmlInitDataDoc.getElementsByTagName('result_name')[0].childNodes[0].nodeValue;
        this.authenticator = xmlInitDataDoc.getElementsByTagName('authenticator')[0].childNodes[0].nodeValue;
        this.userCreditTotal = parseFloat(xmlInitDataDoc.getElementsByTagName('user_total_credit')[0].childNodes[0].nodeValue);
        this.userCreditAverage = parseFloat(xmlInitDataDoc.getElementsByTagName('user_expavg_credit')[0].childNodes[0].nodeValue);
        this.hostCreditTotal = parseFloat(xmlInitDataDoc.getElementsByTagName('host_total_credit')[0].childNodes[0].nodeValue);
        this.hostCreditAverage = parseFloat(xmlInitDataDoc.getElementsByTagName('host_expavg_credit')[0].childNodes[0].nodeValue);

        // Optional elements
        //
        try {
            this.teamName = xmlInitDataDoc.getElementsByTagName('team_name')[0].childNodes[0].nodeValue;
        } catch (e) {
            this.teamName = '';
        }
        try {
            this.userId = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('userid')[0].childNodes[0].nodeValue);
        } catch (e) {
            this.userId = 0;
        }
        try {
            this.teamId = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('teamid')[0].childNodes[0].nodeValue);
        } catch (e) {
            this.teamId = 0;
        }
        try {
            this.hostId = parseInt(xmlGraphicsStatusDoc.getElementsByTagName('hostid')[0].childNodes[0].nodeValue);
        } catch (e) {
            this.hostId = 0;
        }

        this.sendRequest('/api/resetReadFlag');
    }
}

BOINC.prototype.getUpdatedTime = function () {
    return this.updatedTime;
}

BOINC.prototype.getUpdatedStateTime = function () {
    return this.updatedStateTime;
}

BOINC.prototype.getUserName = function () {
    return this.userName;
}

BOINC.prototype.getTeamName = function () {
    return this.teamName;
}

BOINC.prototype.getWorkunitName = function () {
    return this.wuName;
}

BOINC.prototype.getResultName = function () {
    return this.resultName;
}

BOINC.prototype.getAuthenticator = function () {
    return this.authenticator;
}

BOINC.prototype.getUserId = function () {
    return this.userId;
}

BOINC.prototype.getTeamId = function () {
    return this.teamId;
}

BOINC.prototype.getHostId = function () {
    return this.hostId;
}

BOINC.prototype.getUserCreditTotal = function () {
    return this.userCreditTotal;
}

BOINC.prototype.getUserCreditAverage = function () {
    return this.userCreditAverage;
}

BOINC.prototype.getHostCreditTotal = function () {
    return this.hostCreditTotal;
}

BOINC.prototype.getHostCreditAverage = function () {
    return this.hostCreditAverage;
}

BOINC.prototype.isExiting = function () {
    return this.exit;
}

BOINC.prototype.getExitTimeout = function () {
    return this.exit_timeout;
}

BOINC.prototype.isVrtualBoxJob = function () {
    return this.vboxJob;
}

BOINC.prototype.getWebAPIPort = function () {
    return this.webAPIPort;
}

BOINC.prototype.getRemoteDesktopPort = function () {
    return this.remoteDesktopPort;
}

BOINC.prototype.getFractionDone = function () {
    return this.fractionDone;
}

BOINC.prototype.getElapsedTime = function () {
    return this.elapsedTime;
}

BOINC.prototype.getCPUTime = function () {
    return this.cpuTime;
}

BOINC.prototype.isSuspended = function () {
    return this.suspended;
}

BOINC.prototype.isNetworkSuspended = function () {
    return this.networkSuspended;
}

BOINC.prototype.isAbortRequested = function () {
    return this.abortRequested;
}

BOINC.prototype.isQuitRequested = function () {
    return this.quitRequested;
}

BOINC.prototype.isStateFileUpdated = function () {
    return this.stateFileUpdated;
}
