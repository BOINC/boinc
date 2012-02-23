<?php
// $Id: flag.views_bookmark.inc,v 1.1.2.8 2009/02/10 04:27:35 quicksketch Exp $

/**
 * @file
 * Upgrade code to migrate from Views bookmark to Flag module.
 */

/**
 * Determine if Views Bookmark needs to be updated.
 */
function flag_views_bookmark_update_needed() {
  return db_table_exists('views_bookmarks');
}

/**
 * Perform all pieces of the update.
 */
function flag_views_bookmark_update() {
  drupal_set_message(t('Migrating Views Bookmark to Flag module. More information about the changes between Views Bookmark and Flag may be found on the <a href="http://drupal.org/node/268269">upgrading handbook page</a>.'));

  // Update Views Bookmark and Remove current Flag tables.
  flag_views_bookmark_update_prepare();
  // Migrate the tables.
  flag_views_bookmark_update_tables();
  // Uninstall Views Bookmark.
  flag_views_bookmark_update_uninstall();
  // Update views.
  // In Drupal 6, this is handled in flag.views_convert.inc.

  drupal_set_message(t('The Flag module has been installed, replacing Views Bookmark. Views Bookmark has been disabled, please remove it from your installation.'));
}

/**
 * Prepare for the Views Bookmark to Flag migration.
 *
 * Uninstall Flag module if needed.
 * Bring Views Bookmark up to the latest schema.
 */
function flag_views_bookmark_update_prepare() {
  // Ensure the Flag tables are not installed.
  if (db_table_exists('flags')) {
    // Remove Flag tables if already present.
    flag_uninstall();
  }

  // Update Views Bookmark to the latest schema.
  $vb_schema = drupal_get_installed_schema_version('views_bookmark');
  $ret = array();
  if (!empty($vb_schema) && $vb_schema <= 5102) {
    for ($version = $vb_schema + 1; $version <= 5102; $version++) {
      $update_function = 'flag_views_bookmark_update_'. $version;
      if (function_exists($update_function)) {
        $ret += $update_function();
      }
    }
  }

}

/**
 * Update Views Bookmark to the Flag schema.
 */
