#! /usr/bin/perl -w

use strict;
use vars qw(@replaces);
use IO::File;

sub apply($) {
	my $line = shift;
	foreach my $elem (@replaces) {
		my $reg = $elem->[0];
		my $rep = $elem->[1];
		$line =~ s/$reg/$rep/g;
	}
	return $line;
}

# From gtkdoc-mkdb
sub CreateValidSGMLID($) {
    my ($id) = $_[0];

    # Append ":CAPS" to all all-caps identifiers

    # Special case, '_' would end up as '' so we use 'gettext-macro' instead.
    if ($id eq "_") { return "gettext-macro"; }

    if ($id !~ /[a-z]/) { $id .= ":CAPS" };

    $id =~ s/[_ ]/-/g;
    $id =~ s/[,\.]//g;
    $id =~ s/^-*//;
    $id =~ s/::/-/g;

    return $id;
}
die("Missing prefix\n") unless $ARGV[2];

my $fh = new IO::File $ARGV[0], O_RDONLY
	or die("Failed to open symbol file " . $ARGV[0] . "\n");
while (my $line = $fh->getline) {
	chomp $line;
	next if $line =~ m!/!;

	$line = CreateValidSGMLID($line);

	my $reg = 'id="' . $line;
	push @replaces, [ qr/$reg/, 'id="' . $ARGV[2] . $line ];
	$reg = 'linkend="' . $line;
	push @replaces, [ qr/$reg/, 'linkend="' . $ARGV[2] . $line ];
}
$fh->close;

$fh = new IO::File $ARGV[1], O_RDONLY
	or die("Failed to open input file " . $ARGV[1] . "\n");
my $ofh = new IO::File $ARGV[1] . '.new', O_WRONLY | O_CREAT | O_TRUNC
	or die("Failed to open new input file " . $ARGV[1] . ".new\n");
while (my $line = $fh->getline) {
	my $newline = apply($line);
	$ofh->print($newline);
}

$fh->close;
$ofh->close;
rename($ARGV[1] . '.new', $ARGV[1]);
