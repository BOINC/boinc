$Id: README.txt,v 1.3 2009/05/24 06:06:00 darrenoh Exp $

Views Or allows Views to combine filters with OR and fields with COALESCE.


Combining filters or arguments with OR
--------------------------------------

By default, each record in a view must match all filters. Sometimes records need
to be included if they match one or more filters in a list of alternatives.
Views has the ability to combine filters only with AND. If one filter is false,
the combined list will be false. Views Or adds the ability to combine blocks of
filters with OR. If one of the filters in the block is true, the combined block
will be true.

Views Or provides three filters:

  - Begin alternatives

    Begins a block of alternative filters.

  - Next alternative

    Separates alternative filters in an alternatives block.

  - End alternatives

    Ends a block of alternative filters.

Insert these filters between other filters to create alternatives. For example,
the following sequence of filters

  - Views Or: Begin alternatives =
  - Taxonomy: Term ID = Carrot
  - Views Or: Next alternative =
  - Taxonomy: Term ID = Elephant
  - Views Or: End alternatives =

is equivalent to the following logical expression:

  (Taxonomy: Term ID = Carrot) OR (Taxonomy: Term ID = Elephant)

Here is a more advanced example. The following sequence of filters

  - Node: Published True
  - Views Or: Begin alternatives =
  - Taxonomy: Term ID = Carrot
  - Node: Type = Forum topic
  - Views Or: Next alternative =
  - Taxonomy: Term ID = Elephant
  - Node: Type = Story
  - Views Or: End alternatives =
  - Node: Post date >= -3 weeks

is equivalent to the following logical expression:

  (Node: Published True) AND (
    (Taxonomy: Term ID = Carrot AND Node: Type = Forum topic)
    OR
    (Taxonomy: Term ID = Elephant AND Node: Type = Story)
  ) AND (Node: Post date >= -3 weeks)

Arguments can be combined in the same way as filters. By default, when arguments
are combined, the same arguments will be used for both sets of alternatives. So
if there are two alternative arguments in a block, the view needs to receive
only one argument. Uncheck "Share arguments" in the "Views Or: Next alternative"
argument to allow each set of alternatives receive different arguments from the
URL.


Combining fields with COALESCE
------------------------------

When using relationships in a view, the same field may appear multiple times.
Views Or adds the ability to combine multiple fields into one using the COALESCE
function. The value of the combined field will be taken from the first field
which contains a value.

Views Or provides two fields:

  - Begin alternatives

    Begins a group of alternative fields.

  - End alternatives

    Ends a group of alternative fields.

Insert other fields between these fields to create a combined field. The title
and settings of the first field in the group will be used for the combined
field, even if another field contains the value.
