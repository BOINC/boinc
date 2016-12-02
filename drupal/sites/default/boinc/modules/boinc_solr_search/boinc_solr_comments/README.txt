INTRODUCTION
------------

The boinc_solr_comments module will index the comments of a BOINC
project's Web site. The apachesolr module is a requirement as this
module uses hooks from apachesolr to perform its job.

REQUIREMENTS
------------

This module requires the following modules:

 * apachesolr (https://www.drupal.org/project/apachesolr)
   - Use version family 6.x-3.x

INSTALLATION
------------

 * Install as you would normally install a contributed Drupal module.
 * Enable the module normally.

CONFIGURATION
-------------

 * Configure this module in Administration » Site configuration »
   Apache Solr search » Index Comments

Admins should choose what content (node) types shall be have their
comments indexed by Apache Solr. By default, all node types are
selected for indexing. Use the check-boxes to select content types.

Every time this module is (re)configured, the Apache Solr Index needs
to be completely re-indexed. Delete the current index and re-index the
site.

DESCRIPTION
-----------

Quick summary of apachesolr module: For every node that is created,
the apachesolr module creates an object of type
ApacheSolrDocument. These ApacheSolrDocuments contain multiple fields:
such as 'content', node’s main content, and importantly for this
description 'ts_comments', the node’s comments. For example, for a
forum topic the 'content' is the forum post that starts the thread. All
comments are flattened into 'ts_comments'.

What this module does is to load each node’s document and remove the
'ts_comments' field. Then it loops over all comments attached to the
node and creates a separate ApacheSolrDocument objects for each
one. In this way a forum topic with 6 comments is separated into 7
ApacheSolrDocuments (1 forum topic + 6 comments) for indexing into
Solr.

In addition, the module will detect when a node or comment has been
deleted, and inform Solr of that change.

FAQ
---

Q: When I re-index the site, why does the number of documents Apache
   Solr reports not match? e.g., '50 items successfully processed. 122
   documents successfully sent to Solr.'

A: This is normal behavior for this module. Without this module
   installed, each node is a separate Apache Solr Document. The
   apachesolr module will report the number of "items successfully
   processed", which corresponds to the number of nodes. This is 50 in
   the example above.

   However, this module will create a separate Document for each
   comment. Thus if there are 72 comments to be indexed, this module
   will create 72 Documents, which with total the 122 documents shown
   in the example above.
