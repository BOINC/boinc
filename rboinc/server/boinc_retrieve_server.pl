#!/usr/bin/perl -w

# $Id: boinc_retrieve_server.pl 354 2010-03-02 14:56:33Z toni $

# The remote-boinc server-side perl script. Should run as a CGI in an
# Apache2 instance.

=head1 NAME

boinc_submit_server.pl - CGI for handling incoming remote
    boinc submissions

=head1 SYNOPSIS

To be installed in Apache's cgi-bin directory

=head1 DESCRIPTION

This documentation is provisional.

Currently receives GET requests with two arguments, "action" and
"group".  The script authenticates the user, goes in the workflow
directory, checks if the user owns the dir named "group". If so,
generates a random sting, creates a dir with the said name in the DAV
area, and therein creates links to all the files which can be
downloaded. The CGI returns the random id name.

=head1 AUTHOR

Toni Giorgino

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


# ----------------

# Prepend non-standard location for modules
use lib qw(/home/boinc/toni/perl/lib);

# Prepend location for support modules
#use lib qw(/home/toni/work/RemoteBoincProject/server);


# ----------------


use strict;
use CGI::Lite;			# parsing requests
use CGI::Carp;  		# for error messages
use XML::Simple;		# to process xml easily
use File::Copy;
use File::Path;
use File::Basename;
use Cwd;
use Error qw(:try);

require qw(boinc_authentication.pl);



# ----------------
# System-derived variables and constants

my $response_header="Content-type: text/plain\n\n";
my $cgidir=dirname($ENV{SCRIPT_FILENAME});
my $xmlroot = "RetrieveStatus";
my $xml_description_file="description_file.xml";


# ----------------
# Provisional variables: will soon be made global. Underscores are
# important because a user could otherwise submit a job named `pool'

my $pool_dirname="pool";
my $delete_dirname="rboinc_trash_dir";
my $process_stop="force-process-stop";
my $process_vars="process.vars";
my $safepurge=0;



# ----------------
# Provisional variables: will soon be replaced by true variables
# passed by the client via the description mechanism

my $temporary_boinc_email='nobody@nil.es';


# -----------------
# Set configuration variables

carp "------------------------------------------------------";

our $config=do "$cgidir/boinc_configuration.pl";

if(!$config) {
    die "Can't open configuration: $!";
}


my $remove_min_age=$config->{REMOVE_MIN_AGE} || 1.0;



# -----------------
# Begin the actual program




# Prepare the CGI machinery for parsing input

my $cgi = new CGI::Lite;
$cgi->add_timestamp(0);
my $action;

# Receive the call and get the arguments
do { #  ???
    try {
	my $form = $cgi->parse_form_data();
	die ("Error receiving data: ".$cgi->get_error_message) 
	    if $cgi->is_error;

	$action=$form->{action};

	if(! defined $action || $action eq '') {
	    # Simple answer if no random-id provided (direct call)
	    voidAnswer();
	} elsif ($action eq 'get_dav_url') {
	    handleGetDavUrl();
	} elsif ($action eq 'get_wu_template') {
	    handleGetWuTemplate($form->{application});
	} elsif ($action eq 'retrieve') {
	    my $form_group=$form->{group};
	    my $form_name=$form->{name};
	    my $form_loginname=$form->{loginname};
	    my $userpfx=(uc "$form_loginname")."_";
	    handleRetrieve($userpfx.$form_group,$form_name);
	} elsif ($action eq 'remove') {
	    handleRemove($form->{dir});
	} elsif ($action eq 'purge') {
	    my $form_group=$form->{group};
	    my $form_loginname=$form->{loginname};
	    my $userpfx=(uc "$form_loginname")."_";
	    handlePurge($userpfx.$form_group,$form->{name});
	} elsif ($action eq 'stop') {
	    my $form_group=$form->{group};
	    my $form_loginname=$form->{loginname};
	    my $userpfx=(uc "$form_loginname")."_";
	    handleStop($userpfx.$form_group);
	} elsif ($action eq 'status') {
	    my $form_group=$form->{group};
	    my $form_loginname=$form->{loginname};
	    my $userpfx=(uc "$form_loginname")."_";
	    handleStatus($userpfx.$form_group);
	} elsif ($action eq 'gridstatus') {
	    my $form_loginname=$form->{loginname};
	    handleGridStatus($form_loginname);
	} else {
	    returnFailure($xmlroot,"Unsupported action $action");
	}

    } catch Error with {
	my $ex=shift;
	my $mess=$ex->text();
	carp "Exception: ($action error) $mess";
	returnFailure($xmlroot,"($action error) $mess");
    }
};

