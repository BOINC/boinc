// Remap console functions if the console object doesn't exist in the correct
// location
//

if (!(window.console)) {
    var req = null;
    var XMLHttpFactories = [
    function () { return new XMLHttpRequest() },
    function () { return new ActiveXObject("Msxml2.XMLHTTP") },
    function () { return new ActiveXObject("Msxml3.XMLHTTP") },
    function () { return new ActiveXObject("Microsoft.XMLHTTP") }
    ];

    console = {};

    console.createXMLHTTPObject = function () {
        var xmlhttp = false;
        for (var i = 0; i < XMLHttpFactories.length; i++) {
            try {
                xmlhttp = XMLHttpFactories[i]();
            }
            catch (e) {
                continue;
            }
            break;
        }
        return xmlhttp;
    }

    console.sendRequest = function (url, postData) {
        if (req == null) {
            req = createXMLHTTPObject();
        }
        if (!req) return;
        var method = (postData) ? "POST" : "GET";
        req.open(method, url, false);
        req.setRequestHeader("User-Agent", "XMLHTTP/1.0");
        if (postData)
            req.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        req.send(postData);
    }

    console.logMessage = function (level, message) {
        console.sendRequest("/api/logMessage", "level=" + level + "&message=" + message);
    };

    console.log = function (message) {
        console.logMessage("INFO", message);
    };

    console.debug = function (message) {
        console.logMessage("DEBUG", message);
    };

    console.info = function (message) {
        console.logMessage("INFO", message);
    };

    console.warn = function (message) {
        console.logMessage("WARNING", message);
    };

    console.error = function (message) {
        console.logMessage("ERROR", message);
    };
}
