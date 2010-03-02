#!/usr/bin/perl

=head1 COPYRIGHT

This file is part of RemoteBOINC.

Copyright (C) 2010 Toni Giorgino, Universitat Pompeu Fabra

RemoteBOINC is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RemoteBOINC is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

=cut


# $Id: boinc_submit.pl 356 2010-03-02 15:00:31Z toni $

use strict;


use FindBin qw($Bin);		# where was script installed?
use lib $FindBin::Bin;		# use that dir for libs, too
use lib "$Bin/lib/$]";
use Error qw(:try);

use Getopt::Long qw(:config ignore_case pass_through);
use XML::Simple;
use HTTP::Request::Common qw(POST);
use HTTP::DAV;
use LWP::UserAgent;
use File::Basename;
use File::Temp qw(tempfile);

require qw(boinc_lib.pl);



# ----------------------------------------

# Constants 

use constant XMLROOT => "BoincRemote";

# The temporary description file
my $description_file_xml="description_file.xml";
my ($description_file_fh,$description_file)=
    tempfile("/tmp/description_temp.xml.XXXXX",
	     UNLINK=>1);


# Constant, relative to url
my $cgi_submit="boinc_submit_server.pl";
my $cgi_retrieve="boinc_retrieve_server.pl";




# Todo: replace config with longopts +
# http://www.perl.com/pub/a/2007/07/12/options-and-configuration.html?page=2

my $unparsed_cli="@ARGV";


# Keep these in the same order as help

my $url=$ENV{RBOINC_URL};
my $app_name='meta';
my $user=$ENV{USER};
my $email='';
my $authenticator='';

my $group='';
my $name='';

my $metadata_file='';
my $assign_user_all='';
my $num_steps=1;
my $priority=1;

my $fpops_est='';
my $fpops_bound='';
my $memory_bound='';
my $target_nresults=1;
my $balance=0;
my $min_quorum=1;
my $dry_run='';
my $batch='';

my $help='';
my $help_parameters='';
our $verbose='';



# Do the first pass at option parsing. 
# Keep these in the same order as help

# This list should be as short as possible because it is validated BEFORE the server-passed list.
# Any argument matched here will not be matched against the template-specific one.

GetOptions(
    'url=s' => \$url,
    'app_name=s' => \$app_name,
    'user=s' => \$user,
    'email=s' => \$email,
    'authenticator=s' => \$authenticator,

    'help' => \$help,
    'help_parameters' => \$help_parameters,
    'verbose' => \$verbose,
    ) or die "Error parsing command line";


if($help) {
    printHelp();
    exit(0);
}




############
# Option checking

checkMandatoryArguments(['url','app_name']) or exit 1;


# Ask the server for the template  and parse it
logInfo("Getting WU template");
my $wu_template=getWuTemplate("$url/$cgi_retrieve",$app_name);

logInfo("Parsing WU template");
my $parameters=parseWuTemplate($wu_template);

if($help_parameters) {
    printParametersHelp($wu_template);
    exit(0);
}






# Second pass at option parsing. This is tricky. We will use a version
# of getopts which requires two arguments:
#  a. a hash ref of ( optnameN => ref to destination )
#  b. a list of (optname1=s optname2=i ...) - note the =x part
# To roll-back, go to SVN version 318


#  1. create a hash ($parfiles) which will hold the variable options
#     and their content. It will be written through the references
#     created later
my $parfiles={};		


# 2. create a hash specifying the VARIABLE options and their
#    destination, in parfiles
my %getopt2_files_options=();

foreach my $opt (keys %$parameters) {
    $parfiles->{$opt}='';
    my $rr=\($parfiles->{$opt});
    $getopt2_files_options{$opt."=s"}=$rr;
}


# Now, we shall divide the options parsed at this stage in a
# "constant" group and a "variable" one, taken from the template

#  3. create a hash specifying the CONSTANT options and their
#     destinations (in plain variables)
my %getopt2_constant_options=(
	   'group=s' => \$group,
	   'name=s' => \$name,

	   'metadata_file=s' => \$metadata_file,
	   'assign_user_all=i' => \$assign_user_all,
	   'num_steps=i' => \$num_steps,
	   'priority=i' => \$priority,
	   
	   'fpops_est=f' => \$fpops_est,
	   'fpops_bound=f' => \$fpops_bound,
	   'memory_bound=f' => \$memory_bound,
	   'target_nresults=i' => \$target_nresults,
	   'balance=i' => \$balance,
	   'min_quorum=i' => \$min_quorum,
	   'dry_run' => \$dry_run,
	   'batch=i' => \$batch,
);


