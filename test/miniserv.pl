#!/usr/bin/env perl
# $Id$
# from http://www.webmin.com/updates/miniserv.pl
#   slight kludge modifications:
#      any executable file in /cgi-bin/ is considered a cgi program.
#      .php files are run using php cgi.
#   No considerations for security because this is ONLY meant for single-run TESTING

# A very simple perl web server used by Webmin

# Require basic libraries
package miniserv;
use Socket;
use POSIX;
use Sys::Hostname;

### see 'KLUDGE' below ###  --kc
my $PHP_CGI_PATH = `which php`; chomp $PHP_CGI_PATH;
die "couldn't find PHP" unless -x $PHP_CGI_PATH;

# Find and read config file
if (@ARGV != 1) {
	die "Usage: miniserv.pl <config file>";
	}
if ($ARGV[0] =~ /^\//) {
	$conf = $ARGV[0];
	}
else {
	chop($pwd = `pwd`);
	$conf = "$pwd/$ARGV[0]";
	}
open(CONF, $conf) || die "Failed to open config file $conf : $!";
while(<CONF>) {
	s/\r|\n//g;
	if (/^#/ || !/\S/) { next; }
	/^([^=]+)=(.*)$/;
	$name = $1; $val = $2;
	$name =~ s/^\s+//g; $name =~ s/\s+$//g;
	$val =~ s/^\s+//g; $val =~ s/\s+$//g;
	$config{$name} = $val;
	}
close(CONF);

# Check is SSL is enabled and available
if ($config{'ssl'}) {
	eval "use Net::SSLeay";
	if (!$@) {
		$use_ssl = 1;
		# These functions only exist for SSLeay 1.0
		eval "Net::SSLeay::SSLeay_add_ssl_algorithms()";
		eval "Net::SSLeay::load_error_strings()";
		if (defined(&Net::SSLeay::X509_STORE_CTX_get_current_cert) &&
		    defined(&Net::SSLeay::CTX_load_verify_locations) &&
		    defined(&Net::SSLeay::CTX_set_verify)) {
			$client_certs = 1;
			}
		}
	}

# Check if the syslog module is available to log hacking attempts
if ($config{'syslog'}) {
	eval "use Sys::Syslog qw(:DEFAULT setlogsock)";
	if (!$@) {
		$use_syslog = 1;
		}
	}

# check if the PAM module is available to authenticate
eval "use Authen::PAM";
if (!$@) {
	# check if the PAM authentication can be used by opening a handle
	if (! ref($pamh = new Authen::PAM("webmin", "root", \&pam_conv_func))) {
		print STDERR "PAM module available, but error during init !\n";
		print STDERR "Disabling PAM functions.\n";
		}
	else {
		$use_pam = 1;
		}
	}

# Get miniserv's perl path and location
$miniserv_path = $0;
open(SOURCE, $miniserv_path);
<SOURCE> =~ /^#!(\S+)/; $perl_path = $1;
close(SOURCE);
@miniserv_argv = @ARGV;

# Check vital config options
%vital = ("port", 80,
	  "root", "./",
	  "server", "MiniServ/0.01",
	  "index_docs", "index.html index.htm index.cgi",
	  "addtype_html", "text/html",
	  "addtype_txt", "text/plain",
	  "addtype_gif", "image/gif",
	  "addtype_jpg", "image/jpeg",
	  "addtype_jpeg", "image/jpeg",
	  "realm", "MiniServ",
	  "session_login", "/session_login.cgi"
	 );
foreach $v (keys %vital) {
	if (!$config{$v}) {
		if ($vital{$v} eq "") {
			die "Missing config option $v";
			}
		$config{$v} = $vital{$v};
		}
	}
if (!$config{'sessiondb'}) {
	$config{'pidfile'} =~ /^(.*)\/[^\/]+$/;
	$config{'sessiondb'} = "$1/sessiondb";
	}
die "Session authentication cannot be used in inetd mode"
	if ($config{'inetd'} && $config{'session'});

# init days and months for http_date
@weekday = ( "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" );
@month = ( "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" );

# Change dir to the server root
chdir($config{'root'});
$user_homedir = (getpwuid($<))[7];

# Read users file
if ($config{'userfile'}) {
	open(USERS, $config{'userfile'});
	while(<USERS>) {
		s/\r|\n//g;
		local @user = split(/:/, $_);
		$users{$user[0]} = $user[1];
		$certs{$user[0]} = $user[3] if ($user[3]);
		if ($user[4] =~ /^allow\s+(.*)/) {
			$allow{$user[0]} = [ &to_ipaddress(split(/\s+/, $1)) ];
			}
		elsif ($user[4] =~ /^deny\s+(.*)/) {
			$deny{$user[0]} = [ &to_ipaddress(split(/\s+/, $1)) ];
			}
		}
	close(USERS);
	}

# Setup SSL if possible and if requested
if ($use_ssl) {
	$ssl_ctx = Net::SSLeay::CTX_new() ||
		die "Failed to create SSL context : $!";
	$client_certs = 0 if (!$config{'ca'} || !%certs);
	if ($client_certs) {
		Net::SSLeay::CTX_load_verify_locations(
			$ssl_ctx, $config{'ca'}, "");
		Net::SSLeay::CTX_set_verify(
			$ssl_ctx, &Net::SSLeay::VERIFY_PEER, \&verify_client);
		}

	Net::SSLeay::CTX_use_RSAPrivateKey_file(
		$ssl_ctx, $config{'keyfile'},
		&Net::SSLeay::FILETYPE_PEM) || die "Failed to open SSL key";
	Net::SSLeay::CTX_use_certificate_file(
		$ssl_ctx, $config{'keyfile'},
		&Net::SSLeay::FILETYPE_PEM);
	}

# Setup syslog support if possible and if requested
if ($use_syslog) {
	openlog("webmin", "cons,pid,ndelay", "daemon");
	}

# Read MIME types file and add extra types
if ($config{"mimetypes"} ne "") {
	open(MIME, $config{"mimetypes"});
	while(<MIME>) {
		chop; s/#.*$//;
		if (/^(\S+)\s+(.*)$/) {
			$type = $1; @exts = split(/\s+/, $2);
			foreach $ext (@exts) {
				$mime{$ext} = $type;
				}
			}
		}
	close(MIME);
	}
foreach $k (keys %config) {
	if ($k !~ /^addtype_(.*)$/) { next; }
	$mime{$1} = $config{$k};
	}

# get the time zone
if ($config{'log'}) {
	local(@gmt, @lct, $days, $hours, $mins);
	@make_date_marr = ("Jan", "Feb", "Mar", "Apr", "May", "Jun",
		 	   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec");
	@gmt = gmtime(time());
	@lct = localtime(time());
	$days = $lct[3] - $gmt[3];
	$hours = ($days < -1 ? 24 : 1 < $days ? -24 : $days * 24) +
		 $lct[2] - $gmt[2];
	$mins = $hours * 60 + $lct[1] - $gmt[1];
	$timezone = ($mins < 0 ? "-" : "+"); $mins = abs($mins);
	$timezone .= sprintf "%2.2d%2.2d", $mins/60, $mins%60;
	}

if ($config{'inetd'}) {
	# We are being run from inetd - go direct to handling the request
	$SIG{'HUP'} = 'IGNORE';
	$SIG{'TERM'} = 'DEFAULT';
	$SIG{'PIPE'} = 'DEFAULT';
	open(SOCK, "+>&STDIN");

	# Check if it is time for the logfile to be cleared
	if ($config{'logclear'}) {
		local $write_logtime = 0;
		local @st = stat("$config{'logfile'}.time");
		if (@st) {
			if ($st[9]+$config{'logtime'}*60*60 < time()){
				# need to clear log
				$write_logtime = 1;
				unlink($config{'logfile'});
				}
			}
		else { $write_logtime = 1; }
		if ($write_logtime) {
			open(LOGTIME, ">$config{'logfile'}.time");
			print LOGTIME time(),"\n";
			close(LOGTIME);
			}
		}

	# Initialize SSL for this connection
	if ($use_ssl) {
		$ssl_con = Net::SSLeay::new($ssl_ctx);
		Net::SSLeay::set_fd($ssl_con, fileno(SOCK));
		#Net::SSLeay::use_RSAPrivateKey_file(
		#	$ssl_con, $config{'keyfile'},
		#	&Net::SSLeay::FILETYPE_PEM);
		#Net::SSLeay::use_certificate_file(
		#	$ssl_con, $config{'keyfile'},
		#	&Net::SSLeay::FILETYPE_PEM);
		Net::SSLeay::accept($ssl_con) || exit;
		}

	# Work out the hostname for this web server
	if (!$config{'host'}) {
		($myport, $myaddr) =
			unpack_sockaddr_in(getsockname(SOCK));
		$myname = gethostbyaddr($myaddr, AF_INET);
		if ($myname eq "") {
			$myname = inet_ntoa($myaddr);
			}
		$host = $myname;
		}
	else { $host = $config{'host'}; }
	$port = $config{'port'};

	while(&handle_request(getpeername(SOCK), getsockname(SOCK))) { }
	close(SOCK);
	exit;
	}

# Open main socket
$proto = getprotobyname('tcp');
socket(MAIN, PF_INET, SOCK_STREAM, $proto) ||
	die "Failed to open main socket : $!";
setsockopt(MAIN, SOL_SOCKET, SO_REUSEADDR, pack("l", 1));
$baddr = $config{"bind"} ? inet_aton($config{"bind"}) : INADDR_ANY;
for($i=0; $i<5; $i++) {
	last if (bind(MAIN, sockaddr_in($config{port}, $baddr)));
	sleep(1);
	}
die "Failed to bind port $config{port} : $!" if ($i == 5);
listen(MAIN, SOMAXCONN);

if ($config{'listen'}) {
	# Open the socket that allows other webmin servers to find this one
	$proto = getprotobyname('udp');
	if (socket(LISTEN, PF_INET, SOCK_DGRAM, $proto)) {
		setsockopt(LISTEN, SOL_SOCKET, SO_REUSEADDR, pack("l", 1));
		bind(LISTEN, sockaddr_in($config{'listen'}, INADDR_ANY));
		listen(LISTEN, SOMAXCONN);
		}
	else {
		print STDERR "Failed to open listening socket : $!\n";
		$config{'listen'} = 0;
		}
	}


# Split from the controlling terminal
if (fork()) { exit; }
setsid();

# write out the PID file
open(PIDFILE, "> $config{'pidfile'}");
printf PIDFILE "%d\n", getpid();
close(PIDFILE);

# Start the log-clearing process, if needed. This checks every minute
# to see if the log has passed its reset time, and if so clears it
if ($config{'logclear'}) {
	if (!($logclearer = fork())) {
		while(1) {
			local $write_logtime = 0;
			local @st = stat("$config{'logfile'}.time");
			if (@st) {
				if ($st[9]+$config{'logtime'}*60*60 < time()){
					# need to clear log
					$write_logtime = 1;
					unlink($config{'logfile'});
					}
				}
			else { $write_logtime = 1; }
			if ($write_logtime) {
				open(LOGTIME, ">$config{'logfile'}.time");
				print LOGTIME time(),"\n";
				close(LOGTIME);
				}
			sleep(5*60);
			}
		exit;
		}
	push(@childpids, $logclearer);
	}

# Setup the logout time dbm if needed
if ($config{'session'}) {
	eval "use SDBM_File";
	dbmopen(%sessiondb, $config{'sessiondb'}, 0700);
	eval { $sessiondb{'1111111111'} = 'foo bar' };
	if ($@) {
		dbmclose(%sessiondb);
		eval "use NDBM_File";
		dbmopen(%sessiondb, $config{'sessiondb'}, 0700);
		}
	}

# Run the main loop
$SIG{'HUP'} = 'miniserv::trigger_restart';
$SIG{'TERM'} = 'miniserv::term_handler';
$SIG{'PIPE'} = 'IGNORE';
@deny = &to_ipaddress(split(/\s+/, $config{"deny"}));
@allow = &to_ipaddress(split(/\s+/, $config{"allow"}));
$p = 0;
while(1) {
	# wait for a new connection, or a message from a child process
	undef($rmask);
	vec($rmask, fileno(MAIN), 1) = 1;
	if ($config{'passdelay'} || $config{'session'}) {
		for($i=0; $i<@passin; $i++) {
			vec($rmask, fileno($passin[$i]), 1) = 1;
			}
		}
	vec($rmask, fileno(LISTEN), 1) = 1 if ($config{'listen'});

	local $sel = select($rmask, undef, undef, 10);
	if ($need_restart) { &restart_miniserv(); }
	local $time_now = time();

	# Clean up finished processes
	local($pid);
	do {	$pid = waitpid(-1, WNOHANG);
		@childpids = grep { $_ != $pid } @childpids;
		} while($pid > 0);

	# run the unblocking procedure to check if enough time has passed to
	# unblock hosts that heve been blocked because of password failures
	if ($config{'blockhost_failures'}) {
		$i = 0;
		while ($i <= $#deny) {
			if ($blockhosttime{$deny[$i]} && $config{'blockhost_time'} != 0 &&
			    ($time_now - $blockhosttime{$deny[$i]}) >= $config{'blockhost_time'}) {
				# the host can be unblocked now
				$hostfail{$deny[$i]} = 0;
				splice(@deny, $i, 1);
				}
			$i++;
			}
		}

	if ($config{'session'}) {
		# Remove sessions with more than 7 days of inactivity
		foreach $s (keys %sessiondb) {
			local ($user, $ltime) = split(/\s+/, $sessiondb{$s});
			if ($time_now - $ltime > 7*24*60*60) {
				delete($sessiondb{$s});
				}
			}
		}
	next if ($sel <= 0);
	if (vec($rmask, fileno(MAIN), 1)) {
		# got new connection
		$acptaddr = accept(SOCK, MAIN);
		if (!$acptaddr) { next; }

		# create pipes
		if ($config{'passdelay'} || $config{'session'}) {
			$PASSINr = "PASSINr$p"; $PASSINw = "PASSINw$p";
			$PASSOUTr = "PASSOUTr$p"; $PASSOUTw = "PASSOUTw$p";
			$p++;
			pipe($PASSINr, $PASSINw);
			pipe($PASSOUTr, $PASSOUTw);
			select($PASSINw); $| = 1; select($PASSINr); $| = 1;
			select($PASSOUTw); $| = 1; select($PASSOUTw); $| = 1;
			}
		select(STDOUT);

		# Check username of connecting user
		local ($peerp, $peera) = unpack_sockaddr_in($acptaddr);
		$localauth_user = undef;
		if ($config{'localauth'} && inet_ntoa($peera) eq "127.0.0.1") {
			if (open(TCP, "/proc/net/tcp")) {
				# Get the info direct from the kernel
				while(<TCP>) {
					s/^\s+//;
					local @t = split(/[\s:]+/, $_);
					if ($t[1] eq '0100007F' &&
					    $t[2] eq sprintf("%4.4X", $peerp)) {
						$localauth_user = getpwuid($t[11]);
						last;
						}
					}
				close(TCP);
				}
			else {
				# Call lsof for the info
				local $lsofpid = open(LSOF,
					"$config{'localauth'} -i TCP\@127.0.0.1:$peerp |");
				while(<LSOF>) {
					if (/^(\S+)\s+(\d+)\s+(\S+)/ &&
					    $2 != $$ && $2 != $lsofpid) {
						$localauth_user = $3;
						}
					}
				close(LSOF);
				}
			}

		# fork the subprocess
		if (!($handpid = fork())) {
			# setup signal handlers
			$SIG{'TERM'} = 'DEFAULT';
			$SIG{'PIPE'} = 'DEFAULT';
			#$SIG{'CHLD'} = 'IGNORE';
			$SIG{'HUP'} = 'IGNORE';

			# Initialize SSL for this connection
			if ($use_ssl) {
				$ssl_con = Net::SSLeay::new($ssl_ctx);
				Net::SSLeay::set_fd($ssl_con, fileno(SOCK));
				#Net::SSLeay::use_RSAPrivateKey_file(
				#	$ssl_con, $config{'keyfile'},
				#	&Net::SSLeay::FILETYPE_PEM);
				#Net::SSLeay::use_certificate_file(
				#	$ssl_con, $config{'keyfile'},
				#	&Net::SSLeay::FILETYPE_PEM);
				Net::SSLeay::accept($ssl_con) || exit;
				}

			# close useless pipes
			if ($config{'passdelay'} || $config{'session'}) {
				foreach $p (@passin) { close($p); }
				foreach $p (@passout) { close($p); }
				close($PASSINr); close($PASSOUTw);
				}
			close(MAIN);

			# Work out the hostname for this web server
			if (!$config{'host'}) {
				($myport, $myaddr) =
					unpack_sockaddr_in(getsockname(SOCK));
				$myname = gethostbyaddr($myaddr, AF_INET);
				if ($myname eq "") {
					$myname = inet_ntoa($myaddr);
					}
				$host = $myname;
				}
			else { $host = $config{'host'}; }
			$port = $config{'port'};

			local $switched = 0;
			if ($config{'remoteuser'} && $localauth_user && !$<) {
				# Switch to the UID of the remote user
				local @u = getpwnam($localauth_user);
				if (@u) {
					$( = $u[3]; $) = "$u[3] $u[3]";
					$< = $> = $u[2];
					$switched = 1;
					}
				}
			if ($config{'switchuser'} && !$< && !$switched) {
				# Switch to the UID of server user
				local @u = getpwnam($config{'switchuser'});
				if (@u) {
					$( = $u[3]; $) = "$u[3] $u[3]";
					$< = $> = $u[2];
					}
				}

			while(&handle_request($acptaddr, getsockname(SOCK))) { }
			shutdown(SOCK, 1);
			close(SOCK);
			close($PASSINw); close($PASSOUTw);
			exit;
			}
		push(@childpids, $handpid);
		if ($config{'passdelay'} || $config{'session'}) {
			close($PASSINw); close($PASSOUTr);
			push(@passin, $PASSINr); push(@passout, $PASSOUTw);
			}
		close(SOCK);
		}

	if ($config{'listen'} && vec($rmask, fileno(LISTEN), 1)) {
		# Got UDP packet from another webmin server
		local $rcvbuf;
		local $from = recv(LISTEN, $rcvbuf, 1024, 0);
		next if (!$from);
		local $fromip = inet_ntoa((unpack_sockaddr_in($from))[1]);
		local $toip = inet_ntoa((unpack_sockaddr_in(
					 getsockname(LISTEN)))[1]);
		if ((!@deny || !&ip_match($fromip, $toip, @deny)) &&
		    (!@allow || &ip_match($fromip, $toip, @allow))) {
			send(LISTEN, "$config{'host'}:$config{'port'}:".
				     "$use_ssl", 0, $from);
			}
		}

	# check for password-timeout messages from subprocesses
	for($i=0; $i<@passin; $i++) {
		if (vec($rmask, fileno($passin[$i]), 1)) {
			# this sub-process is asking about a password
			$infd = $passin[$i]; $outfd = $passout[$i];
			$inline = <$infd>;
			if ($inline =~ /^delay\s+(\S+)\s+(\S+)\s+(\d+)/) {
				# Got a delay request from a subprocess.. for
				# valid logins, there is no delay (to prevent
				# denial of service attacks), but for invalid
				# logins the delay increases with each failed
				# attempt.
				if ($3) {
					# login OK.. no delay
					print $outfd "0 0\n";
					$hostfail{$2} = 0;
					}
				else {
					# login failed..
					$hostfail{$2}++;
					# add the host to the block list if necessary
 					if ($config{'blockhost_failures'} &&
					    $hostfail{$2} >= $config{'blockhost_failures'}) {
						push(@deny, $2);
						$blockhosttime{$2} = $time_now;
						$blocked = 1;
						if ($use_syslog) {
							local $logtext = "Security alert: Host $2 ".
							  "blocked after $config{'blockhost_failures'} ".
							  "failed logins for user $1";
							syslog("crit", $logtext);
							}
						}
					else {
						$blocked = 0;
						}
					$dl = $userdlay{$1} -
					      int(($time_now - $userlast{$1})/50);
					$dl = $dl < 0 ? 0 : $dl+1;
					print $outfd "$dl $blocked\n";
					$userdlay{$1} = $dl;
					}
				$userlast{$1} = $time_now;
				}
			elsif ($inline =~ /^verify\s+(\S+)/) {
				# Verifying a session ID
				local $session_id = $1;
				if (!defined($sessiondb{$session_id})) {
					print $outfd "0 0\n";
					}
				else {
					local ($user, $ltime) = split(/\s+/, $sessiondb{$session_id});
					if ($config{'logouttime'} &&
					    $time_now - $ltime > $config{'logouttime'}*60) {
						print $outfd "1 ",$time_now - $ltime,"\n";
						delete($sessiondb{$session_id});
						}
					else {
						print $outfd "2 $user\n";
						$sessiondb{$session_id} = "$user $time_now";
						}
					}
				}
			elsif ($inline =~ /^new\s+(\S+)\s+(\S+)/) {
				# Creating a new session
				$sessiondb{$1} = "$2 $time_now";
				}
			elsif ($inline =~ /^delete\s+(\S+)/) {
				# Logging out a session
				print $outfd $sessiondb{$1} ? 1 : 0,"\n";
				delete($sessiondb{$1});
				}
			else {
				# close pipe
				close($infd); close($outfd);
				$passin[$i] = $passout[$i] = undef;
				}
			}
		}
	@passin = grep { defined($_) } @passin;
	@passout = grep { defined($_) } @passout;
	}

# handle_request(remoteaddress, localaddress)
# Where the real work is done
sub handle_request
{
$acptip = inet_ntoa((unpack_sockaddr_in($_[0]))[1]);
$localip = $_[1] ? inet_ntoa((unpack_sockaddr_in($_[1]))[1]) : undef;
if ($config{'loghost'}) {
	$acpthost = gethostbyaddr(inet_aton($acptip), AF_INET);
	$acpthost = $acptip if (!$acpthost);
	}
else {
	$acpthost = $acptip;
	}
$datestr = &http_date(time());
$ok_code = 200;
$ok_message = "Document follows";

# Wait at most 60 secs for start of headers (but only for the first time)
if (!$checked_timeout) {
	local $rmask;
	vec($rmask, fileno(SOCK), 1) = 1;
	local $sel = select($rmask, undef, undef, 60);
	$sel || &http_error(400, "Timeout");
	$checked_timeout++;
	}

# Read the HTTP request and headers
($reqline = &read_line()) =~ s/\r|\n//g;
if (!($reqline =~ /^(GET|POST|HEAD)\s+(.*?)\s+HTTP\/1\..$/)) {
	&http_error(400, "Bad Request");
	}
$method = $1; $request_uri = $page = $2;
%header = ();
local $lastheader;
while(1) {
	($headline = &read_line()) =~ s/\r|\n//g;
	last if ($headline eq "");
	if ($headline =~ /^(\S+):\s+(.*)$/) {
		$header{$lastheader = lc($1)} = $2;
		}
	elsif ($headline =~ /^\s+(.*)$/) {
		$header{$lastheader} .= $headline;
		}
	else {
		&http_error(400, "Bad Header $headline");
		}
	}
if (defined($header{'host'})) {
	if ($header{'host'} =~ /^([^:]+):([0-9]+)$/) { $host = $1; $port = $2; }
	else { $host = $header{'host'}; }
	}
undef(%in);
if ($page =~ /^([^\?]+)\?(.*)$/) {
	# There is some query string information
	$page = $1;
	$querystring = $2;
	if ($querystring !~ /=/) {
		$queryargs = $querystring;
		$queryargs =~ s/\+/ /g;
    		$queryargs =~ s/%(..)/pack("c",hex($1))/ge;
		$querystring = "";
		}
	else {
		# Parse query-string parameters
		local @in = split(/\&/, $querystring);
		foreach $i (@in) {
			local ($k, $v) = split(/=/, $i, 2);
			$k =~ s/\+/ /g; $k =~ s/%(..)/pack("c",hex($1))/ge;
			$v =~ s/\+/ /g; $v =~ s/%(..)/pack("c",hex($1))/ge;
			$in{$k} = $v;
			}
		}
	}
$posted_data = undef;
if ($method eq 'POST' &&
    $header{'content-type'} eq 'application/x-www-form-urlencoded') {
	# Read in posted query string information
	$clen = $header{"content-length"};
	while(length($posted_data) < $clen) {
		$buf = &read_data($clen - length($posted_data));
		if (!length($buf)) {
			&http_error(500, "Failed to read POST request");
			}
		$posted_data .= $buf;
		}
	local @in = split(/\&/, $posted_data);
	foreach $i (@in) {
		local ($k, $v) = split(/=/, $i, 2);
		$k =~ s/\+/ /g; $k =~ s/%(..)/pack("c",hex($1))/ge;
		$v =~ s/\+/ /g; $v =~ s/%(..)/pack("c",hex($1))/ge;
		$in{$k} = $v;
		}
	}

# replace %XX sequences in page
$page =~ s/%(..)/pack("c",hex($1))/ge;

# check address against access list
if (@deny && &ip_match($acptip, $localip, @deny) ||
    @allow && !&ip_match($acptip, $localip, @allow)) {
	&http_error(403, "Access denied for $acptip");
	return 0;
	}

# check for the logout flag file, and if existant deny authentication
if ($config{'logout'} && -r $config{'logout'}.$in{'miniserv_logout_id'}) {
	$deny_authentication++;
	open(LOGOUT, $config{'logout'}.$in{'miniserv_logout_id'});
	chop($count = <LOGOUT>);
	close(LOGOUT);
	$count--;
	if ($count > 0) {
		open(LOGOUT, ">$config{'logout'}$in{'miniserv_logout_id'}");
		print LOGOUT "$count\n";
		close(LOGOUT);
		}
	else {
		unlink($config{'logout'}.$in{'miniserv_logout_id'});
		}
	}

# Check for password if needed
if (%users) {
	$validated = 0;
	$blocked = 0;

	# Session authentication is never used for connections by
	# another webmin server
	if ($header{'user-agent'} =~ /webmin/i) {
		$config{'session'} = 0;
		}

	# check for SSL authentication
	if ($use_ssl && $verified_client) {
		$peername = Net::SSLeay::X509_NAME_oneline(
				Net::SSLeay::X509_get_subject_name(
					Net::SSLeay::get_peer_certificate(
						$ssl_con)));
		foreach $u (keys %certs) {
			if ($certs{$u} eq $peername) {
				$authuser = $u;
				$validated = 2;
				last;
				}
			}
		}

	# Check for normal HTTP authentication
	if (!$validated && !$deny_authentication && !$config{'session'} &&
	    $header{authorization} =~ /^basic\s+(\S+)$/i) {
		# authorization given..
		($authuser, $authpass) = split(/:/, &b64decode($1));
		$validated = &validate_user($authuser, $authpass);

		if ($config{'passdelay'} && !$config{'inetd'}) {
			# check with main process for delay
			print $PASSINw "delay $authuser $acptip $validated\n";
			<$PASSOUTr> =~ /(\d+) (\d+)/;
			$blocked = $2;
			sleep($1);
			}
		}

	# Check for new session validation
	if ($config{'session'} && !$deny_authentication && $page eq $config{'session_login'}) {
		local $ok = &validate_user($in{'user'}, $in{'pass'});

		# check if the test cookie is set
		#if ($header{'cookie'} !~ /testing=1/ && $in{'user'}) {
		#	&http_error(500, "No cookies",
		#		    "Your browser does not support cookies, ".
		#		    "which are required for Webmin to work in ".
		#		    "session authentication mode");
		#	}

		# check with main process for delay
		if ($config{'passdelay'} && $in{'user'}) {
			print $PASSINw "delay $in{'user'} $acptip $ok\n";
			<$PASSOUTr> =~ /(\d+) (\d+)/;
			$blocked = $2;
			sleep($1);
			}

		if ($ok) {
			# Logged in OK! Tell the main process about the new SID
			local $sid = time();
			local $mul = 1;
			foreach $c (split(//, crypt($in{'pass'}, substr($$, -2)))) {
				$sid += ord($c) * $mul;
				$mul *= 3;
				}
			print $PASSINw "new $sid $in{'user'}\n";

			# Set cookie and redirect
			&write_data("HTTP/1.0 302 Moved Temporarily\r\n");
			&write_data("Date: $datestr\r\n");
			&write_data("Server: $config{'server'}\r\n");
			$portstr = $port == 80 && !$use_ssl ? "" :
				   $port == 443 && $use_ssl ? "" : ":$port";
			$prot = $use_ssl ? "https" : "http";
			if ($in{'save'}) {
				&write_data("Set-Cookie: sid=$sid; path=/; expires=\"Fri, 1-Jan-2038 00:00:01\"\r\n");
				}
			else {
				&write_data("Set-Cookie: sid=$sid; path=/\r\n");
				}
			&write_data("Location: $prot://$host$portstr$in{'page'}\r\n");
			&write_keep_alive(0);
			&write_data("\r\n");
			&log_request($acpthost, $authuser, $reqline, 302, 0);
			return 0;
			}
		elsif ($in{'logout'} && $header{'cookie'} =~ /sid=(\d+)/) {
			# Logout clicked .. remove the session
			print $PASSINw "delete $1\n";
			local $dummy = <$PASSINr>;
			$logout = 1;
			$already_session_id = undef;
			}
		else {
			# Login failed .. display the form again
			$failed_user = $in{'user'};
			$request_uri = $in{'page'};
			$already_session_id = undef;
			}
		}

	# Check for an existing session
	if ($config{'session'} && !$validated) {
		if ($already_session_id) {
			$session_id = $already_session_id;
			$authuser = $already_authuser;
			$validated = 1;
			}
		elsif (!$deny_authentication && $header{'cookie'} =~ /sid=(\d+)/) {
			$session_id = $1;
			print $PASSINw "verify $session_id\n";
			<$PASSOUTr> =~ /(\d+)\s+(\S+)/;
			if ($1 == 2) {
				# Valid session continuation
				$validated = 1;
				$authuser = $2;
				$already_session_id = $session_id;
				$already_authuser = $authuser;
				}
			elsif ($1 == 1) {
				# Session timed out
				$timed_out = $2;
				}
			else {
				# Invalid session ID .. don't set verified
				}
			}
		}

	# Check for local authentication
	if ($localauth_user) {
		if (defined($users{$localauth_user})) {
			$validated = 1;
			$authuser = $localauth_user;
			}
		else {
			$localauth_user = undef;
			}
		}

	if (!$validated) {
		if ($blocked == 0) {
			# No password given.. ask
			if ($config{'session'}) {
				# Force CGI for session login
				$validated = 1;
				if ($logout) {
					$querystring .= "&logout=1&page=/";
					}
				else {
					$querystring = "page=".&urlize($request_uri);
					}
				$querystring .= "&failed=$failed_user" if ($failed_user);
				$querystring .= "&timed_out=$timed_out" if ($timed_out);
				$queryargs = "";
				$page = $config{'session_login'};
				}
			else {
				# Ask for login with HTTP authentication
				&write_data("HTTP/1.0 401 Unauthorized\r\n");
				&write_data("Date: $datestr\r\n");
				&write_data("Server: $config{'server'}\r\n");
				&write_data("WWW-authenticate: Basic ".
					   "realm=\"$config{'realm'}\"\r\n");
				&write_keep_alive(0);
				&write_data("Content-type: text/html\r\n");
				&write_data("\r\n");
				&reset_byte_count();
				&write_data("<html>\n");
				&write_data("<head><title>Unauthorized</title></head>\n");
				&write_data("<body><h1>Unauthorized</h1>\n");
				&write_data("A password is required to access this\n");
				&write_data("web server. Please try again. <p>\n");
				&write_data("</body></html>\n");
				&log_request($acpthost, undef, $reqline, 401, &byte_count());
				return 0;
				}
			}
		else {
			# when the host has been blocked, give it an error message
			&http_error(403, "Access denied for $acptip. The host has been blocked "
				."because of too many authentication failures.");
			}
		}

	# Check per-user IP access control
	if ($deny{$authuser} && &ip_match($acptip, $localip, @{$deny{$authuser}}) ||
	    $allow{$authuser} && !&ip_match($acptip, $localip, @{$allow{$authuser}})) {
		&http_error(403, "Access denied for $acptip");
		return 0;
		}
	}

# Figure out what kind of page was requested
rerun:
$simple = &simplify_path($page, $bogus);
$simple =~ s/[\000-\037]//g;
if ($bogus) {
	&http_error(400, "Invalid path");
	}
undef($full);
if ($config{'preroot'}) {
	# Look in the template root directory first
	$is_directory = 1;
	$sofar = "";
	$full = $config{"preroot"} . $sofar;
	$scriptname = $simple;
	foreach $b (split(/\//, $simple)) {
		if ($b ne "") { $sofar .= "/$b"; }
		$full = $config{"preroot"} . $sofar;
		@st = stat($full);
		if (!@st) { undef($full); last; }

		# Check if this is a directory
		if (-d $full) {
			# It is.. go on parsing
			$is_directory = 1;
			next;
			}
		else { $is_directory = 0; }

		# Check if this is a CGI program
		if (&get_type($full) eq "internal/cgi") {
			$pathinfo = substr($simple, length($sofar));
			$pathinfo .= "/" if ($page =~ /\/$/);
			$scriptname = $sofar;
			last;
			}
		}
	if ($full) {
		if ($sofar eq '') {
			$cgi_pwd = $config{'root'};
			}
		else {
			"$config{'root'}$sofar" =~ /^(.*\/)[^\/]+$/;
			$cgi_pwd = $1;
			}
		if ($is_directory) {
			# Check for index files in the directory
			foreach $idx (split(/\s+/, $config{"index_docs"})) {
				$idxfull = "$full/$idx";
				if (-r $idxfull && !(-d $idxfull)) {
					$full = $idxfull;
					$is_directory = 0;
					$scriptname .= "/"
						if ($scriptname ne "/");
					last;
					}
				}
			}
		}
	}
if (!$full || $is_directory) {
	$sofar = "";
	$full = $config{"root"} . $sofar;
	$scriptname = $simple;
	foreach $b (split(/\//, $simple)) {
		if ($b ne "") { $sofar .= "/$b"; }
		$full = $config{"root"} . $sofar;
		@st = stat($full);
		if (!@st) { &http_error(404, "File not found"); }

		# Check if this is a directory
		if (-d $full) {
			# It is.. go on parsing
			next;
			}

		# Check if this is a CGI program
		if (&get_type($full) eq "internal/cgi") {
			$pathinfo = substr($simple, length($sofar));
			$pathinfo .= "/" if ($page =~ /\/$/);
			$scriptname = $sofar;
			last;
			}
		}
	$full =~ /^(.*\/)[^\/]+$/; $cgi_pwd = $1;
	}

# check filename against denyfile regexp
local $denyfile = $config{'denyfile'};
if ($denyfile && $full =~ /$denyfile/) {
	&http_error(403, "Access denied to $page");
	return 0;
	}

# Reached the end of the path OK.. see what we've got
if (-d $full) {
	# See if the URL ends with a / as it should
	if ($page !~ /\/$/) {
		# It doesn't.. redirect
		&write_data("HTTP/1.0 302 Moved Temporarily\r\n");
		$portstr = $port == 80 && !$use_ssl ? "" :
			   $port == 443 && $use_ssl ? "" : ":$port";
		&write_data("Date: $datestr\r\n");
		&write_data("Server: $config{server}\r\n");
		$prot = $use_ssl ? "https" : "http";
		&write_data("Location: $prot://$host$portstr$page/\r\n");
		&write_keep_alive(0);
		&write_data("\r\n");
		&log_request($acpthost, $authuser, $reqline, 302, 0);
		return 0;
		}
	# A directory.. check for index files
	foreach $idx (split(/\s+/, $config{"index_docs"})) {
		$idxfull = "$full/$idx";
		if (-r $idxfull && !(-d $idxfull)) {
			$cgi_pwd = $full;
			$full = $idxfull;
			$scriptname .= "/" if ($scriptname ne "/");
			last;
			}
		}
	}
if (-d $full) {
	# This is definately a directory.. list it
	&write_data("HTTP/1.0 $ok_code $ok_message\r\n");
	&write_data("Date: $datestr\r\n");
	&write_data("Server: $config{server}\r\n");
	&write_data("Content-type: text/html\r\n");
	&write_keep_alive(0);
	&write_data("\r\n");
	&reset_byte_count();
	&write_data("<h1>Index of $simple</h1>\n");
	&write_data("<pre>\n");
	&write_data(sprintf "%-35.35s %-20.20s %-10.10s\n",
			"Name", "Last Modified", "Size");
	&write_data("<hr>\n");
	opendir(DIR, $full);
	while($df = readdir(DIR)) {
		if ($df =~ /^\./) { next; }
		(@stbuf = stat("$full/$df")) || next;
		if (-d "$full/$df") { $df .= "/"; }
		@tm = localtime($stbuf[9]);
		$fdate = sprintf "%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d",
				$tm[3],$tm[4]+1,$tm[5]+1900,
				$tm[0],$tm[1],$tm[2];
		$len = length($df); $rest = " "x(35-$len);
		&write_data(sprintf
		 "<a href=\"%s\">%-${len}.${len}s</a>$rest %-20.20s %-10.10s\n",
		 $df, $df, $fdate, $stbuf[7]);
		}
	closedir(DIR);
	&log_request($acpthost, $authuser, $reqline, $ok_code, &byte_count());
	return 0;
	}

# CGI or normal file
local $rv;
if (&get_type($full) eq "internal/cgi") {
	# A CGI program to execute
	$envtz = $ENV{"TZ"};
	$envuser = $ENV{"USER"};
	$envpath = $ENV{"PATH"};
	foreach (keys %ENV) { delete($ENV{$_}); }
	$ENV{"PATH"} = $envpath if ($envpath);
	$ENV{"TZ"} = $envtz if ($envtz);
	$ENV{"USER"} = $envuser if ($envuser);
	$ENV{"HOME"} = $user_homedir;
	$ENV{"SERVER_SOFTWARE"} = $config{"server"};
	$ENV{"SERVER_NAME"} = $host;
	$ENV{"SERVER_ADMIN"} = $config{"email"};
	$ENV{"SERVER_ROOT"} = $config{"root"};
	$ENV{"SERVER_PORT"} = $port;
	$ENV{"REMOTE_HOST"} = $acpthost;
	$ENV{"REMOTE_ADDR"} = $acptip;
	$ENV{"REMOTE_USER"} = $authuser if (defined($authuser));
	$ENV{"SSL_USER"} = $peername if ($validated == 2);
	$ENV{"DOCUMENT_ROOT"} = $config{"root"};
	$ENV{"GATEWAY_INTERFACE"} = "CGI/1.1";
	$ENV{"SERVER_PROTOCOL"} = "HTTP/1.0";
	$ENV{"REQUEST_METHOD"} = $method;
	$ENV{"SCRIPT_NAME"} = $scriptname;
	$ENV{"REQUEST_URI"} = $request_uri;
	$ENV{"PATH_INFO"} = $pathinfo;
	$ENV{"PATH_TRANSLATED"} = "$config{root}/$pathinfo";
	$ENV{"QUERY_STRING"} = $querystring;
	$ENV{"MINISERV_CONFIG"} = $conf;
	$ENV{"HTTPS"} = "ON" if ($use_ssl);
	$ENV{"SESSION_ID"} = $session_id if ($session_id);
	$ENV{"LOCAL_USER"} = $localauth_user if ($localauth_user);
	if (defined($header{"content-length"})) {
		$ENV{"CONTENT_LENGTH"} = $header{"content-length"};
		}
	if (defined($header{"content-type"})) {
		$ENV{"CONTENT_TYPE"} = $header{"content-type"};
		}
	foreach $h (keys %header) {
		($hname = $h) =~ tr/a-z/A-Z/;
		$hname =~ s/\-/_/g;
		$ENV{"HTTP_$hname"} = $header{$h};
		}
	$ENV{"PWD"} = $cgi_pwd;
	foreach $k (keys %config) {
		if ($k =~ /^env_(\S+)$/) {
			$ENV{$1} = $config{$k};
			}
		}
	delete($ENV{'HTTP_AUTHORIZATION'});
	$ENV{'HTTP_COOKIE'} =~ s/;?\s*sid=(\d+)//;

	# Check if the CGI can be handled internally
	open(CGI, $full);
	local $first = <CGI>;
	close(CGI);
	$first =~ s/[#!\r\n]//g;
	$nph_script = ($full =~ /\/nph-([^\/]+)$/);
	if (!$config{'forkcgis'} && $first eq $perl_path && $] >= 5.004) {
		# setup environment for eval
		chdir($ENV{"PWD"});
		@ARGV = split(/\s+/, $queryargs);
		$0 = $full;
		if ($posted_data) {
			# Already read the post input
			$postinput = $posted_data;
			}
		elsif ($method eq "POST") {
			$clen = $header{"content-length"};
			while(length($postinput) < $clen) {
				$buf = &read_data($clen - length($postinput));
				if (!length($buf)) {
					&http_error(500, "Failed to read ".
							 "POST request");
					}
				$postinput .= $buf;
				}
			}
		$SIG{'CHLD'} = 'DEFAULT';
		eval {
			# Have SOCK closed if the perl exec's something
			use Fcntl;
			fcntl(SOCK, F_SETFD, FD_CLOEXEC);
			};
		shutdown(SOCK, 0);

		if ($config{'log'}) {
			open(MINISERVLOG, ">>$config{'logfile'}");
			chmod(0600, $config{'logfile'});
			}
		$doing_eval = 1;
		eval {
			package main;
			tie(*STDOUT, 'miniserv');
			tie(*STDIN, 'miniserv');
			do $miniserv::full;
			die $@ if ($@);
			};
		$doing_eval = 0;
		if ($@) {
			# Error in perl!
			&http_error(500, "Perl execution failed", $@);
			}
		elsif (!$doneheaders && !$nph_script) {
			&http_error(500, "Missing Headers");
			}
		#close(SOCK);
		$rv = 0;
		}
	else {
		# fork the process that actually executes the CGI
		pipe(CGIINr, CGIINw);
		pipe(CGIOUTr, CGIOUTw);
		pipe(CGIERRr, CGIERRw);
		if (!($cgipid = fork())) {
			chdir($ENV{"PWD"});
			close(SOCK);
			open(STDIN, "<&CGIINr");
			open(STDOUT, ">&CGIOUTw");
			open(STDERR, ">&CGIERRw");
			close(CGIINw); close(CGIOUTr); close(CGIERRr);
                        ### KLUDGE ### --kc
                        if ($full =~ /[.]php[34]?$/) {
                            $queryargs = "$full $queryargs";
                            $full = $PHP_CGI_PATH;
                        }
			exec($full, split(/\s+/, $queryargs));
			print STDERR "Failed to exec $full : $!\n";
			exit;
			}
		close(CGIINr); close(CGIOUTw); close(CGIERRw);

		# send post data
		if ($posted_data) {
			# already read the posted data
			print CGIINw $posted_data;
			}
		elsif ($method eq "POST") {
			$got = 0; $clen = $header{"content-length"};
			while($got < $clen) {
				$buf = &read_data($clen-$got);
				if (!length($buf)) {
					kill('TERM', $cgipid);
					&http_error(500, "Failed to read ".
							 "POST request");
					}
				$got += length($buf);
				print CGIINw $buf;
				}
			}
		close(CGIINw);
		shutdown(SOCK, 0);

		if (!$nph_script) {
			# read back cgi headers
			select(CGIOUTr); $|=1; select(STDOUT);
			$got_blank = 0;
			while(1) {
				$line = <CGIOUTr>;
				$line =~ s/\r|\n//g;
				if ($line eq "") {
					if ($got_blank || %cgiheader) { last; }
					$got_blank++;
					next;
					}
				($line =~ /^(\S+):\s+(.*)$/) ||
					&http_error(500, "Bad Header",
						    &read_errors(CGIERRr));
				$cgiheader{lc($1)} = $2;
				}
			if ($cgiheader{"location"}) {
				&write_data("HTTP/1.0 302 Moved Temporarily\r\n");
				&write_data("Date: $datestr\r\n");
				&write_data("Server: $config{'server'}\r\n");
				&write_keep_alive(0);
				# ignore the rest of the output. This is a hack, but
				# is necessary for IE in some cases :(
				close(CGIOUTr); close(CGIERRr);
				}
			elsif ($cgiheader{"content-type"} eq "") {
				&http_error(500, "Missing Content-Type Header",
					    &read_errors(CGIERRr));
				}
			else {
				&write_data("HTTP/1.0 $ok_code $ok_message\r\n");
				&write_data("Date: $datestr\r\n");
				&write_data("Server: $config{'server'}\r\n");
				&write_keep_alive(0);
				}
			foreach $h (keys %cgiheader) {
				&write_data("$h: $cgiheader{$h}\r\n");
				}
			&write_data("\r\n");
			}
		&reset_byte_count();
		while($line = <CGIOUTr>) {
			&write_data($line);
			}
		close(CGIOUTr); close(CGIERRr);
		$rv = 0;
		}
	}
else {
	# A file to output
	local @st = stat($full);
	open(FILE, $full) || &http_error(404, "Failed to open file");
	&write_data("HTTP/1.0 $ok_code $ok_message\r\n");
	&write_data("Date: $datestr\r\n");
	&write_data("Server: $config{server}\r\n");
	&write_data("Content-type: ".&get_type($full)."\r\n");
	&write_data("Content-length: $st[7]\r\n");
	&write_data("Last-Modified: ".&http_date($st[9])."\r\n");
	&write_keep_alive();
	&write_data("\r\n");
	&reset_byte_count();
	while(read(FILE, $buf, 1024) > 0) {
		&write_data($buf);
		}
	close(FILE);
	$rv = &check_keep_alive();
	}

# log the request
&log_request($acpthost, $authuser, $reqline,
	     $cgiheader{"location"} ? "302" : $ok_code, &byte_count());
return $rv;
}

# http_error(code, message, body, [dontexit])
sub http_error
{
close(CGIOUT);
local $eh = $error_handler_recurse ? undef :
	    $config{"error_handler_$_[0]"} ? $config{"error_handler_$_[0]"} :
	    $config{'error_handler'} ? $config{'error_handler'} : undef;
if ($eh) {
	# Call a CGI program for the error
	$page = "/$eh";
	$querystring = "code=$_[0]&message=".&urlize($_[1]).
		       "&body=".&urlize($_[2]);
	$error_handler_recurse++;
	$ok_code = $_[0];
	$ok_message = $_[1];
	goto rerun;
	}
else {
	# Use the standard error message display
	&write_data("HTTP/1.0 $_[0] $_[1]\r\n");
	&write_data("Server: $config{server}\r\n");
	&write_data("Date: $datestr\r\n");
	&write_data("Content-type: text/html\r\n");
	&write_keep_alive(0);
	&write_data("\r\n");
	&reset_byte_count();
	&write_data("<h1>Error - $_[1]</h1>\n");
	if ($_[2]) {
		&write_data("<pre>$_[2]</pre>\n");
		}
	}
&log_request($acpthost, $authuser, $reqline, $_[0], &byte_count())
	if ($reqline);
shutdown(SOCK, 1);
exit if (!$_[3]);
}

sub get_type
{
if ($_[0] =~ /\.([A-z0-9]+)$/) {
	$t = $mime{$1};
	if ($t ne "") {
		return $t;
		}
    }
if (-f $_[0] && -x $_[0] && $_[0] =~ m,/cgi-bin/,) { ### KLUDGE ### --kc
    return "internal/cgi";
}
return "text/plain";
}

# simplify_path(path, bogus)
# Given a path, maybe containing stuff like ".." and "." convert it to a
# clean, absolute form.
sub simplify_path
{
local($dir, @bits, @fixedbits, $b);
$dir = $_[0];
$dir =~ s/^\/+//g;
$dir =~ s/\/+$//g;
@bits = split(/\/+/, $dir);
@fixedbits = ();
$_[1] = 0;
foreach $b (@bits) {
        if ($b eq ".") {
                # Do nothing..
                }
        elsif ($b eq "..") {
                # Remove last dir
                if (scalar(@fixedbits) == 0) {
                        $_[1] = 1;
                        return "/";
                        }
                pop(@fixedbits);
                }
        else {
                # Add dir to list
                push(@fixedbits, $b);
                }
        }
return "/" . join('/', @fixedbits);
}

# b64decode(string)
# Converts a string from base64 format to normal
sub b64decode
{
    local($str) = $_[0];
    local($res);
    $str =~ tr|A-Za-z0-9+=/||cd;
    $str =~ s/=+$//;
    $str =~ tr|A-Za-z0-9+/| -_|;
    while ($str =~ /(.{1,60})/gs) {
        my $len = chr(32 + length($1)*3/4);
        $res .= unpack("u", $len . $1 );
    }
    return $res;
}

# ip_match(remoteip, localip, [match]+)
# Checks an IP address against a list of IPs, networks and networks/masks
sub ip_match
{
local(@io, @mo, @ms, $i, $j);
@io = split(/\./, $_[0]);
local $hn;
if (!defined($hn = $ip_match_cache{$_[0]})) {
	$hn = gethostbyaddr(inet_aton($_[0]), AF_INET);
	$hn = "" if ((&to_ipaddress($hn))[0] ne $_[0]);
	$ip_match_cache{$_[0]} = $hn;
	}
for($i=2; $i<@_; $i++) {
	local $mismatch = 0;
	if ($_[$i] =~ /^(\S+)\/(\S+)$/) {
		# Compare with network/mask
		@mo = split(/\./, $1); @ms = split(/\./, $2);
		for($j=0; $j<4; $j++) {
			if ((int($io[$j]) & int($ms[$j])) != int($mo[$j])) {
				$mismatch = 1;
				}
			}
		}
	elsif ($_[$i] =~ /^\*(\S+)$/) {
		# Compare with hostname regexp
		$mismatch = 1 if ($hn !~ /$1$/);
		}
	elsif ($_[$i] eq 'LOCAL') {
		# Compare with local network
		local @lo = split(/\./, $_[1]);
		if ($lo[0] < 128) {
			$mismatch = 1 if ($lo[0] != $io[0]);
			}
		elsif ($lo[0] < 192) {
			$mismatch = 1 if ($lo[0] != $io[0] ||
					  $lo[1] != $io[1]);
			}
		else {
			$mismatch = 1 if ($lo[0] != $io[0] ||
					  $lo[1] != $io[1] ||
					  $lo[2] != $io[2]);
			}
		}
	else {
		# Compare with IP or network
		@mo = split(/\./, $_[$i]);
		while(@mo && !$mo[$#mo]) { pop(@mo); }
		for($j=0; $j<@mo; $j++) {
			if ($mo[$j] != $io[$j]) {
				$mismatch = 1;
				}
			}
		}
	return 1 if (!$mismatch);
	}
return 0;
}

# restart_miniserv()
# Called when a SIGHUP is received to restart the web server. This is done
# by exec()ing perl with the same command line as was originally used
sub restart_miniserv
{
close(SOCK); close(MAIN);
foreach $p (@passin) { close($p); }
foreach $p (@passout) { close($p); }
if ($logclearer) { kill('TERM', $logclearer);	}
exec($perl_path, $miniserv_path, @miniserv_argv);
die "Failed to restart miniserv with $perl_path $miniserv_path";
}

sub trigger_restart
{
$need_restart = 1;
}

sub to_ipaddress
{
local (@rv, $i);
foreach $i (@_) {
	if ($i =~ /(\S+)\/(\S+)/ || $i =~ /^\*\S+$/ ||
	    $i eq 'LOCAL') { push(@rv, $i); }
	else { push(@rv, join('.', unpack("CCCC", inet_aton($i)))); }
	}
return @rv;
}

# read_line()
# Reads one line from SOCK or SSL
sub read_line
{
local($idx, $more, $rv);
if ($use_ssl) {
	while(($idx = index($read_buffer, "\n")) < 0) {
		# need to read more..
		if (!($more = Net::SSLeay::read($ssl_con))) {
			# end of the data
			$rv = $read_buffer;
			undef($read_buffer);
			return $rv;
			}
		$read_buffer .= $more;
		}
	$rv = substr($read_buffer, 0, $idx+1);
	$read_buffer = substr($read_buffer, $idx+1);
	return $rv;
	}
else { return <SOCK>; }
}

# read_data(length)
# Reads up to some amount of data from SOCK or the SSL connection
sub read_data
{
if ($use_ssl) {
	local($rv);
	if (length($read_buffer)) {
		$rv = $read_buffer;
		undef($read_buffer);
		return $rv;
		}
	else {
		return Net::SSLeay::read($ssl_con, $_[0]);
		}
	}
else {
	local $buf;
	read(SOCK, $buf, $_[0]) || return undef;
	return $buf;
	}
}

# write_data(data)
# Writes a string to SOCK or the SSL connection
sub write_data
{
if ($use_ssl) {
	Net::SSLeay::write($ssl_con, $_[0]);
	}
else {
	syswrite(SOCK, $_[0], length($_[0]));
	}
$write_data_count += length($_[0]);
}

# reset_byte_count()
sub reset_byte_count { $write_data_count = 0; }

# byte_count()
sub byte_count { return $write_data_count; }

# log_request(hostname, user, request, code, bytes)
sub log_request
{
if ($config{'log'}) {
	local(@tm, $dstr, $user, $ident, $headers);
	if ($config{'logident'}) {
		# add support for rfc1413 identity checking here
		}
	else { $ident = "-"; }
	@tm = localtime(time());
	$dstr = sprintf "%2.2d/%s/%4.4d:%2.2d:%2.2d:%2.2d %s",
			$tm[3], $make_date_marr[$tm[4]], $tm[5]+1900,
	                $tm[2], $tm[1], $tm[0], $timezone;
	$user = $_[1] ? $_[1] : "-";
	if (fileno(MINISERVLOG)) {
		seek(MINISERVLOG, 0, 2);
		}
	else {
		open(MINISERVLOG, ">>$config{'logfile'}");
		chmod(0600, $config{'logfile'});
		}
	foreach $h (split(/\s+/, $config{'logheaders'})) {
		$headers .= " $h=\"$header{$h}\"";
		}
	print MINISERVLOG "$_[0] $ident $user [$dstr] \"$_[2]\" ",
			  "$_[3] $_[4]$headers\n";
	close(MINISERVLOG);
	}
}

# read_errors(handle)
# Read and return all input from some filehandle
sub read_errors
{
local($fh, $_, $rv);
$fh = $_[0];
while(<$fh>) { $rv .= $_; }
return $rv;
}

sub write_keep_alive
{
local $mode;
if (@_) { $mode = $_[0]; }
else { $mode = &check_keep_alive(); }
&write_data("Connection: ".($mode ? "Keep-Alive" : "close")."\r\n");
}

sub check_keep_alive
{
return $header{'connection'} =~ /keep-alive/i;
}

sub term_handler
{
if (@childpids) {
	kill('TERM', @childpids);
	}
exit(1);
}

sub http_date
{
local @tm = gmtime($_[0]);
return sprintf "%s, %d %s %d %2.2d:%2.2d:%2.2d GMT",
		$weekday[$tm[6]], $tm[3], $month[$tm[4]], $tm[5]+1900,
		$tm[2], $tm[1], $tm[0];
}

sub TIEHANDLE
{
my $i; bless \$i, shift;
}

sub WRITE
{
$r = shift;
my($buf,$len,$offset) = @_;
&write_to_sock(substr($buf, $offset, $len));
}

sub PRINT
{
$r = shift;
$$r++;
&write_to_sock(@_);
}

sub PRINTF
{
shift;
my $fmt = shift;
&write_to_sock(sprintf $fmt, @_);
}

sub READ
{
$r = shift;
substr($_[0], $_[2], $_[1]) = substr($postinput, $postpos, $_[1]);
$postpos += $_[1];
}

sub OPEN
{
print STDERR "open() called - should never happen!\n";
}

sub READLINE
{
if ($postpos >= length($postinput)) {
	return undef;
	}
local $idx = index($postinput, "\n", $postpos);
if ($idx < 0) {
	local $rv = substr($postinput, $postpos);
	$postpos = length($postinput);
	return $rv;
	}
else {
	local $rv = substr($postinput, $postpos, $idx-$postpos+1);
	$postpos = $idx+1;
	return $rv;
	}
}

sub GETC
{
return $postpos >= length($postinput) ? undef
				      : substr($postinput, $postpos++, 1);
}

sub CLOSE { }

sub DESTROY { }

# write_to_sock(data, ...)
sub write_to_sock
{
foreach $d (@_) {
	if ($doneheaders || $miniserv::nph_script) {
		&write_data($d);
		}
	else {
		$headers .= $d;
		while(!$doneheaders && $headers =~ s/^(.*)(\r)?\n//) {
			if ($1 =~ /^(\S+):\s+(.*)$/) {
				$cgiheader{lc($1)} = $2;
				}
			elsif ($1 !~ /\S/) {
				$doneheaders++;
				}
			else {
				&http_error(500, "Bad Header");
				}
			}
		if ($doneheaders) {
			if ($cgiheader{"location"}) {
				&write_data(
					"HTTP/1.0 302 Moved Temporarily\r\n");
				&write_data("Date: $datestr\r\n");
				&write_data("Server: $config{server}\r\n");
				&write_keep_alive(0);
				}
			elsif ($cgiheader{"content-type"} eq "") {
				&http_error(500, "Missing Content-Type Header");
				}
			else {
				&write_data("HTTP/1.0 $ok_code $ok_message\r\n");
				&write_data("Date: $datestr\r\n");
				&write_data("Server: $config{server}\r\n");
				&write_keep_alive(0);
				}
			foreach $h (keys %cgiheader) {
				&write_data("$h: $cgiheader{$h}\r\n");
				}
			&write_data("\r\n");
			&reset_byte_count();
			&write_data($headers);
			}
		}
	}
}

sub verify_client
{
local $cert = Net::SSLeay::X509_STORE_CTX_get_current_cert($_[1]);
if ($cert) {
	local $errnum = Net::SSLeay::X509_STORE_CTX_get_error($_[1]);
	$verified_client = 1 if (!$errnum);
	}
return 1;
}

sub END
{
if ($doing_eval) {
	# A CGI program called exit! This is a horrible hack to
	# finish up before really exiting
	close(SOCK);
	&log_request($acpthost, $authuser, $reqline,
		     $cgiheader{"location"} ? "302" : $ok_code, &byte_count());
	}
}

# urlize
# Convert a string to a form ok for putting in a URL
sub urlize {
  local($tmp, $tmp2, $c);
  $tmp = $_[0];
  $tmp2 = "";
  while(($c = chop($tmp)) ne "") {
	if ($c !~ /[A-z0-9]/) {
		$c = sprintf("%%%2.2X", ord($c));
		}
	$tmp2 = $c . $tmp2;
	}
  return $tmp2;
}

# validate_user(username, password)
sub validate_user
{
return 0 if (!$_[0] || !$users{$_[0]});
if ($users{$_[0]} eq 'x' && $use_pam) {
	$pam_username = $_[0];
	$pam_password = $_[1];
	local $pamh = new Authen::PAM("webmin", $pam_username, \&pam_conv_func);
	if (!ref($pamh)) {
		print STDERR "PAM init failed : $pamh\n";
		return 0;
		}
	local $pam_ret = $pamh->pam_authenticate();
	return $pam_ret == PAM_SUCCESS ? 1 : 0;
	}
else {
	return $users{$_[0]} eq crypt($_[1], $users{$_[0]}) ? 1 : 0;
	}
}

# the PAM conversation function for interactive logins
sub pam_conv_func
{
my @res;
while ( @_ ) {
	my $code = shift;
	my $msg = shift;
	my $ans = "";

	$ans = $pam_username if ($code == PAM_PROMPT_ECHO_ON() );
	$ans = $pam_password if ($code == PAM_PROMPT_ECHO_OFF() );

	push @res, PAM_SUCCESS();
	push @res, $ans;
	}
push @res, PAM_SUCCESS();
return @res;
}


