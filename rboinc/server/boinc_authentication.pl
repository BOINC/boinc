#!/usr/bin/perl -w

# $Id: boinc_authentication.pl 355 2010-03-02 14:56:57Z toni $


=head1 NAME

boinc_authentication - functions to authenticate an user with Boinc,
and other miscellaneous helpers.

=head1 SYNOPSIS

    lookup_email(project_url,email)
    lookup_email_password(project_url,email,password)

=head1 DESCRIPTION

Connects to a boinc project and checks if an user exists, or if a
given username/password combination is valid. The boinc authenticator
can also be used in lieu of the password. Passwords are never sent in
clear text.

=head1 AUTHOR

Toni Giorgino

=head1 COPYRIGHT

This file is part of RemoteBOINC.
Copyright (C) 2010 Toni Giorgino, Universitat Pompeu Fabra

RemoteBOINC is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

RemoteBOINC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


=cut



use strict;

use Digest::MD5  qw(md5 md5_hex md5_base64);
use Error qw(:try);

use LWP::Simple;
use URI::Escape;
use XML::Simple;

use constant LOOKUP_ACCOUNT_PHP         => 'lookup_account.php';

use vars qw($config);


sub md5_file {
    my $file = shift;
    open(FILE, $file) or die 'Error file $file for md5: $!';
    binmode(FILE);
    my $md= Digest::MD5->new->addfile(*FILE)->hexdigest;
    close FILE;
    return $md;
}



sub lookup_email {
    my $boinc_project_url=shift;
    my $boinc_email=shift;

    my $url=$boinc_project_url . "/" . LOOKUP_ACCOUNT_PHP;
    my $query=$url."?email_addr=".uri_escape($boinc_email);
    my $content=get($query);

    die "Couldn't get it!" unless defined $content;
    return $content;

}


sub lookup_email_password {
    my $boinc_project_url=shift;
    my $boinc_email=shift;
    my $boinc_password=shift;

    my $boinc_digesthash=md5_hex($boinc_password.$boinc_email);

    my $url=$boinc_project_url . "/" . LOOKUP_ACCOUNT_PHP;
    my $query=$url."?email_addr=".uri_escape($boinc_email).
	"&passwd_hash=".uri_escape($boinc_digesthash);

    my $content=get($query);

    die "Couldn't get it!" unless defined $content;
    return $content;
}



# Return an  human-readable answer, for testing

sub voidAnswer {
    my $response_header="Content-type: text/plain\n\n";
    print  $response_header;
    print "Remote boinc CGI is alive.\n";
}



# Return an xml-encoded failure code

sub returnFailure {
    my $xmlroot=shift;
    my $reason=shift;
    my $response_header="Content-type: text/plain\n\n";

    my $r={};
    $r->{Failure}={Reason=>$reason};

    my $xr=XMLout($r, RootName => $xmlroot, AttrIndent => 1);

    print $response_header;
    print $xr;
}



# Returns false if tag contains forbidden chars, else true

sub isTagValid {
    my $tag=shift;
    if($tag =~ /^[a-zA-Z0-9_]+$/) {
	return 1;
    } else {
	return 0;
    }
}


# Returns false if tag contains forbidden chars in user name, else true

sub isUserValid {
    my $tag=shift;
    if($tag =~ /^[a-zA-Z0-9]+$/) {
	return 1;
    } else {
	return 0;
    }
}


sub isNameReserved {
    my $name=shift;
    my @res=("pool","process","process_stop");

    foreach my $n (@res) {
	if($name eq $n) {
	    return(1);
	}
    }
    return 0;
}



# Template-handling


# Parse the wu template and return it as an hash
sub parse_wu_template {
    my $tpl=shift;
    
    if(!isTagValid($tpl)) {
	die "Invalid character in application request";
    }

    # Read the template's content
    my $tfile=$config->{PROJECT_DIR}."/templates/";
    $tfile=$tfile."rboinc_".$tpl."_wu";

    open F,"<$tfile" or die "Error opening template: $!";
    my @lines=<F>;
    close F;
    my $ttext=join "",@lines;
    
    # Add the root element, otherwise ill-formed
    my $txml=XMLin("<opt>$ttext</opt>",
		   ForceArray => ["file_ref"]);
    return $txml;

}



# Parse the wu template and return it as an hash
sub parse_result_template {
    my $tpl=shift;
    
    if(!isTagValid($tpl)) {
	die "Invalid character in application request";
    }

    # Read the template's content
    my $tfile=$config->{PROJECT_DIR}."/templates/";
    $tfile=$tfile."rboinc_".$tpl."_result";

    open F,"<$tfile" or die "Error opening template: $!";
    my @lines=<F>;
    close F;
    my $ttext=join "",@lines;
    
    # Add the root element, otherwise ill-formed
    my $txml=XMLin("<opt>$ttext</opt>",
		   ForceArray => ["file_ref"]);
    return $txml;

}



# Convert the input list into an ORDERED array of hash refs.
# Each hash ref contains info for an input file 
sub build_input_files_list {
    my $th=shift;
    my $out=[];

    foreach my $ir (@{$th->{workunit}->{file_ref}}) {
	my $num=$ir->{file_number};
	$out->[$num]=$ir;
    }
    
    return $out;
}



# Build list of chained files, inverting links in the
# results template
sub build_chain_files_list {
    my $th=shift;
    my $out={};

    foreach my $ir (@{$th->{file_info}}) {
	my $name=(%{$ir->{name}})[0]; # this will become the new input
	$name=~/[0-9]+$/;
	my $num=$&;
	my $chain=$ir->{rboinc}->{chain}; # at this slot
	if($chain) {
	    $out->{$chain}=$num;
	}
    }
    
    return $out;
}

# Build a fragment of bash script
# applying the chain-rule replacements
sub build_do_chain_bash_function {
    my $th=shift;

    my $clist=build_chain_files_list($th);

    my $scr=<<'EOF';
function do_chain {
  iarray=($INPUT_LIST)
EOF
    
    foreach my $in (sort keys %$clist) {
	my $out=$clist->{$in};
	$scr .= " iarray[$in]=\$1_$out\n";
    }

    $scr.=<<'EOF';
  INPUT_LIST="${iarray[@]}"
}
EOF

    return $scr;
}



1;







# References
# http://snippets.dzone.com/posts/show/3163
# http://boinc.berkeley.edu/trac/wiki/WebRpc#lookup_account
#     project_url/lookup_account.php 
# http://search.cpan.org/dist/libwww-perl/lib/HTTP/Request/Common.pm
