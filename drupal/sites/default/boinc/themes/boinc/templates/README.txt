// $Id: README.txt,v 1.3.2.1 2010/01/12 16:32:29 johnalbin Exp $

TEMPLATES
---------

Drupal 6 contains the following template files which you can override and modify
by copying them to your sub-theme. The Zen theme overrides a handful of Drupal's
templates. In order to override those templates, you should copy them from the
zen/templates folder to your sub-theme's templates folder.

As always, when adding a new template file to your sub-theme, you will need to
rebuild the "theme registry" in order for Drupal to see it. For more info, see:
  http://drupal.org/node/173880#theme-registry

Located in zen/templates:
  page.tpl.php
  maintenance-page.tpl.php
  node.tpl.php
  block.tpl.php
  comment-wrapper.tpl.php
  comment.tpl.php
  views-view.tpl.php

Located in /modules/aggregator:
  aggregator-feed-source.tpl.php
  aggregator-item.tpl.php
  aggregator-summary-item.tpl.php
  aggregator-summary-items.tpl.php
  aggregator-wrapper.tpl.php

Located in /modules/block:
  block-admin-display-form.tpl.php

Located in /modules/book:
  book-all-books-block.tpl.php
  book-export-html.tpl.php
  book-navigation.tpl.php
  book-node-export-html.tpl.php

Located in /modules/comment:
  comment-folded.tpl.php
  comment-wrapper.tpl.php  (overridden by Zen)
  comment.tpl.php  (overridden by Zen)

Located in /modules/forum:
  forum-icon.tpl.php
  forum-list.tpl.php
  forum-submitted.tpl.php
  forum-topic-list.tpl.php
  forum-topic-navigation.tpl.php
  forums.tpl.php

Located in /modules/node:
  node.tpl.php  (overridden by Zen)

Located in /modules/poll:
  poll-bar-block.tpl.php
  poll-bar.tpl.php
  poll-results-block.tpl.php
  poll-results.tpl.php
  poll-vote.tpl.php

Located in /modules/profile:
  profile-block.tpl.php
  profile-listing.tpl.php
  profile-wrapper.tpl.php

Located in /modules/search:
  search-block-form.tpl.php
  search-result.tpl.php
  search-results.tpl.php
  search-theme-form.tpl.php

Located in /modules/system:
  block.tpl.php  (overridden by Zen)
  box.tpl.php
  maintenance-page.tpl.php  (overridden by Zen)
  page.tpl.php  (overridden by Zen)

Located in /modules/user:
  user-picture.tpl.php
  user-profile-category.tpl.php
  user-profile-item.tpl.php
  user-profile.tpl.php
