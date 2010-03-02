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


# $Id: boinc_retrieve.pl 356 2010-03-02 15:00:31Z toni $

use strict;


use FindBin qw($Bin);		# where was script installed?
use lib $FindBin::Bin;		# use that dir for libs, too
use lib "$Bin/lib/$]";

use Getopt::Long qw(:config ignore_case auto_help);
use XML::Simple;
use HTTP::Request::Common qw(POST);
use HTTP::DAV;
use LWP::UserAgent;
use File::Basename;
use Pod::Usage;
use Error qw(:try);

use constant XMLROOT => "BoincRemote";

require qw(boinc_lib.pl);



# Constant, relative to url
my $cgi_retrieve="boinc_retrieve_server.pl";



# ----------------
# Parsing command line

my $group='';

my $name='';
my $into='',
our $verbose='';
my $quiet='';
my $keep='';
my $purge='';
my $stop='';
my $status='';
my $gridstatus='';
my $help='';

my $url=$ENV{RBOINC_URL};
my $user=$ENV{USER};
my $email='';
my $authenticator='';



GetOptions(
    'group=s' => \$group,

    'name=s' => \$name,
    'into=s' => \$into,
    'verbose' => \$verbose,
    'quiet' => \$quiet,
    'keep'  => \$keep,
    'purge' => \$purge,
    'stop' => \$stop,
    'status' => \$status,
    'gridstatus' => \$gridstatus,
    'help' => \$help,

    'url=s' => \$url,
    'user=s' => \$user,
    'email=s' => \$email,
    'authenticator=s' => \$authenticator,
    ) or die "Error parsing command line";


pod2usage(1) if $help;



# ----------------
# Check arguments

checkMandatoryArguments(["group","url"]) or exit 1;
my $cgi_url = "$url/$cgi_retrieve";




# ----------------
# Authentication TODO





# ----------------
# Remote action invocation

if($purge) {
    handlePurge();
} elsif($stop) {
    handleStop();
} elsif($status) {
    handleStatus();
} elsif($gridstatus) {
    handleGridStatus();
} else {
    handleRetrieve();
}

exit(0);










########################################
# Handle purge action


sub handlePurge {

    if($into)  {
	print STDERR "Action --purge deletes only. It makes no sense in combination with --into.\n";
	exit 1;
    }

    confirmOrDie("The operation will IRREVERSIBLY delete results from the server.\n".
		 "Note: You won't be able to submit new WUs with equal names until old ones\nwill be pending in the server (check with -gridstatus).\nConfirm? ");
    my $xmlcontent=invokeRMI({action=>'purge',group=>$group,name=>$name});
    my $message=$xmlcontent->{Success}->{Message};
    print "Success. Message from server: $message\n";
}




########################################
# Handle stop action


sub handleStop {

    if($into || $name) {
	print STDERR "Action --stop  makes no sense in combination with --into or --name.\n";
    }

    confirmOrDie("The operation will IRREVERSIBLY stop the WU. Results can still be retrieved. Confirm? ");
    my $xmlcontent=invokeRMI({action=>'stop',group=>$group});
    my $message=$xmlcontent->{Success}->{Message};
    print "Success. Message from server: $message\n";
}




########################################
# Handle status action


sub handleStatus {

    if($into || $name) {
	print STDERR "Action --status  makes no sense in combination with --into or --name.\n";
    }

    my $xmlcontent=invokeRMI({action=>'status',group=>$group});
    my $message=$xmlcontent->{Success}->{Message};
    print "Success. Message from server: $message\n";

    my $steps=$xmlcontent->{Success}->{StepList};
    my %st=%$steps;
    foreach my $n (keys %st) {
	my $nn=$n;		# strip "bin_"
	$nn=~s/^Bin_//;
	print "$nn\t".$st{$n}."\n";
    }

}



########################################
# Handle status action


sub handleGridStatus {

    if($into || $name) {
	print STDERR "Action --gridstatus  makes no sense in combination with --into or --name.\n";
    }

    my $xmlcontent=invokeRMI({action=>'gridstatus'});
    my $message=$xmlcontent->{Success}->{Message};
    print "Success. Message from server: $message\n";

    my $list=$xmlcontent->{Success}->{content};
    print "$list\n";

}







########################################
# Handle retrieve action


