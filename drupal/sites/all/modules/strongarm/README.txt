
Strongarm 2.x for Drupal 6.x
----------------------------
Strongarm gives site builders a way to override the default variable values that
Drupal core and contributed modules ship with. It is not an end user tool, but a
developer and site builder tool which provides an API and a limited UI.

An example of such a variable is `site_frontpage`. In Drupal this defaults to
`node`, which ensures that the front page gets content as soon as some exists,
but for many Drupal sites this setting is simply wrong. Strongarm gives the site
builder a place in the equation - an opportunity to set the default value of
`site_frontpage` to something that makes sense for their site.


Installation
------------
Strongarm can be installed like any other Drupal module -- place it in
the modules directory for your site and enable it (and its requirement,
CTools) on the `admin/build/modules` page.

Strongarm is an API module. It does absolutely nothing for the end user out of
the box without other modules that take advantage of its API.


How Strongarm works
-------------------
Strongarm uses the CTools export API to make entries in the system module's
`variable` table exportables. Exportables are Drupal configuration objects that
lead a dual life -- they may be *defaults* set by code exports in modules, they
may be *overridden* if a user chooses to change the value in the database, or
they may be *normal* if the configuration object lives only in the database but
not in code. To learn more about exportables and the CTools export API, see the
CTools advanced help on "Exportable objects tool."


Exporting variables
-------------------
If you are a developer or site builder Strongarm gives you tools to export the
settings of variables in your site database and manage any overrides to default
values.

To export variable values, you will need to enable either the [Features][1]
module or the [Bulk Export][2] module provided by CTools (as of June 29, 2010
you must use a recent checkout CTools `DRUPAL-6--1` in order to use Bulk Export
with Strongarm). Features provides a UI for adding variable exports to a
feature. Bulk Export provides a UI for generating defaults hooks with exported
variables that you can add to your own modules. You do not need to enable both
modules.


Maintainers
-----------

- jmiccolis (Jeff Miccolis)
- yhahn (Young Hahn)


[1]: http://drupal.org/project/features
[2]: http://drupal.org/project/ctools
