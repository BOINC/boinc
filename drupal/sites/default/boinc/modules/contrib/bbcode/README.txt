Drupal bbcode.module README.txt
==============================================================================

The Drupal bbcode.module adds a BBCode filter to Drupal. This allows you
to use HTML-like tags as an alternative to HTML itself for adding markup
to your posts. BBCode is easier to use than HTML and helps to prevent
malicious users from disrupting your site's formatting.

See the help screen of the module (or the code) for information on which
tags and variants are supported. This implementation is not necessarily the
same as the original BBCode implementaion.
 
Note that this filter also recognizes and converts URLs and email addresses
to links automatically.

Installation
------------------------------------------------------------------------------
 
  - Download the BBCode module from http://drupal.org/project/bbcode

  - Create a .../modules/bbcode/ subdirectory and copy the files into it.

  - Enable the module as usual from Drupal's admin pages 
    (Administer » Modules)
 
Configuration
------------------------------------------------------------------------------

  - Before using BBCode you need to enable the BBCode filter in an input
    format (see Administer » Input formats » add input format)

  - You can enable/ disable the following features in the configuration page
    of the input format in which BBCode is enabled:

    * Convert web and email addresses to links
    * Javascript encoding of emails
    * Smart paragraph and line breaks
    * Print debugging info

  - If you've disabled "smart paragraph and line breaks", you need to enable
    Drupal's "Line break converter" with the BBCode filter. Don't use both
    together!

  - If you would like to use BBCode as a replacement for HTML, you could
    enable Drupal's "HTML filter" to remove or escape user entered HTML tags.

  - If you've enabled multiple filters, you may need to rearrange them to
    ensure they execute in the correct order. For example, if HTML filtering 
    is enabled, it is essential that BBCode be sorted AFTER the HTML filter. 
    If not this module will change the BBCode into HTML, and the HTML filter 
    will disallow and remove the code again.

Complementing Modules
------------------------------------------------------------------------------

The following optional modules may be used to enhance your BBCode 
installation:

  - Quicktags - http://drupal.org/project/quicktags
    Adds a BBCode formatting bar above all your textareas. 

  - Smileys module - http://drupal.org/project/smileys

Note: these are independent projects. Please do not report issues with them 
as BBCode problems!

Additional tags:
------------------------------------------------------------------------------

Here are some tags that's not part of the official BBCode implementation. 
You may want to add them to your bbcode-filter.inc file:

  - '#\[move(?::\w+)?\](.*?)\[/move(?::\w+)?\]#si' => '<marquee>\\1</marquee>',
  - '#\[mp3\](.*?)\[/mp3(?::\w+)?\]#si' => '<swf file="\\1">',

Credits / Contacts
------------------------------------------------------------------------------

  - The original author of this module is Alastair Maw, who can be reached at
    drupal-bbcode[at]almaw.com. 

  - Gabor Hojtsy (goba[at]php.net) also contributed to the module.

  - Javascript encoding of emails by László Bácsi (lackac[at]math.bme.hu).

  - Frank Naude converted this module to Drupal 4.7 and 5.0, added several
    BBCode tags, a linebreak converter, quicktags integration, etc.

TODO List
------------------------------------------------------------------------------

 - Translate this module into other languages.

 - Fix non-compliant HTML when "Smart paragraph and line breaks:" is set to 
   "Line and paragraph breaks". If HTML validation is important to you, use 
   one of the other options or submit a patch.

 - Configuration of which BBCode tags are allowed 
   (will require a complete rewrite).

