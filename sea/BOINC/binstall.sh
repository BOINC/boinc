cd BOINC && 
echo "cd \"`pwd`\" && ./boinc_client" > run_client &&
chmod +x run_client
echo now run `pwd`/run_client to run the client and `pwd`/boinc_gui to run the GUI
