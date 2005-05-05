<?

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