function flag_views_bookmark_update_tables() {
  include_once './includes/install.inc';

  $ret = array();

  // Drop all keys.
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP PRIMARY KEY");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodes} DROP PRIMARY KEY");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_node_count} DROP PRIMARY KEY");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodetypes} DROP PRIMARY KEY");

  // Rename columns.
  $ret[] = update_sql("ALTER TABLE {views_bookmark_node_count} CHANGE COLUMN vbid fid smallint unsigned NOT NULL default '0'");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_node_count} CHANGE COLUMN nid content_id int unsigned NOT NULL default '0'");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodes} CHANGE COLUMN vbid fid smallint unsigned NOT NULL default '0'");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodes} CHANGE COLUMN nid content_id int unsigned NOT NULL default '0'");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodes} CHANGE COLUMN uid uid int unsigned NOT NULL default '0' AFTER content_id");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodetypes} CHANGE COLUMN vbid fid smallint unsigned NOT NULL default '0'");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodetypes} CHANGE COLUMN type type varchar(32) NOT NULL default ''");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} CHANGE COLUMN vbid fid smallint unsigned NOT NULL default '0'");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} CHANGE COLUMN global global tinyint default 0");

  // Add columns.
  $ret[] = update_sql("ALTER TABLE {views_bookmark_node_count} ADD content_type varchar(32) default '' AFTER fid");
  $ret[] = update_sql("ALTER TABLE {views_bookmark_nodes} ADD content_type varchar(32) default '' AFTER fid");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} ADD content_type varchar(32) default '' AFTER fid");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} ADD name varchar(32) default '' AFTER content_type");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} ADD options text default NULL");

  // Move options.
  $result = db_query("SELECT * FROM {views_bookmarks}");
  while ($bookmark = db_fetch_object($result)) {
    $options = array(
      'flag_short' => $bookmark->mark,
      'flag_long' => $bookmark->mark_long,
      'flag_message' => $bookmark->mark_message,
      'unflag_short' => $bookmark->unmark,
      'unflag_long' => $bookmark->unmark_long,
      'unflag_message' => $bookmark->unmark_message,
      'show_on_form' => $bookmark->show_on_form,
      'show_on_teaser' => $bookmark->teaser,
      'show_on_page' => 1,
    );
    db_query("UPDATE {views_bookmarks} SET options = '%s' WHERE fid = %d", serialize($options), $bookmark->fid);
  }

  // Delete columns.
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN mark");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN mark_long");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN mark_message");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN unmark");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN unmark_long");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN unmark_message");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN show_on_form");
  $ret[] = update_sql("ALTER TABLE {views_bookmarks} DROP COLUMN teaser");

  // Rename all tables.
  $ret[] = update_sql("RENAME TABLE {views_bookmarks} TO {flags}");
  $ret[] = update_sql("RENAME TABLE {views_bookmark_nodes} TO {flag_content}");
  $ret[] = update_sql("RENAME TABLE {views_bookmark_node_count} TO {flag_counts}");
  $ret[] = update_sql("RENAME TABLE {views_bookmark_nodetypes} TO {flag_types}");

  // Set defaults.
  $ret[] = update_sql("UPDATE {flag_content} SET content_type = 'node'");
  $ret[] = update_sql("UPDATE {flag_counts} SET content_type = 'node'");
  $ret[] = update_sql("UPDATE {flags} SET content_type = 'node'");

  // Create primary keys.
  $ret[] = update_sql("ALTER TABLE {flags} ADD PRIMARY KEY (fid)");
  $ret[] = update_sql("ALTER TABLE {flag_types} ADD INDEX (fid)");
  $ret[] = update_sql("ALTER TABLE {flag_counts} ADD PRIMARY KEY (fid, content_type, content_id)");
  $ret[] = update_sql("ALTER TABLE {flag_content} ADD fcid int unsigned NOT NULL auto_increment FIRST, ADD PRIMARY KEY (fcid)");
  $ret[] = update_sql("ALTER TABLE {flag_content} ADD UNIQUE INDEX fid_content_type_content_id_uid (fid, content_type, content_id, uid)");
  $ret[] = update_sql("ALTER TABLE {flag_content} ADD INDEX content_type_content_id (content_type, content_id)");
  $ret[] = update_sql("ALTER TABLE {flag_content} ADD INDEX content_type_uid (content_type, uid)");

  // Change fid to a serial column.
  $ret[] = update_sql("ALTER TABLE {flags} CHANGE COLUMN fid fid int(10) unsigned NOT NULL auto_increment");

  // Give each flag a "name" attribute.
  $result = db_query("SELECT fid, title FROM {flags}");
  while ($row = db_fetch_object($result)) {
    $name = substr(preg_replace('/[^a-z_]/', '', str_replace(' ', '_', drupal_strtolower(trim($row->title)))), 0, 32);
    $ret[] = update_sql("UPDATE {flags} SET name = '". $name ."' WHERE fid = ". $row->fid);
    drupal_set_message(t('The views bookmark %bookmark has been migrated to Flag. It was given the machine-name %name, which you may change on the <a href="!url">%bookmark configuration form</a>.', array('%bookmark' => $row->title, '%name' => $name, '!url' => url('admin/build/flags/edit/'. $name))));
  }
  // The key for flag name must be added after giving names.
  $ret[] = update_sql("ALTER TABLE {flags} ADD UNIQUE KEY (name)");

  return $ret;
}

/**
 * Finishing tasks.
 */
function flag_views_bookmark_update_uninstall() {
  // Uninstall Views Bookmark.
  module_disable(array('views_bookmark'));
  drupal_set_installed_schema_version('views_bookmark', SCHEMA_UNINSTALLED);
}

if (!function_exists('update_sql')) {
  function update_sql($sql) {
    $result = db_query($sql);
    return array('success' => $result !== FALSE, 'query' => check_plain($sql));
  }
}

/**
 * Replace keys with primary keys.
 */
function flag_views_bookmark_update_1() {
  $ret = array();
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $ret[] = update_sql('ALTER TABLE {views_bookmarks} DROP INDEX vbid');
      $ret[] = update_sql('ALTER TABLE {views_bookmarks} ADD PRIMARY KEY (vbid)');
      $ret[] = update_sql('ALTER TABLE {views_bookmark_nodes} DROP INDEX nid');
      $ret[] = update_sql('ALTER TABLE {views_bookmark_nodes} ADD PRIMARY KEY (vbid, uid, nid)');
      $ret[] = update_sql('ALTER TABLE {views_bookmark_nodetypes} ADD PRIMARY KEY (vbid, type)');
    break;
  }
  return $ret;
}