#  4. merge 2+3 into "all options"
my %getopt2_all_options=(%getopt2_files_options,%getopt2_constant_options);


#  5. use "copy_fix_options" on 2+3 to generate variables required at
#     points a+b above
my $temp_all_opts=copy_fix_options(\%getopt2_all_options);


Getopt::Long::Configure("no_pass_through"); # this time strict parsing
GetOptions($temp_all_opts, keys %getopt2_all_options) or die "Error parsing command line";

# End stage-2 option parsing. 




# Mandatory ones
checkMandatoryArguments(['name',
			 'group']) or exit 1;


# Now in parfiles we have the user-supplied arguments, minus the
# missing ones, as a hash. In $parameters we have the complete
# list. Check for missing arguments and unreadable files.

foreach my $o (keys %$parameters) {
    my $ov=$parfiles->{$o};
    if($ov) {
	logInfo("Option -$o has value $ov");
	if(! -r $ov) {
	    die "Can't open file $ov";
	}
    } else {
	logInfo("Option -$o is empty");
	if($parameters->{$o}->{optional}) {
	    logInfo("...which is allowed, server will supply a default");
	} else {
	    print STDERR "Mandatory file for parameter -$o is missing.\n";
	    exit 1;
	}
    }
}



############
# Build up list of files to upload. Currently this is
# "$description_file.xml", created above, plus those referenced by the
# keys of the %parameters hash.

# Keys is the remote file, value the local one.
# We already have this list.

my %toupload=%$parfiles;

$toupload{$description_file_xml}=$description_file;




############
# Make a random number as an upload id
my $rid=sprintf('up%06d',int(rand(999999)));



############
# Build the description list

my $oh={};

$oh->{RandomId}=$rid;
$oh->{UnparsedCommandLine}={content => $unparsed_cli};
$oh->{UploadedFiles}={file => [values %toupload] };
$oh->{TimeStamp}={Unix   => time,
		  String => scalar localtime};
$oh->{ClientRevision}='$Revision: 356 $';
$oh->{LoginName}=$user;

$oh->{Email}=$email;
$oh->{Group}=$group;
$oh->{Name}=$name;

$oh->{Template}=$app_name;
$oh->{AssignUserAll}=$assign_user_all;
$oh->{NumSteps}=$num_steps;
$oh->{Priority}=$priority;
$oh->{FPopsEst}=$fpops_est;
$oh->{FPopsBound}=$fpops_bound;
$oh->{MemoryBound}=$memory_bound;

$oh->{TargetNResults}=$target_nresults;
$oh->{Balance}=$balance;
$oh->{MinQuorum}=$min_quorum;
$oh->{Batch}=$batch;

$oh->{DryRun}=$dry_run;



my $desc= XMLout($oh, RootName => XMLROOT, AttrIndent => 1 );
print $description_file_fh $desc;
close $description_file_fh;

logInfo("Description file:\n$desc\n",1);





##############
# Make the DAV connection

logInfo("Requesting the  DAV address");
my $dav_url=getDavUrl("$url/$cgi_retrieve");

logInfo("Connecting to DAV server");
my $dav = new HTTP::DAV;
# optionally: set credentials $dav->credentials
$dav->open( -url=> $dav_url )
    or die("Couldn’t open $dav_url: " .$dav->message . "\n");

# optionally: lock

# Create a directory and cd in it
$dav->mkcol($rid);
$dav->cwd($rid);




############
# Upload  files

logInfo("Uploading files in dir $rid.");

foreach my $upfile (keys %toupload) {
    $dav->put(-local=>$toupload{$upfile},
	      -url=>$upfile);
    # add error checking
}




############
# Remove the description file - not necessary: it is handled by UNLINK and tempfile()

# unlink $description_file or die "Error removing temporary description file: $!";




############
# Call the remote endpoint

logInfo("Invoking remote CGI");

my $ua = new LWP::UserAgent; 
my $cgi_url = "$url/$cgi_submit";
my $response = $ua->post( $cgi_url,
			  { random_id => $rid,
			    # more parameters here if needed
			  });
# error will be checked after deleting the temporary directory
my $content = $response->content; 
logInfo("Response received:\n$content",1);


