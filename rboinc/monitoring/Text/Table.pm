# Text::Table - Organize Data in Tables
package Text::Table;
use strict;
use warnings;

use Text::Aligner qw( align);

BEGIN {
    use Exporter ();
    our $VERSION = ('$Revision: 1.114 $' =~ /(\d+.\d+)/)[ 0];

}

use overload
    '""'  => 'stringify',
;

### User interface:  How to specify columns and column separators

sub _is_sep {
    local $_ = shift;
    defined and ( ref eq 'SCALAR' or ( ref eq 'HASH' and $_->{ is_sep}));
}

sub _parse_sep {
    local $_ = shift;
    defined $_ or $_ = '';
    my ( $title, $body);
    if ( ref eq 'HASH' ) {
        ( $title, $body) = @{ $_}{ qw( title body)};
    } else {
        ( $title, $body) = split /\n/, $$_, -1;
    }
    $body = $title unless defined $body;
    align( 'left', $title, $body);
    {
        is_sep => 1,
        title  => $title,
        body   => $body,
    }
}

sub _parse_spec {
    local $_ = shift;
    defined $_ or $_ = '';
    my $alispec = qr/^ *(?:left|center|right|num|point|auto)/;
    my ( $title, $align, $align_title, $align_title_lines, $sample);
    if ( ref eq 'HASH' ) {
        ( $title, $align, $align_title, $align_title_lines, $sample) =
            @{ $_}{ qw( title align align_title align_title_lines sample)};
    } else {
        my $alispec = qr/&(.*)/;
        if ( $_ =~ $alispec ) {
            ( $title, $align, $sample) = /(.*)^$alispec\n?(.*)/sm;
        } else {
            $title = $_;
        }
        defined and chomp for $title, $sample;
    }
    defined or $_ = [] for $title, $sample;
    defined $align and length $align or $align = 'auto';
    ref eq 'ARRAY' or $_ = [ split /\n/, $_, -1] for $title, $sample;
    unless (
        ref $align eq 'Regex' or
        $align =~ /^(?:left|center|right|num\(?|point\(?|auto)/
    ) {
        _warn( "Invalid align specification: '$align', using 'auto'");
        $align = 'auto';
    }
    defined $align_title and length $align_title or $align_title = 'left';
    unless ( $align_title =~ /^(?:left|center|right)/ ) {
        _warn( "Invalid align_title specification: " .
            "'$align_title', using 'left'",
        );
        $align_title = 'left';
    }
    defined $align_title_lines and length $align_title_lines or
        $align_title_lines = $align_title;
    unless ( $align_title_lines =~ /^(?:left|center|right)/ ) {
        _warn( "Invalid align_title_lines specification: " .
            "'$align_title_lines', using 'left'",
        );
        $align_title_lines = 'left';
    }
    {
        title             => $title,
        align             => $align,
        align_title       => $align_title,
        align_title_lines => $align_title_lines,
        sample            => $sample,
    }
}

### table creation

sub new {
    my $tb = bless {}, shift;
    $tb->_entitle( @_);
}

sub _entitle {
    my $tb = shift; # will be completely overwritten
    # find active separators and, well separate them from col specs.
    # n+1 separators for n cols
    my ( @seps, @spec); # separators and column specifications
    my $sep;
    for ( @_ ) {
        if ( _is_sep ( $_) ) {
            $sep = _parse_sep( $_);
        } else {
            push @seps, $sep;
            push @spec, _parse_spec( $_);
            undef $sep;
        }
    }
    push @seps, $sep;
    # build sprintf formats from separators
    my $title_form =
        _compile_format( map defined() ? $_->{ title} : undef, @seps);
    my $body_form  =
        _compile_format( map defined() ? $_->{ body} : undef, @seps);

    # pre_align titles
    my @titles = map [ @{ $_->{ title}}], @spec;
    my $title_height = 0;
    _to_max( $title_height, scalar @$_) for @titles;
    push @$_, ( '') x ( $title_height - @$_) for @titles;
#   align( 'left', @$_) for @titles; # ready for use'
    my @styles = map $_->{ align_title_lines}, @spec;
    align( shift @styles, @$_) for @titles; # in-place alignment

    # build data structure
    %$tb = (
        spec => \ @spec,                     # column spec for reuse
        titles => \ @titles,                 # titles, pre-aligned
        cols => [ map [], 1 .. @spec],       # data columns
        forms => [ $title_form, $body_form], # separators condensed
    );
    $tb->_clear_cache;
}

# sprintf-format for line assembly, using separators
sub _compile_format {
   my @seps = @_; # mix of strings and undef (for default)
   defined or $_ = '' for @seps[ 0, -1]; # first and last default to empty
   defined or $_ = ' ' for @seps; # others default to single space
   s/%/%%/g for @seps; # protect against sprintf
   join '%s', @seps;
}

# reverse format compilation (used by colrange())
sub _recover_separators {
    my $format = shift;
    my @seps = split /(?<!%)%s/, $format, -1;
    s/%%/%/g for @seps;
    @seps;
}

# select some columns, (optionally if in [...]), and add new separators
# (the other table creator)
sub select {
    my $tb = shift;
    my @args = map $tb->_select_group( $_), @_;
    # get column selection, checking indices (some have been checked by
    # _select_group, but not all)
    my @sel = map $tb->_check_index( $_), grep !_is_sep( $_), @args;
    # replace indices with column spec to create subtable
    ! _is_sep( $_) and $_ = $tb->{ spec}->[ $_] for @args;
    my $sub = ref( $tb)->new( @args);
    # sneak in data columns
    @{ $sub->{ cols}} = map [ @$_ ], @{ $tb->{ cols}}[ @sel];
    $sub;
}

# the first non-separator column in the group is tested if it has any data
# if so, the group is returned, else nothing
sub _select_group {
    my ( $tb, $group) = @_;
    return $_ unless ref $group eq 'ARRAY';
    for ( @$group ) {
        next if _is_sep( $_);
        $tb->_check_index( $_);
        return @$group if grep $_, @{ $tb->{ cols}->[ $_]};
        return; # no more tries after non-sep was found
    }
    return; # no column index in group, no select
}

# check index for validity, return arg if returns at all
sub _check_index {
    my $tb = shift;
    my ( $i) = @_;
    my $n = $tb->n_cols;
    my $ok = eval {
        use warnings FATAL => 'numeric';
        -$n <= $i and $i < $n; # in range of column array?
    };
    _warn( "Invalid column index '$_'") if $@ or not $ok;
    shift;
}

### data entry

sub _clear_cache { @{ $_[ 0]}{ qw( lines blank)} = (); $_[ 0] }

# add one data line or split the line into follow-up lines
sub add {
    my $tb = shift;
    $tb->_entitle( ( '') x @_) unless $tb->n_cols;

    $tb->_add( @$_) for _transpose( map [ defined() ? split( $/ ) : '' ], @_);
    $tb->_clear_cache;
}   

# add one data line
sub _add {
    my $tb = shift;
    push @$_, shift for @{ $tb->{ cols}};
    $tb->_clear_cache;
    $tb;
}

# add one or more data lines
sub load {
    my $tb = shift;
    for ( @_ ) {
        defined $_ or $_ = '';
        ref eq 'ARRAY' ? $tb->add( @$_) : $tb->add( split);
    }
    $tb;
}

sub clear {
    my $tb = shift;
    $_ = [] for @{ $tb->{ cols}};
    $tb->_clear_cache;
    $tb;
}

### access to output area

## sizes

# number of table clolumns
sub n_cols { scalar @{ $_[0]->{ spec}} }

# number of title lines
sub title_height { $_[ 0]->n_cols and scalar @{ $_[ 0]->{ titles}->[ 0]} }

# number of data lines
sub body_height { $_[ 0]->n_cols and scalar @{ $_[ 0]->{ cols}->[ 0]} }

# total height
sub table_height { $_[ 0]->title_height + $_[ 0]->body_height }
BEGIN { *height = \ &table_height} # alias

# number of characters in each table line. need to build the table to know
sub width {
    $_[ 0]->height and length( ($_[ 0]->table( 0))[ 0]) - 1;
}

# start and width of each column
sub colrange {
    my ( $tb, $col_index) = @_;
    return ( 0, 0) unless $tb->width; # width called, $tb->{ blank} exists now
    $col_index ||= 0;
    $col_index += $tb->n_cols if $col_index < 0;
    _to_max( $col_index, 0);
    _to_min( $col_index, $tb->n_cols);
    my @widths = map length, @{ $tb->{ blank}}, '';
    @widths = @widths[ 0 .. $col_index];
    my $width = pop @widths;
    my $pos = 0;
    $pos += $_ for @widths;
    my @seps = _recover_separators( $tb->{ forms}->[ 0]);
    $pos += length for @seps[ 0 .. $col_index];
    return ( $pos, $width);
}

## printable output

# whole table
sub table {
    my $tb = shift;
    $tb->_table_portion( $tb->height, 0, @_);
}

# only titles
sub title {
    my $tb = shift;
    $tb->_table_portion( $tb->title_height, 0, @_);
}   

# only body
sub body {
    my $tb = shift;
    $tb->_table_portion( $tb->body_height, $tb->title_height, @_);
}

sub stringify { scalar shift()->table() }

### common internals

# common representation of table(), title() and body()
sub _table_portion {
    my $tb = shift;
    my ( $total, $offset) = ( shift, shift);
    my ( $from, $n) = ( 0, $total); # if no parameters
    if ( @_ ) {
        $from = shift;
        $n = @_ ? shift : 1; # one line if not given
    }
    ( $from, $n) = _limit_range( $total, $from, $n);
    my @lines = do {
        my $limit = $tb->title_height; # title format below
        $from += $offset;
        map $tb->_assemble_line( $_ >= $limit, $tb->_table_line( $_)),
            $from .. $from + $n - 1;
    };
    return @lines if wantarray;
    return join '', @lines;
}

sub _limit_range {
    my ( $total, $from, $n) = @_;
    $from ||= 0;
    $from += $total if $from < 0;
    $n = $total unless defined $n;
    return ( 0, 0) if $from + $n < 0 or $from >= $total;
    $from = 0 if $from < 0;
    $n = $total - $from if $n > $total - $from;
    ( $from, $n);
}

# get table line (formatted, including titles). fill cache if needed
sub _table_line {
    my $tb = shift;
    ($tb->{ lines} ||= [ $tb->_build_table_lines])->[ shift];
}

# build array of lines of justified data items
sub _build_table_lines {
    my $tb = shift;

    # copy data columns, replacing undef with ''
    my @cols = map [ map defined() ? $_ : '', @$_], @{ $tb->{ cols}};

    # add set of empty strings for blank line (needed to build horizontal rules)
    push @$_, '' for @cols;

    # add samples for minimum alignment
    my @samples = map $_->{ sample}, @{ $tb->{ spec}};
    push @$_, @{ shift @samples} for @cols;

    # align to style
    my @styles = map $_->{ align}, @{ $tb->{ spec}};
    align( shift @styles, @$_) for @cols;
    # trim off samples, but leave blank line
    splice @$_, 1 + $tb->body_height for @cols; # + 1 for blank line (brittle)

    # include titles
    my @titles = @{ $tb->{ titles}};
    unshift @$_, @{ shift @titles} for @cols; # add pre-aligned titles

    # align title and body portions of columns
    # blank line will be there even with no data
    @styles = map $_->{ align_title}, @{ $tb->{ spec}};
    align( shift @styles, @$_) for @cols; # in-place alignment

    # deposit a blank line, pulling it off the columns.
    # *_rule() knows this is done
    $tb->{ blank} = [ map pop @$_, @cols];

    _transpose_n( $tb->height, @cols); # bye-bye, @cols
}

# destructively transpose a number of lines/cols from an array of arrayrefs 
sub _transpose_n ($@) {
    my $n = shift;
    map [ map shift @$_, @_], 1 .. $n;
}

# like _transpose_n, but find the number to transpose from max of given
sub _transpose {
    my $m;
    _to_max( $m, scalar @$_) for @_, []; # make sure $m is defined
    _transpose_n( $m, @_);
}

# make a line from a number of formatted data elements
sub _assemble_line {
    my $tb = shift;
    my $in_body = shift; # 0 for title, 1 for body
    sprintf( $tb->{ forms}->[ !!$in_body], @{ shift()}) . "\n";
}

# build a rule line
sub _rule {
    my $tb = shift;
    my $in_body = shift;
    return '' unless $tb->width; # this builds the cache, hence $tb->{ blank}
    my $rule = $tb->_assemble_line( $in_body, $tb->{ blank});
    my ( $char, $alt) = map /(.)/, @_;
    ( defined $char and length $char) or $char = ' ';
    # replace blanks with $char. If $alt is given, replace nonblanks with $alt
    if ( defined $alt ) {
        $rule =~ s/(.)/$1 eq ' ' ? $char : $alt/ge;
    } else {
        $rule =~ s/ /$char/g if $char ne ' ';
    }
    $rule;
}

sub rule {
    my $tb = shift;
    $tb->_rule( 0, @_);
}

sub body_rule {
    my $tb = shift;
    $tb->_rule( 1, @_);
}

# min/max utilitiess (modifying first argument)

sub _to_max {
    defined $_[ 0] and $_[ 0] > $_[ 1] or $_[ 0] = $_[ 1] if defined $_[ 1];
}

sub _to_min {
    defined $_[ 0] and $_[ 0] < $_[ 1] or $_[ 0] = $_[ 1] if defined $_[ 1];
}

### warning behavior
use Carp;

{
    my ( $warn, $fatal) = ( 0, 0);

    sub warnings {
        shift; # ignore class/object
        local $_ = shift || 'on';
        if ( $_ eq 'off' ) {
            ( $warn, $fatal) = ( 0, 0);
        } elsif ( $_ eq 'fatal' ) {
            ( $warn, $fatal) = ( 1, 1);
        } else {
            ( $warn, $fatal) = ( 1, 0);
        }
        return 'fatal' if $fatal;
        return 'on' if $warn;
        return 'off';
    }

    sub _warn {
        my $msg = shift;
        return unless $warn;
        croak( $msg) if $fatal;
        carp( $msg);
    }
}

__END__
########################################### main pod documentation begin ##

=head1 NAME

Text::Table - Organize Data in Tables

=head1 SYNOPSIS

    use Text::Table;
    my $tb = Text::Table->new(
        "Planet", "Radius\nkm", "Density\ng/cm^3"
    );
    $tb->load(
        [ "Mercury", 2360, 3.7 ],
        [ "Venus", 6110, 5.1 ],
        [ "Earth", 6378, 5.52 ],
        [ "Jupiter", 71030, 1.3 ],
    );
    print $tb;

This prints a table from the given title and data like this:

  Planet  Radius Density
          km     g/cm^3 
  Mercury  2360  3.7    
  Venus    6110  5.1    
  Earth    6378  5.52   
  Jupiter 71030  1.3    

Note that two-line titles work, and that the planet names are aligned
differently than the numbers.

=head1 DESCRIPTION

Organization of data in table form is a time-honored and useful
method of data representation.  While columns of data are trivially
generated by computer through formatted output, even simple tasks
like keeping titles aligned with the data columns are not trivial,
and the one-shot solutions one comes up with tend to be particularly
hard to maintain.  Text::Table allows you to create and maintain
tables that adapt to alignment requirements as you use them.

=head2 Overview

The process is simple: you create a table (a Text::Table object) by
describing the columns the table is going to have.  Then you load
lines of data into the table, and finally print the resulting output
lines.  Alignment of data and column titles is handled dynamically
in dependence on the data present.

=head2 Table Creation

In the simplest case, if all you want is a number of (untitled) columns,
you create an unspecified table and start adding data to it.  The number
of columns is taken fronm the first line of data.

To specify a table you specify its columns.  A column description
can contain a title and alignment requirements for the data, both
optional.  Additionally, you can specify how the title is aligned with
the body of a column, and how the lines of a multiline title are
aligned among themselves.

The columns are collected in the table in the
order they are given.  On data entry, each column corresponds to
one data item, and in column selection columns are indexed left to
right, starting from 0.

Each title can be a multiline string which will be blank-filled to
the length of the longest partial line.  The largest number of title
lines in a column determines how many title lines the table has as a
whole, including the case that no column has any titles.

On output, Columns are separated by a single blank.  You can control
what goes between columns by specifying separators between (or before,
or after) columns.  Separators don't contain any data and don't count
in column indexing.  They also don't accumulate: in a sequence of only
separators and no columns, only the last one counts.

=head2 Status Information

The width (in characters), height (in lines), number of columns, and
similar data about the table is available.

=head2 Data Loading

Table data is entered line-wise, each time specifying data entries
for all table columns.  A bulk loader for many lines at once is also
available.  You can clear the data from the table for re-use (though
you will more likely just create another table).

=head2 Table Output

The output area of a table is divided in the title and the body.

The title contains the combined titles from the table columns, if
any.  Its content never changes with a given table, but it may be
spread out differently on the page through alignment with the data.

The body contains the data lines, aligned column-wise as specified,
and left-aligned with the column title.

Each of these is arranged like a Perl array (counting from 0) and can
be accessed in portions by specifying a first line and the number
of following lines.  Also like an array, giving a negative first line
counts from the end of the area.  The whole table, the title followed
by the body, can also be accessed in this manner.

The subdivisions are there so you can repeat the title (or parts of
it) along with parts of the body on output, whether for screen paging
or printout.

A rule line is also available, which is the horizontal counterpart to
the separator columns you specify with the table.
It is basically a table line as it would appear if all data entries
in the line were empty, that is, a blank line except for where the
column separators have non-blank entries.  If you print it between
data lines, it will not disrupt the vertical separator structure
as a plain blank line would.  You can also request a solid rule
consisting of any character, and even one with the non-blank column
separators replaced by a character of your choice.  This way you can
get the popular representation of line-crossings like so:

      |
  ----+---
      |

=head2 Warning Control

On table creation, some parameters are checked and warnings issued
if you allow warnings.  You can also turn warnings into fatal errors.

=head1 SPECIFICATIONS

=head2 Column Specification

Each column specification is a single scalar.  Columns can be either proper
data columns or column separators.  Both can be specified either as
(possibly multi-line) strings, or in a more explicit form as hash-refs.
In the string form, proper columns are given as plain strings, and
separators are given as scalar references to strings.  In hash form,
separators have a true value in the field C<is_sep> while proper columns
don't have this field.

=over 4

=item Columns as strings

A column is given as a column title (any number of lines),
optionally followed by alignment requirements.  Alignment requirements
start with a line that begins with an ampersamd "&".  However, only the
last such line counts as such, so if you have title lines that begin
with "&", just append an ampersand on a line by itself as a dummy
alignment section if you don't have one anyway.

What follows the ampersand on its line is the alignment style (like
I<left>, I<right>, ... as described in L<"Alignment">), you want for
the data in this column.  If nothing follows, the general default I<auto>
is used.  If you specify an invalid alignment style, it falls back to
left alignment.

The lines that follow can contain sample data for this column.  These
are considered for alignment in the column, but never actually appear
in the output.  The effect is to guarantee a minimum width for the
column even if the current data doesn't require it.  This helps dampen
the oscillations in the appearance of dynamically aligned tables.

=item Columns as Hashes

The format is

    {
        title   => $title,
        align   => $align,
        sample  => $sample,
        align_title => $align_title,
        align_title_lines => $align_title_lines,
    }

$title contains the title lines and $sample the sample data.  Both can
be given as a string or as an array-ref to the list of lines.  $align contains
the alignment style (without a leading ampersand), usually as a string.
You can also give a regular expression here, which specifies regex alignment.
A regex can only be specified in the hash form of a colunm specification.

In hash form you can also specify how the title of a column is aligned
with its body.  To do this, you specify the keyword C<align_title> with
C<left>, C<right> or C<center>.  Other alignment specifications are not
valid here.  The default is C<left>.

C<align_title> also specifies how the lines of a multiline title are
aligned among themselves.  If you want a different alignment, you
can specify it with the key C<align_title_lines>.  Again, only C<left>,
C<right> or C<center> are allowed.

Do not put other keys than those mentioned above (I<title>, I<align>,
I<align_title>, I<align_title_lines>, and I<sample>) into a hash that
specifies a column.  Most would be ignored, but some would confuse the
interpreter (in particular, I<is_sep> has to be avoided).

=item Separators as strings

A separator must be given as a reference to a string (often a literal,
like C<\' | '>), any string that is given directly describes a column.

It is usually just a (short) string that will be printed between
table columns on all table lines instead of the default single
blank.  If you specify two separators (on two lines), the first one
will be used in the title and the other in the body of the table.

=item Separators as Hashes

The hash representation of a separator has the format

    {
        is_sep => 1,
        title  => $title,
        body   => $body,
    }

$title is the separator to be used in the title area and $body
the one for the body.  If only one is given, the other is used for
both.  If none is given, a blank is used.  If one is shorter than
the other, it is blank filled on the right.

The value of C<is_sep> must be set to a true value, this is the
distinguishing feature of a separator.

=back

=head2 Alignment

The original documentation to L<Text::Aligner> contains all the details
on alignment specification, but here is the rundown:

The possible alignment specifications are I<left>, I<right>, I<center>,
I<num> and I<point> (which are synonyms), and I<auto>.  The first
three explain themselves.

I<num> (and I<point>) align the decimal point in the data, which is
assumed to the right if none is present.  Strings that aren't
numbers are treated the same way, that is, they appear aligned
with the integers unless they contain a ".".  Instead of the
decimal point ".", you can also specify any other string in
the form I<num(,)>, for instance.  The string in parentheses
is aligned in the data.  The synonym I<point> for I<num> may be
more appropriate in contexts that deal with arbitrary
strings, as in I<point(=E<gt>)> (which might be used to align certain
bits of Perl code).

I<regex alignment> is a more sophisticated form of point alignment.
If you specify a regular expression, as delivered by C<qr//>, the start
of the match is used as the alignment point.  If the regex contains
capturing parentheses, the last submatch counts.  [The usefulness of
this feature is under consideration.]

I<auto> alignment combines numeric alignment with left alignment.
Data items that look like numbers, and those that don't, form two
virtual columns and are aligned accordingly: C<num> for numbers and
C<left> for other strings.  These columns are left-aligned with
each other (i.e. the narrower one is blank-filled) to form the 
final alignment.

This way, a column that happens to have only numbers in the data gets
I<num> alignment, a column with no numbers appears I<left>-aligned,
and mixed data is presented in a reasonable way.

=head2 Column Selection

Besides creating tables from scratch, they can be created by
selecting columns from an existing table.  Tables created this
way contain the data from the columns they were built from.

This is done by specifying the columns to select by their index
(where negative indices count backward from the last column).
The same column can be selected more than once and the sequence
of columns can be arbitrarily changed.  Separators don't travel
with columns, but can be specified between the columns at selection
time.

You can make the selection of one or more columns dependent on
the data content of one of them.  If you specify some of the columns
in angle brackets [...], the whole group is only included in the
selection if the first column in the group contains any data that
evaluates to boolean true.  That way you can de-select parts of a
table if it contains no interesting data.  Any column separators
given in brackets are selected or deselected along with the rest
of it.

=head1 PUBLIC METHODS

=head2 Table Creation

=over 4

=item new()

    my $tb = Text::Table->new( $column, ... );

creates a table with the columns specified.  A column can be proper column
which contains and displays data, or a separator which tells how to fill
the space between columns.  The format of the parameters is described under
L<"Column Specification">. Specifying an invalid alignment for a column
results in a warning if these are allowed.

If no columns are specified, the number of columns is taken from the first
line of data added to the table.  The effect is as if you had specified
C<Text::Table-E<gt>new( ( '') x $n)>, where C<$n> is the number of
columns.

=item select()

    my $sub = $tb->select( $column, ...);

creates a table from the listed columns of the table $tb, including
the data.  Columns are specified as integer indices which refer to
the data columns of $tb.  Columns can be repeated and specified in any
order.  Negative indices count from the last column.  If an invalid
index is specified, a warning is issued, if allowed.

As with L<"new()">, separators can be interspersed among the column
indices and will be used between the columns of the new table.

If you enclose some of the arguments (column indices or separators) in
angle brackets C<[...]> (technically, you specify them inside an
arrayref), they form a group for conditional selection.  The group is
only included in the resulting table if the first actual column inside
the group contains any data that evaluate to a boolean true.  This way
you can exclude groups of columns that wouldn't contribute anything
interesting.  Note that separators are selected and de-selected with
their group.  That way, more than one separator can appear between
adjacent columns.  They don't add up, but only the rightmost separator
is used.  A group that contains only separators is never selected.
[Another feature whose usefulness is under consideration.]

=back

=head2 Status Information

=over 4

=item n_cols()

    $tb->n_cols

returns the number of columns in the table.

=item width()

    $tb->width

returns the width (in characters) of the table.  All table lines have
this length (not counting a final "\n" in the line), as well as the
separator lines returned by $tb->rule() and $b->body_rule().
The width of a table can potentially be influenced by any data item
in it.

=item height()

    $tb->height

returns the total number of lines in a table, including title lines
and body lines. For orthogonality, the synonym table_height() also
exists.

=item title_height()

    $tb->title_height

returns the number of title lines in a table.

=item body_height()

    $tb->body_height

returns the number of lines in the table body.

=item colrange()

    $tb->colrange( $i)

returns the start position and width of the $i-th column (counting from 0)
of the table.  If $i is negative, counts from the end of the table.  If $i
is larger than the greatest column index, an imaginary column of width 0
is assumed right of the table.

=back

=head2 Data Loading

=over 4

=item add()

    $tb->add( $col1, ..., $colN)

adds a data line to the table, returns the table.

C<$col1>, ..., C<$colN> are scalars that
correspond to the table columns.  Undefined entries are converted to '',
and extra data beyond the number of table columns is ignored.

Data entries can be multi-line strings.  The partial strings all go into
the same column.  The corresponding fields of other columns remain empty
unless there is another multi-line entry in that column that fills the
fieds.  Adding a line with multi-line entries is equivalent to adding
multiple lines.

Every call to C<add()> increases the body height of the table by the
number of effective lines, one in the absence of multiline entries.

=item load()

    $tb->load( $line, ...)

loads the data lines given into the table, returns the table.

Every argument to C<load()> represents a data line to be added to the
table.  The line can be given as an array(ref) containing the data
items, or as a string, which is split on whitespace to retrieve the
data.  If an undefined argument is given, it is treated as an
empty line.

=item clear()

    $tb->clear;

deletes all data from the table and resets it to the state after
creation.  Returns the table.  The body height of a table is 0 after
C<clear()>.

=back

=head2 Table Output

The three methods C<table()>, C<title()>, and C<body()> are very similar.
They access different parts of the printable output lines of a table with
similar methods.  The details are described with the C<table()> method.

=over 4

=item table()

The C<table()> method returns lines from the entire table, starting
with the first title line and ending with the last body line.

In array context, the lines are returned separately, in scalar context
they are joined together in a single string.

    my @lines = $tb->table;
    my $line  = $tb->table( $line_number);
    my @lines = $tb->table( $line_number, $n);

The first call returns all the lines in the table.  The second call
returns one line given by $line_number.  The third call returns $n
lines, starting with $line_number.  If $line_number is negative, it
counts from the end of the array.  Unlike the C<select()> method,
C<table()> (and its sister methods C<title()> and C<body()>) is
protected against large negative line numbers, it truncates the
range described by $line_number and $n to the existing lines.  If
$n is 0 or negative, no lines are returned (an empty string in scalar
context).

=item title()

Returns lines from the title area of a table, where the column titles
are rendered.  Parameters and response to context are as with C<table()>,
but no lines are returned from outside the title area.

=item body()

Returns lines from the body area of a table, that is the part where
the data content is rendered, so that $tb->body( 0) is the first data
line.  Parameters and response to context are as with C<table()>.

=item rule()

    $tb->rule;
    $tb->rule( $char);
    $tb->rule( $char, $char1);

Returns a rule for the table.

A rule is a line of table width that can be used between table lines
to provide visual horizontal divisions, much like column separators
provide vertical visual divisions.  In its basic form (returned by the
first call) it looks like a table line with no data, hence a blank
line except for the non-blank parts of any column-separators.  If
one character is specified (the second call), it replaces the blanks
in the first form, but non-blank column separators are retained.  If
a second character is specified, it replaces the non-blank parts of
the separators.  So specifying the same character twice gives a solid
line of table width.  Another useful combo is C<$tb-E<lt>rule( '-', '+')>,
together with separators that contain a single nonblank "|", for a
popular representation of line crossings.

C<rule()> uses the column separators for the title section if there
is a difference.

=item body_rule()

C<body_rule()> works like <rule()>, except the rule is generated using
the column separators for the table body.

=back

=head2 Warning Control

=over 4

=item warnings()

    Text::Table->warnings();
    Text::Table->warnings( 'on');
    Text::Table->warnings( 'off'):
    Text::Table->warnings( 'fatal'):

The C<warnings()> method is used to control the appearance of warning
messages while tables are manipulated.  When Text::Table starts, warnings
are disabled.  The default action of C<warnings()> is to turn warnings
on.  The other possible arguments are self-explanatory.  C<warnings()>
can also be called as an object method (C<$tb-E<gt>warnings( ...)>).

=back

=head1 VERSION
    
This document pertains to Text::Table version 1.107

=head1 BUGS

=over 4

=item o

I<auto> alignment doesn't support alternative characters for the decimal
point.  This is actually a bug in the underlying Text::Aligner by the
same author.

=back

=head1 AUTHOR

    Anno Siegel
    CPAN ID: ANNO
    siegel@zrz.tu-berlin.de
    http://www.tu-berlin.de/~siegel

=head1 COPYRIGHT

Copyright (c) 2002 Anno Siegel. All rights reserved.
This program is free software; you can redistribute
it and/or modify it under the same terms as Perl itself.

The full text of the license can be found in the
LICENSE file included with this module.

=head1 SEE ALSO

Text::Aligner, perl(1).

=cut

1;
