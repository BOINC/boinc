// This is a proof of concept for an electron-based BOINC GUI,
// showing how to do GUI RPCs to the client.
//
// I developed this by starting with one of electron's "simple samples" (activity-monitor)
// and replacing the files index.html, styles.css, and window.js
//
// Run this on a host with a running BOINC client,
// in a directory with the GUI password file (gui_rpc_auth.cfg).
//
// Some next steps:
// - do periodic RPCs to update state information
// - add menus and dialogs
// - Turn the Suspend button into a never/prefs/always control

const os = require('os')
const fs = require('fs')
const crypto = require('crypto')

var auth_id = 0;
var auth_seqno = 1;
var auth_salt;
var passwd = null;
var http = new XMLHttpRequest();

var state;	// result of get_state()

// read GUI RPC password from file
//
function read_password() {
	try {
		p = fs.readFileSync("gui_rpc_auth.cfg")
		passwd = String(p).trim();
	} catch (err) {
		console.log("can't read RPC password file");
	}
}

// initiate a GUI RPC, passing authentication info if needed
// returns a Promise; passes reply XML to the resolve function
//
function gui_rpc(request) {
	return new Promise(function(resolve, reject) {
		http.onreadystatechange = function() {
			if (http.readyState != 4) return;
			if (http.status == 200) {
				//console.log('got response: ' + http.responseText);
				resolve(http.responseText);
			} else {
				reject(http);
			}
		};
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
		http.send(request);
	});
}

// Do a set_run_mode() RPC
//
function set_run_mode(mode) {
	return gui_rpc("<set_run_mode><"+mode+"/></set_run_mode>").then(function(reply) {
		console.log("set run mode");
	});
}

// get authentication ID
//
function authorize() {
	read_password();
	if (!passwd) {
		return new Promise(function(resolve, reject) {
			resolve();
		});
	}
	return gui_rpc("<get_auth_id/>").then(function(reply) {
		// TODO: error checking
		x = new DOMParser().parseFromString(reply, "text/xml");
		auth_id = x.getElementsByTagName("auth_id")[0].childNodes[0].nodeValue;
		auth_salt = x.getElementsByTagName("auth_salt")[0].childNodes[0].nodeValue;
		//console.log("auth_id: "+auth_id);
		//console.log("auth_salt: "+auth_salt);
	});
}

// do a get_state() RPC
//
function get_state() {
	return gui_rpc("<get_state/>").then(function(reply) {
		parser = new DOMParser();
		state = parser.parseFromString(reply, "text/xml");
	});
}

function suspend() {
	set_run_mode("never").then(function() {
	});
}

// display the result of get_state()
//
function show_state() {
	// show host name
	//
	d = state.querySelector("domain_name").textContent;
	console.log('domain name', d);
	document.querySelector('#domain_name').innerHTML = d;

	// show projects
	//
	x = '';
	state.querySelectorAll("project").forEach((p)=>{
		x += '<br>'+ p.querySelector('project_name').textContent;
	});
	document.querySelector('#projects').innerHTML = x;

	// show tasks
	x = '';
	state.querySelectorAll("result").forEach((r)=>{
		x += '<br>'+ r.querySelector('name').textContent;
	});
	document.querySelector('#tasks').innerHTML = x;
}


$(() => {		// shorthand for document ready
	// get authorization ID
	//
	authorize().then(function() {
		// then get state and show it
		//
		get_state().then(function() {
			show_state();
		});
	});
})