exit(0);







##################################################

sub handleGetDavUrl {
    my $oh={};
    $oh->{DavUrl}=$config->{DAV_URL};
    $oh->{ServerRevision}='$Revision: 354 $';

    my $xr=XMLout($oh, RootName => $xmlroot, AttrIndent => 1);

    print $response_header;
    print $xr;
}




##################################################

sub handleGetWuTemplate {
    my $app=shift;
    my $thash=parse_wu_template($app);

    my $oh={};
    $oh->{WuTemplate}=$thash;
    $oh->{ServerRevision}='$Revision: 354 $';

    my $xr=XMLout($oh, RootName => $xmlroot, AttrIndent => 1);

    print $response_header;
    print $xr;
}






##################################################
# Prevent the WU from spawning more work

sub handleStop {
    my $group=shift;

    my $wd=$config->{WORKFLOW_DIR};
    my $groupdir="$wd/$group";

    die "invalid tag supplied" if(! isTagValid($group)  );
    checkPoolDir($groupdir);
    chdir($groupdir);
    
    die "Already stopped" if(-e $process_stop);
    system("touch $process_stop");
    sendSuccess("Group successfully marked as stopped, can be purged after assimilation.");

}



##################################################
# Remove the WU and its administrative files.

sub handlePurge {
    my $group=shift;
    my $name=shift;

    my $owd=cwd;
    my $wd=$config->{WORKFLOW_DIR};
    my $groupdir="$wd/$group";
    my $trashdir="$wd/$delete_dirname";
    die "invalid tag supplied" if(! isTagValid($group) ||
				  ($name && isNameReserved($name)) ||
				  ($name && !isTagValid($name) ) 
	);
    checkPoolDir($groupdir);

    my $size='';
    if(!$name) {
	# Purge whole GROUP
	chdir($groupdir);
	if($safepurge) {
	    my @inprog=glob("*/in_progress");
	    die "there are jobs in progress: @inprog" if(@inprog);
	}
	chdir($wd);
	$size=`du -h -s $group`;
	safeRemove($groupdir)  or die "move failed with $!";

    } else {
	# Purge GROUP/NAME
	die "job $name in group $group does not exist" 
	    if(! -d "$groupdir/$name");
	die "job $name in group $group still in progress" 
	    if(-e "$groupdir/$name/in_progress" && $safepurge);

	chdir($groupdir);
	$size=`du -h -s $name`;
	safeRemove($name)  or die "move failed with $!";

    }
    sendSuccess("Freed $size on server");

}







##################################################

# Remove the files from the server. This means that links in DAV will
# become broken. In this case, we also have to remove them from DAV,
# because apache DAV won't remove broken symlinks. Metadata files are
# never removed.

sub handleRemove {
    my $dir=shift;

    my $retrdir=$config->{DAV_DIR}."/$dir/";

    # List of symlinks to original files
    my @llist=glob("$retrdir/*");

    my $nremoved=0;
    my $nkept=0;
    my $keepalive_every=20;

    print $response_header;
    foreach my $f (@llist) {
	next if($f =~ /metadata_file/);
	print "\n" if( ($nremoved+$nkept) % $keepalive_every == 0);

	my $ldest=readlink($f);
	if(canRemove($ldest) ) {
	    safeRemove($ldest);
	    unlink($f);		# remove now-broken symlink
	    $nremoved++;
	} else {
	    $nkept++;		# the client will remove it
	}
    }
    sendRemoveSuccess($nremoved,$nkept);
}



# Return true if conditions are met for the given file name to be
# removed
sub canRemove {
    my $fn=shift;
    my $removable=laterStepsExist($fn) || 
	isLastStep($fn);

    return($removable);
}


