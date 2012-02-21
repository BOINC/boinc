#!/usr/bin/perl 

# $Id$

# Script to generate the per-error per-user pivot  report

use strict;
use warnings;
use Text::Table;
use DBI;
use monitoring_errors_table;

my $dbuser="DB_USER";
my $dbpass="DB_PASSWORD";

# List of errors to monitor
my @tomon=(-198,-197,-233,-186,-226,-185,-177,-187,1,2,3);




## End user-serviceable parts

our %error_table;

my $dbh = DBI->connect('DBI:mysql:DB_NAME', $dbuser, $dbpass
	           ) || die "Could not connect to database: $DBI::errstr";



# Generate a table of errors in the last 24 h
# in principle, the time intervals could be selected as 
# SELECT distinct monitor_time t FROM monitoring m order by t desc limit 1,1
# i.e. the second-to-last monitoring time

$dbh->do('set @ut=unix_timestamp();') || 
    die "Query error: $DBI::errstr";
$dbh->do('set @uf=@ut-3600*24;') ||
    die "Query error: $DBI::errstr";

my $sql = <<'EOF';
    SELECT mon_submitterof(name) sci, exit_status
    FROM   result 
    WHERE  outcome=3 
    AND    received_time BETWEEN @uf and @ut;
EOF

# $sql='select @uf,@ut';

my $st=$dbh->prepare($sql);
$st->execute();



# Initialize the table of descriptions of "interesting" errors
my $colsr={other=>"(Any other exit code)"};
foreach my $e (@tomon) {
    my $t=$error_table{$e} || "(Exit code by app)";
    $t=~s/^ERR_//;
    $colsr->{$e}=$t;
}


# Pivot table
my $data={};
while(my @res=$st->fetchrow_array()) {
    my ($sci,$err)=@res;
    if($colsr->{$err}) {	# interesting?
	$data->{$sci}->{$err}++;
    } else {			# otherwise
	$data->{$sci}->{other}++;
    }
}

# List of scientists: sort keys %$data
# List of errors:     sort keys %$colsr

# Print table header 1
my @header=("Code","Error",sort keys %$data);
my $table = Text::Table->new( @header );


# Table contents
foreach my $col (sort keys %$colsr ) {
    my @body=($col,$colsr->{$col});
    foreach my $sci (sort keys %$data) {
	my $n=$data->{$sci}->{$col} || 0;
	push @body,$n;
    }
    $table->add(@body);
}

print $table;

