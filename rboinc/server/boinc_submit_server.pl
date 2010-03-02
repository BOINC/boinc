#!/usr/bin/perl -w

# $Id: boinc_submit_server.pl 353 2010-03-02 14:56:21Z toni $

# The remote-boinc server-side perl script. Should run as a CGI in an
# Apache2 instance.

=head1 NAME

boinc_submit_server.pl - CGI for handling incoming remote
    boinc submissions

=head1 SYNOPSIS

To be installed in Apache's cgi-bin directory

=head1 DESCRIPTION

This documentation is provisional.

Currently receives GET requests with one argument "random_id",
which is the directory inside a Dav-enabled rendez-vous point with the
client. The client is supposed to create a directory named "random_id"
in the Dav dir, upload relevant files there, and invoke this cgi with
the random_id argument.

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


# ----------------


use strict;
use CGI::Lite;			# parsing requests
use CGI::Carp;  		# for error messages
use XML::Simple;		# to process xml easily
use File::Copy;
use File::Basename;
use POSIX qw(uname);
use Error qw(:try);


require qw(boinc_authentication.pl);



# ----------------
# System-derived variables and constants

my $response_header="Content-type: text/plain\n\n";
my $cgidir=dirname($ENV{SCRIPT_FILENAME});
my $xmlroot = "SubmitStatus";
my $arch=(POSIX::uname())[4];
my $xml_description_file="description_file.xml";
my $reread_db="reread_db";


# ----------------
# Provisional variables: will soon be made global. Underscores are
# important because a user could otherwise submit a job named `pool'

my $pool_dirname="pool";            # underscore is important


# -----------------
# Set configuration variables

carp "------------------------------------------------------";

our $config=do "$cgidir/boinc_configuration.pl";

if(!$config) {
    die "Can't open configuration: $!";
}


# -----------------

# Begin the actual program




# Prepare the CGI machinery for parsing input

my $cgi = new CGI::Lite;

# Some of them may be redundand
# $cgi->set_platform("Unix");
# $cgi->set_file_type("file");
# $cgi->set_directory("/tmp");	# should we  want to POST files, in the future
$cgi->add_timestamp(0);    


# Receive the call and get the arguments
my $form = $cgi->parse_form_data();
die "Error receiving data: ".$cgi->get_error_message if $cgi->is_error;

my $random_id=$form->{random_id};


# Simple answer if no random-id provided (direct call)
if(!$random_id) {
    voidAnswer();
    exit 0;
}


# The DAV upload dir
my $updir=$config->{DAV_DIR} . "/$random_id/";

# Get the client address
my $client_ip=$ENV{REMOTE_ADDR};




# Read and parse the XML description
my $xml_description=XMLin("$updir/$xml_description_file");

# Parameters with no defaults
my $tag=$xml_description->{Tag};
my $group=$xml_description->{Group};
my $name=$xml_description->{Name};

my $timestamp_string=$xml_description->{TimeStamp}->{String};
my $login_name=$xml_description->{LoginName};


my $template=$xml_description->{Template} ||  $config->{DEFAULT_APP_NAME};
my $assign_user_all=$xml_description->{AssignUserAll}; # May be empty
my $num_steps=$xml_description->{NumSteps} || $config->{DEFAULT_NUMSTEPS};
my $priority=$xml_description->{Priority} ||  $config->{DEFAULT_PRIORITY};
my $fpops_est=$xml_description->{FPopsEst}; # May be empty
my $fpops_bound=$xml_description->{FPopsBound};  # May be empty
my $memory_bound=$xml_description->{MemoryBound}; # May be empty

my $target_nresults=$xml_description->{TargetNResults}; # May be empty
my $balance=$xml_description->{Balance} || 0;		   # FIXME
my $min_quorum=$xml_description->{MinQuorum}; # May be empty
my $batch=$xml_description->{Batch}; # May be empty


my $dry_run=$xml_description->{DryRun};

my $delay_bound=$xml_description->{DelayBound};  # May be empty



# Parse WU template

my $wu_template="rboinc_$template"."_wu";
my $wu_template_hash=parse_wu_template($template);
my $input_files=build_input_files_list($wu_template_hash);
my @input_file_names=();
my $app_name=$wu_template_hash->{rboinc}->{application};