# True if given file name is newer than $remove_min_age
sub isOldAge {
    my $fn=shift;
    my $age=(-M $fn);		# days
    return ($age > $remove_min_age);
}



# True if given file name is at its latest step
sub isLastStep {
    my $fn=shift;
    my $rc=0;
    if (stepOf($fn)==maxStepOf($fn)-1) {
	$rc=1;
    }
    return $rc;
}





# Return true if there are more recent steps in the series of
# the given file name
sub laterStepsExist {
    my $fn=shift;
    my $s=stepOf($fn);
    my $gl=mkStepGlob($fn);
    my @ex=glob($gl);		# list of files in the same dependency
				# series as the removee
    my $canremove=0;
    
    foreach my $i (@ex) {
	if(stepOf($i)>$s) {
	    $canremove=1;
	    last;
	}
    }
    
    return $canremove;
}



# Make a glob to match all the results of this step.
# GPUGRID-specific.
# Ex.  /path/N2-TONI_TONIR6-5-10-RND7389_0 -> /path/N2-TONI_TONIR6-*-10-RND7389_0
sub mkStepGlob {
    my $fn=shift;
    my $path=dirname($fn);
    my @p=split(/-/,basename($fn));
    $p[2]="*";
    my $gl=join '-',@p;
    return "$path/$gl";
}


    


# This one does not send the header, because it must 
# precede the keepalives.
sub sendRemoveSuccess {
    my $nr=shift;
    my $nk=shift;

    my $r={};
    $r->{Success}->{NumberRemoved}=$nr;
    $r->{Success}->{NumberKept}=$nk;
    $r->{ServerRevision}='$Revision: 354 $';

    my $xr=XMLout($r, RootName => $xmlroot, AttrIndent => 1);

    print $xr;
}





##################################################



sub handleRetrieve {
    my $group=shift;
    my $name=shift;
    
# Check tag validity
    my $fail='';
    if(! isTagValid($group)) {
	$fail="Invalid group supplied";
    } elsif($name && isNameReserved($name)) {
	$fail="$name is a reserved name";
    } elsif($name && !isTagValid($name)) {
	$fail="Invalid name supplied";
    }

    die("$fail") if($fail);
    

# -----------------
# Authenticate, TODO



# -----------------
# Check if the TAG dir exists (dies on failure)
# TODO fix expected output counts
# http://www.pageresource.com/cgirec/ptut18.htm

    my $groupdir=$config->{WORKFLOW_DIR}."/$group";

    # disabled machinery for computing expected outputs
    my $expouts=-1;
    my $metadata=0;

    # will die on error
    checkPoolDir($groupdir);




# -----------------
# Make a random id and corresponding dl dir

    my $random_id=sprintf('retr%06d',int(rand(999999)));
    my $retrdir=$config->{DAV_DIR} . "/$random_id/";

    mkdir $retrdir,0777 or die "Can't make retrieve dir: $!";



# -----------------
# TODO: find files to be downloaded

    my @flist;
    if($name) {
	@flist=glob("$groupdir/$name-$group-*");
	if(-r "$groupdir/$name/metadata_file") {
	    push @flist,"$groupdir/$name/metadata_file";
	    $metadata++;
	}
    } else {
	@flist=glob("$groupdir/*-$group-*");
    }

    if(! scalar @flist) {
	rmdir($retrdir);
	returnFailure($xmlroot,"No files ready for retrieval");
	return;
    }

    my @blist=();			# basenames
    foreach my $f (@flist) {
	my $bn=basename($f);
	push @blist,$bn;
	symlink $f,"$retrdir/$bn";
    }


    sendRetrieveSuccess($random_id,\@blist,$expouts,$metadata);

}


sub sendRetrieveSuccess {
    my $reason=shift;
    my $rblist=shift;
    my $eo=shift;
    my $meta=shift;

    my $r={};
    $r->{Success}->{Directory}=$reason;
    $r->{Success}->{FinalOutputs}=$eo;
    $r->{Success}->{MetadataFileCount}=$meta;
    $r->{AliasTable}={File=>$config->{ALIAS_TABLE}};
    $r->{FileList}={File=>$rblist};
    $r->{ServerRevision}='$Revision: 354 $';

    my $xr=XMLout($r, RootName => $xmlroot, AttrIndent => 1);

    print $response_header;
    print $xr;
}