############
# Delete uploaded dir

logInfo("Deleting temporary DAV directory");

$dav->cwd("..");
$dav->delete($rid);


############
# Die if the response was an error

if(! $response->is_success) {
    my $stat= $response->status_line;
    print STDERR "The remote web server returned an error: $stat\n\n";
    exit 1;
}


############
# Parse submission status

my $server_xml=XMLin($content);

if($server_xml->{Failure}) {
    print STDERR "The server CGI returned an error. Reason: $server_xml->{Failure}->{Reason}\n";
    exit 1;
} 



############
# Print submission status

my $server_id=$server_xml->{Identifier};
my $server_exitcode=$server_xml->{Return}->{ExitCode};
my $server_stdout=$server_xml->{Return}->{StdOut};
my $server_stderr=$server_xml->{Return}->{StdErr};

print <<EOF;


Submission complete.

Stdout
------
$server_stdout

Stderr
------
$server_stderr
EOF

if($server_exitcode) {
	print "\n\nDetailed error information follows.\n";
	print XMLout($server_xml->{Return}, 
		     RootName => "Return",
		     AttrIndent => 1 );
}


exit $server_exitcode;




# ==================================================
# Auxiliary functions


# Check if the calling environment has all the given variables
# defined.  If not, print one of them. Else, return false. These are
# passed as string in order to be able to be able to print their name.
# Sadly, must be duplicated because otherwise does not have access to
# scope.

sub checkMandatoryArguments {
    my $l=shift;
    foreach my $f (@$l) {
	if(! eval("defined \$$f")) {
	    print STDERR "Missing mandatory argument: $f. See -help.\n";
	    return 0;
	}
    }
    return 1;
}



# do this: { a=s => x }  -->  { a => x }

sub copy_fix_options {
    my $q=shift;
    my $r={};
    foreach my $k (keys %$q) {
	my ($oname,$otype)=split(/=/,$k);
	$r->{$oname}=$q->{$k};
    }
    return $r;
}



sub printParametersHelp {
    my $t=shift;

    my $p=parseWuTemplate($t);

    print "\n";
    print "Remote application queue `$app_name'\n";
    print "Description: $t->{WuTemplate}->{rboinc}->{description}\n";
    print "Application on server: `$t->{WuTemplate}->{rboinc}->{application}'\n";
    print "\n";
    print "Options defined for this application queue:\n";
    foreach my $pn (sort keys %$p) {
	print "    -$pn\t\t";
	if($p->{$pn}->{optional}) {
	    print "(optional) ";
	}
	print $p->{$pn}->{description}."\t";
	print "\n";
    }
    print "\n";
}




# ==================================================


# for sending requests: http://snippets.dzone.com/posts/show/3163


sub printHelp {
    print <<EOF;
Boinc remote submission. Usage: boinc_submit [OPTION]...

**  SEE -help_parameters FOR SIMULATION PARAMETERS  **

Authentication:
    -url       URL       RBoinc URL contact point (*)
    -app_name  APP       Remote application     [$app_name]
    -user      NAME      Override username      [$user]
    -email     ADDRESS   (Not implemented)
    -authenticator ID    (Not implemented)

Simulation identifier (mandatory):
    -group     ID        The simulation group 
    -name      ID        The individual job ID

Scheduling:
    -metadata_file FILE  Additional metadata file
    -assign_user_all ID	 Assign to user		[$assign_user_all]
    -num_steps   NUM     Number of steps to run [$num_steps]
    -priority    NUM     BOINC priority level   [$priority]

Resources:
    -fpops_est   FLOAT   Estimated FP count     [$fpops_est]
    -fpops_bound FLOAT   Max FP count           [$fpops_bound]
    -memory_bound FLOAT  Memory requirement     [$memory_bound]
    -target_nresults NUM Redundancy             [$target_nresults]
    -balance     ID      Load balancing alg. (on=1)
    -min_quorum  NUM     (Not implemented)
    -dry_run             Upload, but not start WUs
    -batch       NUM     Batch number           [$batch]

Miscellaneous:
    -help_parameters     Show list of parameters, ie files to be
                         transferred (depending on the application)
    -help                This message
    -verbose             Be verbose

(*) You can also use the RBOINC_URL environment variable.
    For example: http://www.ps3grid.net:8383/rboinc_cgi

EOF
}




