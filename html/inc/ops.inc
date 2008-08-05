<?php
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

// Only allow this script to run from the command line
//
function cli_only(){
    if (array_key_exists("SERVER_PORT", $_SERVER)) {
        die("<html><h1>
            This script is intended to be run from the command line,
            not from the web server!
            </h1>"
        );
    }
}

// initialize database connection with username & password from
// command line instead of config.xml
//
function db_init_cli() {
    $config = get_config();
    $db_name = parse_config($config, "<db_name>");
    $host = parse_config($config, "<db_host>");
    if ($host == null) {
        $host = "localhost";
    }
    $in = fopen("php://stdin","r");
    print "Database username for $db_name@$host: ";
    $user = rtrim(fgets($in, 80));
    print "Database password for $db_name@host: ";
    $pass = rtrim(fgets($in, 80));

    $retval = mysql_pconnect($host, $user, $pass);
    if (!$retval) {
        return 1;
    }
    if (!mysql_select_db($db_name)) {
        return 2;
    }
    return 0;
}

?>
