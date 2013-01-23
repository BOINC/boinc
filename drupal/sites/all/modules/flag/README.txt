
You may want to visit the handbook of this module, at:

  http://drupal.org/handbook/modules/flag

The Flag module is a flexible flagging system whose primary goal is
to give all the control to the administrator. Using this module, the
site administrator can provide an arbitrary number of 'flags'.

A flag is really just a boolean toggle that is set on a node, comment,
or user. Flags may be per-user, meaning that each user can flag an item
individually, or global, meaning that the item is either flagged or it
is not flagged, and any user who changes that changes it for everyone.

In this way, additional flags (similar to 'published' and 'sticky') can 
be put on nodes, or other items, and dealt with by the system however 
the administration likes.

Each flag allows the administrator to choose the 'flag this' text, and
the place where the user interface for flagging the item will appear
(For example: for nodes, whether a flagging link appears on the node
teaser as well on the full node view).

Each flag can be restricted to use only by certain roles. Each
flag provides data to the Views module, and provides a default
view to list 'My bookmarks'. These default views are somewhat crude,
but are easily tailored to whatever the system administrator would like
it to do. 

Each flag also provides an 'argument' to the Views module that can be 
used to allow a user to view other people's flagged content. This isn't 
turned on by default anywhere, though, and the administrator will need 
to construct a view in order to take advantage of it.

The module will come installed with a simple flag called "bookmarks" and 
a simple view for 'My bookmarks'. This is a default view provided by the 
Flag module, but can be customized to fit the needs of your site. To 
customize this view, go to admin/build/views and find the 
'flags_bookmarks' view. Click the 'Add' action to customize the view. 
Once saved, the new version of the view will be used rather than the one 
provided by Flag.

Besides editing the default view that comes with the module, Flag
provides many views filters, fields, and sort criteria to make all sorts of
displays possible relating to the number of times an item has been flagged.

This module was formerly known as Views Bookmark, which was originally was
written by Earl Miles. Flag was written by Nathan Haug and mystery man Mooffie.

This module built by robots: http://www.lullabot.com

Recommended Modules
-------------------
- Views
- Session API

Installation
------------
1) Copy the flag directory to the modules folder in your installation.

2) Enable the module using Administer -> Modules (/admin/build/modules)

Optional Installation
---------------------
1) The ability for anonymous users to flag content is provided by the Session
   API module, available at http://drupal.org/project/session_api.

Configuration
-------------
The configuration for Flag is spread between Views configuration
and the Flag site building page. To configure:

1) Configure the flags for your site at
   Administer -> Site Building -> Flags (/admin/build/flags)

   You can create and edit flags on this page. Descriptions for the various
   options are provided below each field on the flag edit form.

2) Go to the Views building pages at
   Administer -> Site Building -> Views (/admin/build/views)

   A default view is provided to get you started organizing your flags. You
   can override the view or use it as a template to control the display of your
   flags.

Support
-------
If you experience a problem with flag or have a problem, file a
request or issue on the flag queue at
http://drupal.org/project/issues/flag. DO NOT POST IN THE FORUMS.
Posting in the issue queues is a direct line of communication with the module
authors.

No guarantee is provided with this software, no matter how critical your
information, module authors are not responsible for damage caused by this
software or obligated in any way to correct problems you may experience.

Licensed under the GPL 2.0.
http://www.gnu.org/licenses/gpl-2.0.txt