# Parse result template

my $result_template="rboinc_$template"."_result";
my $result_template_hash=parse_result_template($template);
my $do_chain_bash_function=build_do_chain_bash_function($result_template_hash);






# Should not be user-controllable any longer
my $email=$xml_description->{Email} ||
    $config->{DEFAULT_EMAIL};

my $other_text=sprintf('RND%04d',int(rand(9999)));



# Check old/new style tag validity
my $fail='';
if($tag) {
    $fail="Tag syntax is obsolete. Please use group+name";
} elsif(! defined $name || !$group) {
    $fail="Both group and name must be given";
} elsif(! isTagValid($name)) {
    $fail="Invalid name supplied";
} elsif(! isTagValid($group)) {
    $fail="Invalid group supplied";
} elsif(isNameReserved($name)) {
    $fail="$name is a reserved name";
}



# Check quorum and other sanity parameters

elsif($min_quorum > $target_nresults) {
    $fail="min_quorum (now $min_quorum) must be <= target_nresults ($target_nresults)";
}

# Finally make the shame known to the world 
if($fail) {
    returnFailure($xmlroot,$fail);
    exit(0);
}



##################################################
# Let's start. hash for the output. Put all interesting stuff here,
# will be serialized and sent back to the client

my $oh={};			

$oh->{InputKeys}=$form;
$oh->{ClientIP}=$client_ip;
# $oh->{RandomID}=$random_id;
$oh->{ServerRevision}='$Revision: 353 $';




##################################################
# Move everything to the workflow directory. Make user dir if a deep
# structure is desired. TODO: restrict upload names.

my $userpfx=(uc "$login_name")."_";
my $groupdir="$config->{WORKFLOW_DIR}/$userpfx$group";
my $pooldir="$groupdir/$pool_dirname";
my $namedir="$groupdir/$name";
my $adding_to_group=0;


try 
{
    if(-d $namedir) {
	die "Group/name combination already exists";
    }
    
    if(-d $groupdir) {
	$adding_to_group++;
    } else {
	# need to create group and pool dirs
	mkdir($groupdir,0777) or 
	    die "Can't create group directory on server";
	mkdir($pooldir,0777) or
	    die "Can't create pool directory on server";
    }

    # Create namedir
    mkdir($namedir,0777) or
	die "Can't create name directory on server";
    
    # move files from DAV to NAME
    my @upfiles = glob "$updir/*";
    # $oh->{InputFiles}={file=>\@upfiles};

    chdir($namedir) || die $!;
    foreach my $fn (@upfiles) {
	my $bn=basename($fn);
	# move DAV->NAME
	move($fn,$bn);	
    }




# Process the input list according to template

    foreach my $inf (@$input_files) {

	my $pname="";

	# Create standard COPYRIGHT and LICENSE files, overwriting them
	# even if supplied by the user ("immutable" rboinc flag).
	if($inf->{rboinc}->{immutable}) {
	    $pname=$inf->{open_name};
	    create_standard_files([$pname],1);
	} else {
	    $pname=$inf->{rboinc}->{parameter_name};
	}
	    
	# Create standard idx_file if not supplied by the user.
	if ($inf->{rboinc}->{optional}) {
	    create_standard_files([$pname],0);
	}

	# Encode private files (maybe remove the old ones)
	if($inf->{rboinc}->{encode}) {
	    my $encname=$pname."_enc";
	    encodeinput($pname,$encname,$config->{ENCODE_CODE});
	    $pname=$encname;
	}

	push @input_file_names,'$NAME/'.$pname;

    }




# Move everything into pool (we can play on what to move and what not)
    foreach my $fn (glob "*") {
	move_in_pooldir($fn,$pooldir);
    }

# From now on, files will not be moved in pool any longer
# Prepare machinery for looking at the status
    create_process_vars();

    chdir($groupdir);

# Create the launcher script, if not already there
    if(!$adding_to_group) {
	create_process_sh();
    }




} catch Error with {
    my $ex=shift;
    carp "$ex";
    returnFailure($xmlroot,$ex->text());
    exit(0);
}





# ############################################################
# Launch the job & collect return codes

my $exit_code;

