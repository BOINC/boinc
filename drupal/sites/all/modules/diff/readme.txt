
Diff 2.x for Drupal 6.x
-----------------------
Diff enhances usage of node revisions by adding the following features:

- diff between node revisions on the 'Revisions' tab to view all the changes
  between any two revisions of a node
- highlight changes inline while viewing a node to quickly see color-coded
  additions, changes, and deletions
- preview changes as a diff before updating a node


Installation
------------
Diff can be installed like any other Drupal module -- place it in
the modules directory for your site and enable it on the `admin/build/modules`
page.

Diff needs to be configured to be used with specific node types on your site.
Enable any of diff's options on a content type's settings page (e.g.
`admin/content/node-type/page`).


Technical
---------
- Diff compares the raw data, not the filtered output, making it easier to see
changes to HTML entities, etc.
- The diff engine itself is a GPL'ed php diff engine from phpwiki.

API
---
This module offers `hook_diff()` which modules may use to inject their changes
into the presentation of the diff. For example, this is used by
`content.diff.inc` (see CCK), `upload.inc`, and `taxonomy.inc`.

Maintainers
-----------
- dww (Derek Wright)
- moshe (Moshe Weitzman)
- rötzi (Julian)
- yhahn (Young Hahn)