/*
 * Add count table if it doesn't exist.
 */
function flag_views_bookmark_update_2() {
  $ret = array();
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
    $ret[] = update_sql("CREATE TABLE if not exists {views_bookmark_node_count} (
                  vbid int(10) unsigned NOT NULL default '0',
                  nid int(10) unsigned NOT NULL default '0',
                  count int(10) unsigned NOT NULL default '0',
                  PRIMARY KEY (vbid, nid)
                  ) /*!40100 DEFAULT CHARACTER SET utf8 */;");
      break;
  }
  return $ret;
}

function flag_views_bookmark_update_3() {
  $ret = array();
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $result = db_query('SELECT vbid, nid, count(uid) as count FROM {views_bookmark_nodes} GROUP BY vbid, nid');
      while ($row = db_fetch_object($result)) {
        $ret[] = update_sql("INSERT INTO {views_bookmark_node_count} VALUES ($row->vbid, $row->nid, $row->count)");
      }
      break;
  }
  return $ret;
}

function flag_views_bookmark_update_4() {
  $ret = array();
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $ret[] = update_sql("ALTER TABLE {views_bookmarks} ADD show_on_form int(1) unsigned NOT NULL default '0'");
      break;
  }
  return $ret;
}

/*
 * Add a column to track last added timestamp
 */
function flag_views_bookmark_update_5() {
  $ret = array();
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $timestamp = time();
      $ret[] = update_sql("ALTER TABLE {views_bookmark_nodes} ADD timestamp int(11) unsigned NOT NULL default '0'");
      $ret[] = update_sql("UPDATE {views_bookmark_nodes} SET timestamp = $timestamp WHERE timestamp = 0");
      break;
  }
  return $ret;
}

/**
 * Add columns for configurable messages.
 */
function flag_views_bookmark_update_6() {
  $ret = array();
  switch ($GLOBALS['db_type']) {
    case 'mysql':
    case 'mysqli':
      $ret[] = update_sql("ALTER TABLE {views_bookmarks} ADD mark_message varchar(255) default '' AFTER mark_long");
      $ret[] = update_sql("ALTER TABLE {views_bookmarks} ADD unmark_message varchar(255) default '' AFTER unmark_long");
      break;
  }
  return $ret;
}

/**
 * Add the current user filter to every view using flag filters.
 * Previously this was implied with all filters, now it is optional to increase
 * the flexibility of flag.
 *
 * For some reason Flag began providing a Node: Node ID argument in
 * the 5.x port. Because this argument should be handled by Views' own node.inc
 * file, update existing arguments to use that one instead.
 */
function flag_views_bookmark_update_7() {
  // Code from "Views Bookmark" for Drupal 5, which uses Views 1.x API, removed.
  return array();
}

function flag_views_bookmark_update_5100() {
  // (code moved to 5101.)
  return array();
}

/**
 * Update all views that use the 'views_bookmark_nodes_N.uid' field to use
 * the 'views_bookmark_users_N.name' one. See http://drupal.org/node/211112
 *
 * Update all views that use the 'views_bookmark_nodes_N.vbid' field to use
 * the 'views_bookmark_ops_N.ops' one. See http://drupal.org/node/213488
 */
function flag_views_bookmark_update_5101() {
  // Code from "Views Bookmark" for Drupal 5, which uses Views 1.x API, removed.
  return array();
}

/**
 * Handle issues http://drupal.org/node/218766 and http://drupal.org/node/219146
 */
function flag_views_bookmark_update_5102() {
  // Because of a certain FAPI trait, when the admin didn't selected any
  // roles on the bookmark configuration form it's as if he selected the
  // 'anonymous role'. We clear this bogus selection.
  //
  // (Note: update_sql() doesn't support %placeholders, so we embed the value
  //  directly.)
  $ret[] = update_sql("UPDATE {views_bookmarks} SET roles = '' WHERE roles = '". DRUPAL_ANONYMOUS_RID ."'");

  // We're now making the 'roles' field a required one. If no roles were
  // previously selected, we assign this bookmark to all authenticated users.
  $ret[] = update_sql("UPDATE {views_bookmarks} SET roles = '". DRUPAL_AUTHENTICATED_RID ."' WHERE roles = ''");

  return $ret;
}