if(!$dry_run) {
    $exit_code=system("/bin/bash process start $name >$name/rboinc_stdout 2>$name/rboinc_stderr");
} else {
    $exit_code=system("echo Dry run requested, not running /bin/bash process start $name >$name/rboinc_stdout 2>$name/rboinc_stderr");
}

# See perlfunc "system"; has some quirks
if($exit_code==0) {
    $oh->{Return}->{ExitCode}=0;
    if(!$dry_run) {
	system("touch $name/in_progress");
    }
} elsif($exit_code==-1) {	# failed to start
    $oh->{Return}->{ExitCode}=-1;
} else {
    $oh->{Return}->{ExitCode} = $? >> 8;
    $oh->{Return}->{SignalNum}=$? & 127;
    $oh->{Return}->{DumpedCore}=$? & 128;
    # TODO: delete dir
}


chdir($namedir) || die $!;
$oh->{Return}->{StdOut}=`cat rboinc_stdout`;
$oh->{Return}->{StdErr}=`cat rboinc_stderr`;
$oh->{Success}=1;


# ############################################################
# If using assigned work, re-read the db
# http://boinc.berkeley.edu/trac/wiki/AssignedWork

if($assign_user_all) {
	open TOUCHDB, "> $config->{PROJECT_DIR}/$reread_db";
	close TOUCHDB;
}





# ############################################################
# CGI ends here


my $outmessage=XMLout($oh, RootName => $xmlroot, AttrIndent => 1  );

print $response_header;
print $outmessage;

open SO, ">submission_output.xml" or die $!;
print SO $outmessage;
close SO;



exit 0;











##################################################
# Create the launcher script. Will be sourced by "process.sh" both
# when creating new work, and when going to the next step. This should
# be called with PWD=name directory

sub create_process_vars {
    if($assign_user_all) {
    	$other_text="$other_text"."_asgn";
    }
    

    open PROCESS, ">process.vars" or die $!;
    print PROCESS <<ENDPROCESS;

# Process variables  created on behalf of $email for job tagged $group/$name on
# $timestamp_string, connected from IP $client_ip.
# Automatically generated by $0 : do not copy, do not edit.
# See $xml_description_file in this directory for details.

# You can modify this file on the fly, if you know what you are doing.


# All-important list of input files. Part of it will be overwritten 
# by chained results
INPUT_LIST=\"@input_file_names\"


# Computed, or fixed to default
TEMPLATE_WU=$wu_template
TEMPLATE_RES=$result_template
LOAD_BALANCER=$balance
CATCHUP_TARGET_NRESULTS=$target_nresults
CATCHUP_PRIORITY=2000

# Passed by user
APPNAME=$app_name
ASSIGN_USER_ALL=$assign_user_all
NUM_STEPS=$num_steps
PRIORITY=$priority
OTHER=$other_text
TARGET_NRESULTS=$target_nresults
MAX_ERROR_RESULTS=5
MAX_TOTAL_RESULTS=10
MIN_QUORUM=$min_quorum
DELAY_BOUND=$delay_bound
FPOPS_EST=$fpops_est
FPOPS_BOUND=$fpops_bound
MEMORY_BOUND=$memory_bound
BATCH=$batch

export PROJECT_DIR=$config->{PROJECT_DIR}


$do_chain_bash_function


ENDPROCESS
close(PROCESS);

}







##################################################
# Create the constant part of the launcher script.The file will be
# created in the GROUP directory and launched from there. It may NOT
# use $name variable, or other per-name variable, because the same
# process file will be recycled for several NAMEs. Per-name variables
# should go in the process.var script.

# First run: invoked with two arguments, "start NAME". Successive
# runs: one argument, ie. WU name, NAME-GROUP-S-M-O. Calling "process"
# with no arguments is an error.


