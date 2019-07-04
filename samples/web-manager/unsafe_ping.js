function UnsafePing() {

    var auth_id = 0;
    var auth_seqno = 1;
    var auth_salt;
    var passwd = null;
    var http = new XMLHttpRequest();

    var state;	// result of get_state()

    http.open("POST", "http://localhost:31416", true);
    if (auth_id) {
	http.setRequestHeader("Auth-ID", auth_id);
	console.log("request "+request+ " auth ID " + auth_id + " seqno " + auth_seqno);
	http.setRequestHeader("Auth-Seqno", auth_seqno);
	var seqno = String(auth_seqno);
	var salt = String(auth_salt);
	var x = crypto.createHash('md5').update(seqno+passwd+salt+request).digest("hex");
	http.setRequestHeader("Auth-Hash", x);
	auth_seqno++;
    }
    var request = "<boinc_gui_rpc_request>\n" +
                               " <exchange_versions>\n"+
                               "<major>BOINC_MAJOR_VERSION</major>\n"+
                               "<minor>BOINC_MINOR_VERSION</minor>\n"+
                               " <release>BOINC_RELEASE</release>\n"+
        " </exchange_versions>\n\n</boinc_gui_rpc_request>\n\003";
    http.send(request);
}