##################################################


sub handleStatus {
    my $group=shift;		# the group name
    my $r=currentSteps($group);

    die "invalid tag supplied" if(! isTagValid($group));

    sendSuccess("Returning count of max-steps computed",{Success=>{StepList=>$r}});

}



# Return the list of bins in this group (based on those who
# have a process.vars)

sub currentNamesInGroup {
    my $group=shift;		# the group name
    my $gd=$config->{WORKFLOW_DIR}."/$group";
    checkPoolDir($gd);

    my @plist=glob("$gd/*/$process_vars");
    my @nlist=();
    # for each process_vars, take the last element of the path
    foreach my $i (@plist) {
	push @nlist,basename(dirname($i));
    }
    return @nlist;
}


# existence of step 0 returned as 1
sub maxStepNameGroup {
    my $group=shift;
    my $name=shift;
    my $gd=$config->{WORKFLOW_DIR}."/$group";

    my $gl="$gd/$name-$group-*-*-*_0";
    my @l=glob($gl);

    my $mx=0;
    foreach my $i (@l) {
	my $n=stepOf($i)+1;
	if($n>$mx) {
	    $mx=$n;
	}
    }
    return $mx;

}
    
    
# Using "bin_" to workaround attribute name limitation

sub currentSteps {
    my $group=shift;
    my @nl=currentNamesInGroup($group);
    my $r={};
    foreach my $i (@nl) {
	$r->{"Bin_$i"}=maxStepNameGroup($group,$i);
    }
    return $r;
}




##################################################
# Misc utility functions


sub handleGridStatus {
    my $user=shift;		# the group name

    die "invalid user supplied" if(! isUserValid($user));

    my $cmd=<<"EOL";
echo "call mon_status('$user')" | mysql -t -p XXX YYY
EOL
    my $list=`$cmd`;

    sendSuccess("Returning grid status",{Success=>{content=>$list}});

}


##################################################
# Misc utility functions


# Assert that the TAG dir exists (dies on failure). Will return a
# void.

sub checkPoolDir {
    my $wd=shift;
    my $res=opendir(WD, $wd);
    closedir WD;

    # No special cleanup required
    die("Group does not exist")    if(!$res); 
    
    my $pfn=$wd."/$pool_dirname";
    die("Not a remotely-submitted job") if(! -d $pfn );
}



# Extract the step num from a given result filename
# GPUGRID-specific.
# Ex.  /path/N2-TONI_TONIR6-5-10-RND7389_0 -> 5
sub stepOf {
    my $fn=basename(shift);
    my @p=split(/-/,$fn);
    return $p[2];
}


# /path/N2-TONI_TONIR6-5-10-RND7389_0 -> 10
sub maxStepOf {
    my @p=split(/-/,basename(shift));
    return $p[3];
}




# Remove a dir or file, moving it into the trash dir (renamed with an
# unique suffix).  I use "system mv" because move() has quirks on
# handling directory moves. E.g.  move("/dir1/dir2/dir3","/dir4")
# fails unless "dir3" eq "dir4". Should be changed in a two-step
# move()

sub safeRemove {
    my $fn=shift;

    my $wd=$config->{WORKFLOW_DIR};
    my $t=time();		# TODO improve
    my $trashdir="$wd/$delete_dirname/$t";

    mkpath($trashdir);

    system("/bin/mv $fn $trashdir") == 0
	or die "Error moving: $!";
}




# Generic "success" function - mandatory message, optional
# hash of additional stuff to be transferred

sub sendSuccess {
    my $m=shift;
    my $r=shift || {};
    $r->{Success}->{Message}=$m;
    $r->{ServerRevision}='$Revision: 354 $';

    my $xr=XMLout($r, RootName => $xmlroot, AttrIndent => 1);

    print $response_header;
    print $xr;
}

