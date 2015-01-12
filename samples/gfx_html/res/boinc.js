// 
//

var BOINC = function () {
    var req = null;

    var updatedTime;
    var updatedStateTime;
    var userName;
    var teamName;
    var wuName;
    var elapsedTime;
    var cpuTime;
    var fractionDone;
    var isSuspended;
    var isNetworkSuspended;
    var isAbortRequested;
    var isQuitRequested;
    var rereadInitDataFile;


    var XMLHttpFactories = [
        function () { return new XMLHttpRequest() },
        function () { return new ActiveXObject("Msxml2.XMLHTTP") },
        function () { return new ActiveXObject("Msxml3.XMLHTTP") },
        function () { return new ActiveXObject("Microsoft.XMLHTTP") }
    ];

    for (var i = 0; i < XMLHttpFactories.length; i++) {
        try {
            req = XMLHttpFactories[i]();
        }
        catch (e) {
            continue;
        }
        break;
    }
};

BOINC.prototype.sendRequest = function (url, postData) {
    if (!req) return;
    req.open((postData) ? "POST" : "GET", url, false);
    req.setRequestHeader("User-Agent", "XMLHTTP/1.0");
    if (postData)
        req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    req.send(postData);
    return req.responseXML
}

BOINC.prototype.poll = function () {
    var xmlDoc;

    xmlDoc = sendRequest("/api/getGraphicsStatus", "");

    updatedTime = parseFloat(xmlDoc.getElementsByTagName("updated_time")[0].childNodes[0].nodeValue);
    fractionDone = parseFloat(xmlDoc.getElementsByTagName("fraction_done")[0].childNodes[0].nodeValue);
    elapsedTime = parseFloat(xmlDoc.getElementsByTagName("elapsed_time")[0].childNodes[0].nodeValue);
    cpuTime = parseFloat(xmlDoc.getElementsByTagName("cpu_time")[0].childNodes[0].nodeValue);
    isSuspended = parseInt(xmlDoc.getElementsByTagName("suspended")[0].childNodes[0].nodeValue);
    isNetworkSuspended = parseInt(xmlDoc.getElementsByTagName("network_suspended")[0].childNodes[0].nodeValue);
    isAbortRequested = parseInt(xmlDoc.getElementsByTagName("abort_request")[0].childNodes[0].nodeValue);
    isQuitRequested = parseInt(xmlDoc.getElementsByTagName("quit_request")[0].childNodes[0].nodeValue);
    rereadInitDataFile = parseInt(xmlDoc.getElementsByTagName("reread_init_data_file")[0].childNodes[0].nodeValue);

    xmlDoc = null;

    if ((updatedStateTime != updatedTime) || rereadInitDataFile) {
        updatedStateTime = updatedTime;

        xmlDoc = sendRequest("/api/getInitData", "");

        userName = xmlDoc.getElementsByTagName("user_name")[0].childNodes[0].nodeValue;
        teamName = xmlDoc.getElementsByTagName("team_name")[0].childNodes[0].nodeValue;
        wuName = xmlDoc.getElementsByTagName("wu_name")[0].childNodes[0].nodeValue;

        xmlDoc = null;
    }
}