sub handleRetrieve {

# ----------------
# Change dir and fail early
    if($into) {
	chdir $into or do {
	    print STDERR "Cannot chdir to -into directory `$into': $!\n";
	    exit 1;
	}
    }

# ----------------
# Invoke RMI
    my $xmlcontent=invokeRMI({action=>'retrieve',group=>$group,name=>$name});

# ----------------
# Check outcome
    my $rfilelist=$xmlcontent->{FileList}->{File};
    if(!$rfilelist) {
	die "No files ready for retrieval.\n";
    } 

    my $dav_dir=$xmlcontent->{Success}->{Directory};
    if(!$dav_dir) {
	die "Error requesting download location";
    }

    my $aliasTable=$xmlcontent->{AliasTable};
    my $finalOutputs=$xmlcontent->{Success}->{FinalOutputs};
    my $nMeta=$xmlcontent->{Success}->{MetadataFileCount};


# ----------------
# Download
    logInfo("Requesting the  DAV address");
    my $dav_url=getDavUrl($cgi_url);

    logInfo("Connecting to DAV server");
    my $dav = new HTTP::DAV;
    $dav->open( -url=> $dav_url )
	or die("Couldn’t open $dav_url: " .$dav->message . "\n");
    $dav->cwd($dav_dir)              
	or die("Couldn’t set remote directory $dav_dir: " .$dav->message . "\n");

    my $ndone=0;
    my $nskip=0;
    my @skiplist=();
    my $nexpected=scalar @$rfilelist;
    foreach my $fn (@$rfilelist) {
	if( fileOrAliasExists($fn,$aliasTable) ) {
	    if(!$quiet && $nskip==0) {
		print "Warning: some files are present locally and will not be overwritten.\n";
	    }
	    $nskip++;
	    push @skiplist,$fn;
	} else {
	    $dav->get(-url => $fn,
		      -to => ".") and
			  $ndone++;
	}
	if(!$quiet) {
	    local $|=1;
	    print sprintf("Retrieved $ndone, already present $nskip, out of $nexpected (% 3d%%)\r",100.*($ndone+$nskip)/$nexpected);
	}
    }

    if($verbose) {
	print "The following files were not overwritten: @skiplist\n";
    }

    print "Successfully retrieved $ndone, already present $nskip, out of $nexpected ($nMeta metadata).\n";



# ----------------
# Request deletion of remote files

    if(! $keep) {
	logInfo("Going to remove retrieved files");
	try {
	    $xmlcontent=invokeRMI({action=>'remove',dir=>$dav_dir});

	    my $nremoved=$xmlcontent->{Success}->{NumberRemoved};
	    my $nkept=   $xmlcontent->{Success}->{NumberKept} ;	 
	    
	    if(!$quiet) {
		print "Removed $nremoved server files, $nkept were too recent to remove.\n";
	    }
	} catch Error with {
	    my $ex=shift;
	    my $mess=$ex->text();
    	    print STDERR "Error requesting removal of remote files: $mess; continuing.\n";
	}


    }




# ----------------
# Finalize

    logInfo("Removing retrieval directory");
    foreach my $fn (@$rfilelist) {
	$dav->delete(-url => $fn);
    }
    $dav->cwd("..");
    $dav->delete($dav_dir);

}




# Check if file exists in current directory (1st arg), checking both
# the literal file name and its aliases, appending extensions
# described in the alias list (2nd arg)

sub fileOrAliasExists {
    my $fn=shift;
    my $at=shift;

    logInfo("Testing existence of $fn...");
    if(-e $fn) {
	logInfo("...is there");
	return 1;
    }

    # Extract trailing number
    $fn=~/_([0-9]+)$/;
    my $fileExt=$1;

    # iterate over the alias list
    my $rAliasList=$at->{File};
    foreach my $curAlias ( @$rAliasList ) {
	if($curAlias->{Extension} eq "_$fileExt") {
	    # if alias for current extension, check appending all extensions
	    my $rExtList=$curAlias->{Alias};
	    foreach my $extToTest (@$rExtList) {
		logInfo("Testing existence of $fn + $extToTest...");
		if(-e $fn.$extToTest) {
		    logInfo("...is there");
		    return 1;
		}
	    }
	}
    }

    return 0;

}







# ########################################
# Misc. utility functions, shared by all handlers



# ----------------
# Ask for positive confirmation, or throw exception.

sub confirmOrDie {
    my $answer=promptUser(shift,"N");
    die "Operation aborted." if($answer !~ /^[yY]/ );
}




# ----------------
# Performs the remote method invokation, die-ing on failure
# Returns a parsed XML structure or throws an exception.
# Will use the $user global variable

sub invokeRMI {
    my $params=shift;

    logInfo("Invoking CGI");

    $params->{loginname}=$user;

    my $xmlcontent;
    my $ua = new LWP::UserAgent;
    my $response = $ua->post( $cgi_url,
			      $params  );
    if($response->is_success) {
	my $content = $response->content; 
	if($verbose) {
	    print "Response received:\n";
	    print $content;
	}
	$xmlcontent=XMLin($content,ForceArray=>["File","Alias"]);
    } else {
	my $reason=$response->status_line;
	die "Error in POST from remote: $reason. Server may be down.\n";
    }

    if($xmlcontent->{Failure}) {
	die "Server error message: $xmlcontent->{Failure}->{Reason}";
    } elsif(!$xmlcontent->{Success}) {
	die "Undefined state returned. This is a bug.";
    } 
	
    return($xmlcontent);
}









# Check if the calling environment has all the given variables
# defined.  If not, print one of them. Else, return false. These are
# passed as string in order to be able to be able to print their name.
# Sadly, must be duplicated because otherwise does not have access to
# scope.

sub checkMandatoryArguments {
    my $l=shift;
    foreach my $f (@$l) {
	if(! eval('$'."$f")) {
	    print STDERR "Missing mandatory argument: $f. See -help.\n";
	    return 0;
	}
    }
    return 1;
}






__END__

=pod
=head1 NAME

boinc_retrieve - Retrieving and administering remote boinc jobs


=head1 SYNOPSIS

boinc_retrieve [options]


=head1 OPTIONS

=head2 Mandatory parameters

=begin text

    -group GROUP         The simulation group to be retrieved

=end text

=head2 Modifiers

=begin text

    -name NAME           Retrieve only a specific job and its metadata
    -into DIR            Put files into specified directory (default ".")
    -verbose             Be verbose
    -quiet               Hide download progress indicator
    -keep                Do not remove retrieved files from server 
    -purge               Completely remove GROUP from server (if finished)
    -stop                Prevent more work to be spawned for a given group
    -status              Show step numbers for given group
    -gridstatus          Show resources consumed and statuses for all groups
    -help                This message

=end text

=head2 Authentication

=begin text

    -url URL             RBoinc URL contact point (*)
    -user NAME           Override username [$user]
    -email ADDRESS       (Not implemented)
    -authenticator ID    (Not implemented)

    (*) You can also use the RBOINC_URL environment variable
	For example: http://www.ps3grid.net:8383/rboinc_cgi

=end text



=head1 SEE ALSO

L<boinc_submit>


=head1 AUTHOR

Toni Giorgino

=cut

