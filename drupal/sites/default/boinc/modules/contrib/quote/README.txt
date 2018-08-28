Quote.module
------------

This module adds a 'quote' link below nodes and comments. When clicked, the 
contents of the node or comment are placed into a new comment enclosed with 
special markup that indicates it is quoted material. 

This module also features a filter which translates the special markup into html
code.

When output by Drupal, the quote will be displayed with special formatting to 
indicate the material has been quoted.


Installation
------------

The "quote.module" and "quote.css" files should be uploaded to "modules/quote/".

Quote.module must be enabled via the 'administer/modules' interface. 


Filter
------

The Quote filter should be activated for each input format that you want to have
it available (input filters are edited via the 'administer/filters' interface).

For best effect, the Quote filter must be applied *after* any filters that 
replace HTML, and *before* the Linebreak filter. Or conversely, if
HTML filters consider <div> tags to be valid, the quote filter can be placed
before them. Filters can be rearranged by using the weight selectors within the
'rearrange filters' tab.

Additionally, the Quote filter must be applied *before* the BBCode filter if you
have the optional bbcode.module installed.

As the quote filter accesses the node (being quoted) directly, any content 
within will be displayed without any processing. For example, if a user is 
quoting a page node containing PHP (which by default is evaluated by the PHP 
filter) or any other sensitive code, it is quoted as is without the PHP (or any
other) filter being applied. While the PHP code is never evaluated in a comment,
it is nevertheless possible that sensitive server side information is made 
available to the public. To avoid this situation, quote links can be 
enabled/disabled for the parent node via the settings page. This does not affect
comment quote links.

Settings
--------

The Quote module can be configured further through its settings page at
admin/quote. This page allows the filter to be associated with specific node 
types, control if the quote link is displayed for nodes (as opposed to comments)
and so on.

Format
------

Quoted content can be placed between [quote] tags in order to be displayed as a
quote:

[quote]This is a simple quote.[/quote]

There is an optional attribute which allows quotes to cite the author:

[quote=author's name]This is a quote with an attribution line.[/quote]


Theme
-----

There are two css rules located in "quote.css" which can be altered to change
the display of the quotes.

'quote-msg' controls the display of the quote content.
'quote-author' controls the display of the attribution line.

The default "quote.css" rules are designed for Drupal's default Bluemarine 
theme. By default, quoted content is placed into an indented box, which has a 
light gray background.

Alternatively, the rules from "quote.css" can be copied into your theme's
"style.css" files. If you do this, remember to remove "quote.css" from the
"modules/quote/" folder.


Project navigation
------------------

Quote module settings page: admin -> quote.
Filter management: admin -> input formats

Project URL: http://drupal.org/project/quote
