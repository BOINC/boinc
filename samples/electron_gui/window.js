// This is a proof of concept for an electron-based BOINC GUI,
// showing how to start the client and do GUI RPCs to it.
//
// I developed this by starting with one of electron's "simple samples" (activity-monitor)
// and replacing the files index.html, styles.css, and window.js
//
// Run this in a directory with the GUI password file (gui_rpc_auth.cfg).
//
// Logic for connecting to and starting the client:
// try an RPC.  If it fails, start a client.
// If the client exits, wait N seconds before restarting.
//
// Some next steps:
// - add menus and dialogs
// - Turn the Suspend button into a never/prefs/always control

const os = require('os')
const fs = require('fs')
const crypto = require('crypto')
const { execFile } = require('child_process');

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
		return true
	} catch (err) {
		console.log("can't read RPC password file");
		return false
	}
}

// initiate a GUI RPC, passing authentication info if needed
// returns a Promise; passes reply XML to the resolve function
//
function gui_rpc(request) {
	return new Promise(function(resolve, reject) {
		request = "<boinc_gui_rpc_request>"+request+"</boinc_gui_rpc_request>"
		http.onreadystatechange = function() {
			if (http.readyState != 4) return;
			if (http.status == 200) {
				//console.log('got response: ' + http.responseText);
				resolve(http.responseText)
			} else {
				reject(http)
			}
		};
		http.open("POST", "http://localhost:31416", true)
		if (auth_id) {
			http.setRequestHeader("Auth-ID", auth_id)
			console.log("request "+request+ " auth ID " + auth_id + " seqno " + auth_seqno)
			http.setRequestHeader("Auth-Seqno", auth_seqno)
			var seqno = String(auth_seqno)
			var salt = String(auth_salt)
			var x = crypto.createHash('md5').update(seqno+passwd+salt+request).digest("hex")
			http.setRequestHeader("Auth-Hash", x)
			auth_seqno++
		}
		http.send(request)
	});
}

// Do a set_run_mode() RPC
//
function set_run_mode(mode) {
	return gui_rpc("<set_run_mode><"+mode+"/></set_run_mode>").then((reply)=>{
		console.log("set run mode: "+mode)
	});
}

// get authentication ID
//
function authorize() {
	if (!read_password()) {
		return new Promise((resolve, reject)=>{
			reject();
		});
	}
	return gui_rpc("<get_auth_id/>").then((reply)=>{
		// TODO: error checking
		x = new DOMParser().parseFromString(reply, "text/xml")
		auth_id = x.getElementsByTagName("auth_id")[0].childNodes[0].nodeValue
		auth_salt = x.getElementsByTagName("auth_salt")[0].childNodes[0].nodeValue
		//console.log("auth_id: "+auth_id)
		//console.log("auth_salt: "+auth_salt)
	});
}

// do a get_state() RPC; return promise
//
function get_state() {
	return gui_rpc("<get_state/>").then((reply)=>{
		parser = new DOMParser()
		state = parser.parseFromString(reply, "text/xml")
	})
	.catch((err)=>{
		show_status('disconnected')
		console.log(err)
	});
}

function suspend() {
	set_run_mode("never").then(()=>{
	});
}

function parse_reply(reply) {
	x = new DOMParser().parseFromString(reply, "text/xml")
	if (x.getElementsByTagName('success')) {
		return 0
	}
	return x.getElementsByTagName("status")[0].childNodes[0].nodeValue
}

function attach_poll() {
	console.log('doing poll')
	req = '<project_attach_poll/>'
	gui_rpc(req).then((reply)=>{
		console.log(reply)
		r = parse_reply(reply)
		switch(r) {
		case -199:
			setTimeout(attach_poll, 1000)
			break
		case 0:
			console.log('attached')
			break
		default:
			console.log('attach failed: ', reply)
			break
		}
	})
}

function attach() {
	console.log('attach')
	url = 'https://boinc.berkeley.edu/test/'
	auth = 'xxx'
	req = '<project_attach>'
	req += '<project_url>'+url+'</project_url>'
	req += '<authenticator>'+auth+'</authenticator>'
	req += '<project_name>BOINC test project</project_name>'
	req += '</project_attach>'
	gui_rpc(req).then((reply)=>{
		console.log('did attach RPC')
		attach_poll()
	})
}

function client_exited(error, stdout, stderr) {
	console.log('client exited');
	if (error) {
		throw error
	}
	console.log(stdout)
	console.log(stderr)
}

function show_status(s) {
	document.querySelector('#status').innerHTML = s
}

// Start the BOINC client.
// On Win, the client will find its data directory in the registry.
// May need to set current dir on other platforms.
// If there's already a client running,
// the new one will detect this and exit.
//
function start_client() {
	const child = execFile(
		'c:/Users/David/Documents/BOINC_git/boinc/win_build/Build/x64/Debug/boinc.exe',
		['--redirectio', '--launched_by_manager'],
		{},
		client_exited
	);
}

// display the result of get_state()
//
function show_state() {
	// show host name
	//
	show_status('Connected to ' + state.querySelector("domain_name").textContent)

	// show projects
	//
	x = ''
	state.querySelectorAll("project").forEach((p)=>{
		x += '<br>'+ p.querySelector('project_name').textContent
	});
	document.querySelector('#projects').innerHTML = x

	// show tasks
	x = ''
	state.querySelectorAll("result").forEach((r)=>{
		//console.log(r)
		name = r.querySelector('name').textContent
		x += '<br>'+name
		active_task = r.querySelector('active_task')
		if (active_task) {
			cpu_time = active_task.querySelector('current_cpu_time')
			x += ' CPU time: '+cpu_time.textContent
		}
	});
	document.querySelector('#tasks').innerHTML = x
}

function main_loop() {
	get_state().then(()=>{
		show_state()
		setTimeout(main_loop, 1000)
	})
	.catch(()=>{
		show_status('disconnected')
	});
}

var started_client = false

function startup() {
	authorize().then(function() {
		console.log('authorized')
		main_loop()
	})
	.catch(()=>{
		if (passwd == null) {
			show_status('no password file')
			return
		}
		if (started_client) {
			console.log('waiting for client')
		} else {
			console.log('auth failed - starting client')
			show_status('starting client')
			start_client()
			started_client = true
		}
		setTimeout(startup, 1000)
	});
}

$(() => {		// shorthand for document ready
	console.log('starting');
	startup();
})
