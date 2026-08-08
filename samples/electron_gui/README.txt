This is the beginning of an electron-based BOINC GUI.

Currently to use this you need to:
1) clone the electron "simple-samples" repo
2) copy these files to the "activity-monitor" directory
	(overwrite the files that are there)
3) copy your BOINC client's gui_rpc_auth.cfg file to that directory
4) from a command prompt in that directory, do
	npm install
	nmp start

	It should open a window with your host name, lists of projects and tasks,
	and a "suspend" button.

Many things to do:

1) eliminate the need for the simple-samples repo
2) factor out the GUI RPC code into a separate file, in lib/
3) finish the GUI
