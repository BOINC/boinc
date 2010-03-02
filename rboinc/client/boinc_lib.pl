# Common client routines.

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


# $Id: boinc_lib.pl 356 2010-03-02 15:00:31Z toni $

use XML::Simple;
use LWP::UserAgent;

use vars qw($verbose);
use strict;


# Request the remote DAV address. Dies on failure.

sub getDavUrl {
    my $cgi_url=shift;

    my $ua = new LWP::UserAgent; 
    my $response = $ua->post( $cgi_url,
			      { action => 'get_dav_url'
			      });

    if($response->is_success) {
	my $content = $response->content; 
	logInfo("Response received for DAV:\n$content\n",1);
	my $ch=XMLin($content);
	my $du=$ch->{DavUrl};
	if(!$du) {
	    die "Error getting DAV address";
	}
	return $du;
    } else {
	my $reason=$response->status_line;
	die "Error in getting DAV address from remote: $reason. Server may be down.\n";
    }

}




# Request the wu template for a specific app (used to extract cli
# arguments). Dies on failure.

sub getWuTemplate {
    my $cgi_url=shift;
    my $app=shift;

    my $ua = new LWP::UserAgent; 
    my $response = $ua->post( $cgi_url,
			      { action => 'get_wu_template',
				application => $app
			      });

    if($response->is_success) {
	my $content = $response->content; 
	logInfo("Response received for template:\n$content\n",1);
	# fixup the template, which lacks the root element
	my $ch=XMLin($content,
		     ForceArray => ["file_info"] );
	if($ch->{Failure}) {
	    my $reason=$ch->{Failure}->{Reason};
	    die "Error in getting WU template remote: $reason.\n".
		"You may have selected a non-configured application";
	}
	return $ch;
    } else {
	my $reason=$response->status_line;
	die "Error in getting template from remote: $reason. Server may be down.\n";
    }

}


# Extract parameter list from the template (in no particular order)

sub parseWuTemplate {
    my $t=shift;
    my $p={};

    if(!($t->{WuTemplate}->{rboinc}->{application})) {
	die "Not a remote-enabled application.";
    }
    
    # reference to array of file_info. Each element is a hash of number and rboinc
    my @fi=@{$t->{WuTemplate}->{workunit}->{file_ref}};
    foreach my $fn (@fi) {
	my $pname=$fn->{rboinc}->{parameter_name};
	my $pdesc=$fn->{rboinc}->{parameter_description} || "(undocumented)";
	my $poptional=$fn->{rboinc}->{optional};
	if($pname) {
	    $p->{$pname}={
		description=>$pdesc,
		optional=>$poptional }
	}
    }
    return $p;
}





# Check if the given files (as list) are all readable.

sub checkReadableFiles {
    my $l=shift;
    foreach my $f (@$l) {
	if(! -r $f) {
	    print STDERR "File $f is not readable. Exiting.\n";
	    return 0;
	}
    }
    return 1;
}




# Unused

sub waitKey {
    my $tmp=<>;
}



# Print on STDOUT if the $verbose global variable is defined.
# 1st arg: message; 2nd arg: whether to print separator lines.

sub logInfo {
    my $msg=shift;
    my $lines=shift;
    my $in="";
    my $out="";

    if(defined $lines) {
	 $in="vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n";
	$out="^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";
    }

    if($verbose) {
	print "$in$msg$out\n";
    }
}



# http://www.devdaily.com/perl/edu/articles/pl010005/pl010005.shtml

#----------------------------(  promptUser  )-----------------------------#
#                                                                         #
#  FUNCTION:	promptUser                                                #
#                                                                         #
#  PURPOSE:	Prompt the user for some type of input, and return the    #
#		input back to the calling program.                        #
#                                                                         #
#  ARGS:	$promptString - what you want to prompt the user with     #
#		$defaultValue - (optional) a default value for the prompt #
#                                                                         #
#-------------------------------------------------------------------------#

sub promptUser {

   #-------------------------------------------------------------------#
   #  two possible input arguments - $promptString, and $defaultValue  #
   #  make the input arguments local variables.                        #
   #-------------------------------------------------------------------#

   my ($promptString,$defaultValue) = @_;

   #-------------------------------------------------------------------#
   #  if there is a default value, use the first print statement; if   #
   #  no default is provided, print the second string.                 #
   #-------------------------------------------------------------------#

   if ($defaultValue) {
      print $promptString, "[", $defaultValue, "]: ";
   } else {
      print $promptString, ": ";
   }

   $| = 1;               # force a flush after our print
   $_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)


   #------------------------------------------------------------------#
   # remove the newline character from the end of the input the user  #
   # gave us.                                                         #
   #------------------------------------------------------------------#

   chomp;

   #-----------------------------------------------------------------#
   #  if we had a $default value, and the user gave us input, then   #
   #  return the input; if we had a default, and they gave us no     #
   #  no input, return the $defaultValue.                            #
   #                                                                 # 
   #  if we did not have a default value, then just return whatever  #
   #  the user gave us.  if they just hit the <enter> key,           #
   #  the calling routine will have to deal with that.               #
   #-----------------------------------------------------------------#

   if ("$defaultValue") {
      return $_ ? $_ : $defaultValue;    # return $_ if it has a value
   } else {
      return $_;
   }
}



1;
