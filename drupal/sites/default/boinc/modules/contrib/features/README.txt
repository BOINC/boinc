
Features 1.x for Drupal 6.x
---------------------------
The features module enables the capture and management of features in Drupal. A
feature is a collection of Drupal entities which taken together satisfy a
certain use-case.

Features provides a UI and API for taking different site building components
from modules with exportables and bundling them together in a single feature
module. A feature module is like any other Drupal module except that it declares
its components (e.g. views, contexts, CCK fields, etc.) in its `.info` file so
that it can be checked, updated, or reverted programmatically.

Examples of features might be:

- A blog
- A pressroom
- An image gallery
- An e-commerce t-shirt store


Installation
------------
Features can be installed like any other Drupal module -- place it in the
modules directory for your site and enable it on the `admin/build/modules` page.
To take full advantage of some of the workflow benefits provided by Features,
you should install [Drush][1].


Basic usage
-----------
Features is geared toward usage by developers and site builders. It
is not intended to be used by the general audience of your Drupal site.
Features provides tools for accomplishing two important tasks:

### Task 1: Export features

You can build features in Drupal by using site building tools that are supported
(see a short list under the *Compatibility* section).

Once you've built and configured functionality on a site, you can export it into
a feature module by using the feature create page at
`admin/build/features/create`.


### Task 2: Manage features

The features module also provides a way to manage features through a more
targeted interface than `admin/build/modules`. The interface at
`admin/build/features` shows you only feature modules, and will also inform you
if any of their components have been overridden. If this is the case, you can
also re-create features to bring the module code up to date with any changes
that have occurred in the database.


Including custom code and adding to your feature
------------------------------------------------
Once you've exported your feature you will see that you have several files:

    myfeature.info
    myfeature.module
    myfeature.[*].inc

You can add custom code (e.g. custom hook implementations, other functionality,
etc.) to your feature in `myfeature.module` as you would with any other module.
Do not change or add to any of the features `.inc` files unless you know what
you are doing. These files are written to by features on updates so any custom
changes may be overwritten.


Using Features to manage development
------------------------------------
Because Features provides a centralized way to manage exportable components and
write them to code it can be used during development in conjunction with a
version control like SVN or git as a way to manage changes between development,
staging and production sites. An example workflow for a developer using Features
is to:

1. Make configuration changes to a feature on her local development site.
2. Update her local feature codebase using `drush features-update`.
3. Commit those changes using `svn commit`.
4. Roll out her changes to the development site codebase by running `svn update`
  on the server. Other collaborating developers can also get her changes with
  `svn update`.
5. Reverting any configuration on the staging site to match the updated codebase
by running `drush features-revert`.
6. Rinse, repeat.

Features also provides integration with the [Diff][3] module if enabled to show
differences between configuration in the database and that in code. For site
builders interested in using Features for development, enabling the diff module
and reading `API.txt` for more details on the inner workings of Features is
highly recommended.


Drush usage
-----------
Features provides several useful drush commands:

- `drush features`

  List all the available features on your site and their status.

- `drush features-export [feature name] [component list]`

  Write a new feature in code containing the components listed.

- `drush features-update [feature name]`

  Update the code of an existing feature to include any overrides/changes in
  your database (e.g. a new view).

- `drush features-revert [feature name]`

  Revert the components of a feature in your site's database to the state
  described in your feature module's defaults.

- `drush features-diff [feature name]`

  Show a diff between a feature's database components and those in code.
  Requires the Diff module.

Additional commands and options can be found using `drush help`.


Compatibility
-------------
Features provides integration for the following exportables:

- CTools export API implementers (Context, Spaces, Boxes, Strongarm, Page
  Manager)
- ImageCache
- Views
- [Other contributed modules][2]

Features also provides faux-exportable functionality for the following Drupal
core and contrib components:

- CCK fields
- CCK fieldgroups
- Content types
- Input filters
- User roles/permissions
- Custom menus and menu links *
- Taxonomy vocabularies *

* Currently in development.


For developers
--------------
Please read `API.txt` for more information about the concepts and integration
points in the Features module.


Maintainers
-----------
- yhahn (Young Hahn)
- jmiccolis (Jeff Miccolis)


[1]: http://drupal.org/project/drush
[2]: (http://drupal.org/taxonomy/term/11478)