sub create_process_sh {

    open PROCESS, ">process" or die $!;
    print PROCESS <<ENDPROCESS;
#!/bin/bash

# Launcher script created on behalf of $email for job tagged
# $group/$name on $timestamp_string, connected from IP $client_ip.
# Automatically generated by $0 : do not copy, do not edit except
# where indicated.  See $xml_description_file in this directory for
# details.
ENDPROCESS
    
    print PROCESS <<'ENDPROCESS';


# The following will fail if process is a symlink.
GROUP_DIR=$(cd $(dirname $0); pwd -P)


# To manually change priority, target_nresults, etc., see below at the
# *** MANUAL OVERRIDE *** section


# *** MANUAL STOP *** 
# There are three possibilities to interrupt a working workflow
# (best to worst):
#
# 1. use boinc_retrieve -stop -group XX
# 2. create a file named "force-process-stop" in the group
#    or name directory (stops all/one)
# 3. uncomment the line below

# echo "Manually stopped." && exit 0





# Invocation from remote submit
if [ "$#" == 2 ]; then
    # $1 ignored, but supposedly "start"
    NAME=$2
    GROUP=`basename $PWD`
    NEW_STEP=0

elif [ "$#" == 1 ]; then
    # Invocation from assimilator. We have to go in the NAME directory 
    # in order to recover pdb, etc

    # Parse current WU name
    WU_NAME=$1
    OLDIFS=$IFS
    IFS=-
    WU_NAME_ARRAY=($@)
    IFS=$OLDIFS

    NAME=${WU_NAME_ARRAY[0]}
    GROUP=${WU_NAME_ARRAY[1]}
    STEP=${WU_NAME_ARRAY[2]}
    NUM_STEPS=${WU_NAME_ARRAY[3]}
    NEW_STEP=$(($STEP+1))

    if [ "$NEW_STEP" -eq $NUM_STEPS ] ; then
          echo "End of workflow for ${GROUP}-${NAME}."
          rm $GROUP_DIR/$NAME/in_progress
          exit
    fi
else 
    cat <<EOF
Error: process called with wrong number of arguments.
Usage: process start NAME
(or)   process <NAME>-<GROUP>-<STEP>-<MAXSTEP>-<OTHER>
EOF
    exit 1
fi



# Check if we should stop forcibly
if [ -e "$GROUP_DIR/force-process-stop" ]; then
    rm $GROUP_DIR/*/in_progress
    echo "Obeying stop action" && exit 0
fi
if [ -e "$GROUP_DIR/$NAME/force-process-stop" ]; then
    rm $GROUP_DIR/$NAME/in_progress
    echo "Obeying stop action" && exit 0
fi




# Now that we know $NAME, source the job-specific variables to be
# used in the submission

source $GROUP_DIR/$NAME/process.vars


# And now that we have the do_chain function, defined in the sourced
# file, call it if we are at step > 0

if [[ $NEW_STEP -gt 0 ]]; then
    do_chain $WU_NAME
fi
    




# *** MANUAL OVERRIDE *** You can override parameters globally by
# uncommenting them below:

# PRIORITY=100
# TARGET_NRESULTS=2
# DELAY_BOUND=1e5
# LOAD_BALANCER=1
# BATCH=128


# Perform load-balancing: figure out our step wrt the others in this
# group.

catchup=0
pushd $GROUP_DIR > /dev/null

# Update the current and max step for this group
mx=maxstep-file
if [[ -e $mx ]]; then
    maxstep=`cat $mx`
else
    maxstep=0
fi
if (( NEW_STEP > maxstep )); then
    echo $NEW_STEP > $mx
fi

# Check if we have to hurry up
if (( LOAD_BALANCER == 1 && NEW_STEP < maxstep-2 )); then
    catchup=1
fi

popd > /dev/null



# Perform load-balancing: alter parameters to achieve a speed-up

if [[ $catchup -eq 1 ]]; then
    echo "Balancer: catchup mode on"
    TARGET_NRESULTS=$CATCHUP_TARGET_NRESULTS
    PRIORITY=$CATCHUP_PRIORITY
fi



# order is important here, should correspond to the above (?)
# CP_SRC will be prepended WUNIQ -> SRC
# SRC will be hier-'d

WUNIQ=${NAME}-${GROUP}-${NEW_STEP} 
WU_NEW_NAME=${NAME}-${GROUP}-${NEW_STEP}-${NUM_STEPS}-${OTHER}
CP_SRC="$INPUT_LIST"

echo "Processing input '$@' to wu '$WU_NEW_NAME'"




# we now generate SRC on the fly. formerly: SRC="$WUNIQ-LICENSE
# $WUNIQ-COPYRIGHT $WUNIQ-$COO_FILE $WUNIQ-$VEL_FILE $WUNIQ-$IDX_FILE
# $WUNIQ-$PDB_FILE $WUNIQ-$PSF_FILE $WUNIQ-$PAR_FILE $WUNIQ-${NAME}"

cd $PROJECT_DIR

# generate symlinks from download dir to the right places
# and generate list of symlinked names

SRC=""
for T in $CP_SRC; do
    fname=`basename $T`

    # ufname is the official unique name - we could even reduce it as long as stays unique
    ufname="$WUNIQ-$fname"
    SRC="$SRC $ufname"
    file_in_workarea=$GROUP_DIR/$T
    file_in_upload=`bin/dir_hier_path $ufname`

    # Check existence or die
    if [[ ! -r $file_in_workarea ]]; then
        echo "Download file $file_in_workarea missing, creating empty"
        touch $file_in_workarea
        #exit 1
    fi


    # This could produce an error if files are shared
    ln $file_in_workarea $file_in_upload
    if [[ $? -ne 0 ]]; then
        echo "Command ln $file_in_workarea $file_in_upload failed. Continuing."
        # rm $file_in_upload
        # ln $file_in_workarea $file_in_upload
        # exit 1
    fi
done

bin/create_work \
    ${TARGET_NRESULTS:+ -target_nresults $TARGET_NRESULTS} \
    ${MIN_QUORUM:+ -min_quorum $MIN_QUORUM} \
    -max_error_results ${MAX_ERROR_RESULTS} \
    -max_total_results ${MAX_TOTAL_RESULTS} \
    ${DELAY_BOUND:+ -delay_bound $DELAY_BOUND} \
    ${ASSIGN_USER_ALL:+ -assign_user_all $ASSIGN_USER_ALL} \
    ${FPOPS_EST:+ -rsc_fpops_est $FPOPS_EST} \
    ${FPOPS_BOUND:+ -rsc_fpops_bound $FPOPS_BOUND} \
    ${MEMORY_BOUND:+ -rsc_memory_bound $MEMORY_BOUND} \
    ${BATCH:+ -batch $BATCH} \
    -appname ${APPNAME} \
    -wu_name ${WU_NEW_NAME} \
    -wu_template templates/${TEMPLATE_WU} \
    -result_template templates/${TEMPLATE_RES} \
    -priority $(($PRIORITY-$NEW_STEP)) \
    ${SRC}

ecode=$?
if [[ $ecode -ne 0 ]]; then 
    echo "Submit failure for $1: $ecode"
    exit $ecode
else
    echo ""
    echo "Submitted $WU_NEW_NAME"
fi

ENDPROCESS

close PROCESS;

system("chmod a+x process");

}



















# These will be copied from the CGI directory. If first argument=1,
# overwrite files even if they exist

sub create_standard_files {
    my $filelist=shift;
    my $overwrite=shift;
    foreach my $f (@$filelist) {
	if(! -e $f || $overwrite) {
	    copy("$config->{ETC_DIR}/default_$f","$f") || die "Error copying default $f: $!";
	}
    }
}





# Performs the encode-input command. Relies on $arch to be set
sub encodeinput {
    my ($in,$out,$code)=@_;
    my $r=system("$config->{ENCODE_EXECUTABLE}.$arch $in $out $code");
    if($r!=0) {
	die "encoder returned error $r";
    }
}



# Move the given file in pooldir, and link to it
# Assume we are in namedir
# $fn is the basename of the file to be moved
# $pooldir is the path to the pooldir
# after execution, $fn will be a link
sub move_in_pooldir {
    my ($fn,$pooldir)=@_;

    my $md5=md5_file($fn);
    my $poolfile="$pooldir/$md5";

    if(-r $poolfile) {
	# Already in pool: drop current copy, make link
	unlink($fn) or die "Error unlinking $fn"; 
	link($poolfile,$fn) or die "Error hard-linking pool to $fn";
    } else {
	# Not in pool: link current copy there
	link($fn,$poolfile) or die "Error hard-linking $fn to pool";
    }

    # old-style symlinking
    # symlink("$pooldir/$md5","$fn") or die "Error symlinking $fn";

}

