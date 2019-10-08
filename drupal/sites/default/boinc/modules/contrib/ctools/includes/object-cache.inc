<?php

/**
 * @file
 * The non-volatile object cache is used to store an object while it is
 * being edited, so that we don't have to save until we're completely
 * done. The cache should be 'cleaned' on a regular basis, meaning to
 * remove old objects from the cache, but otherwise the data in this
 * cache must remain stable, as it includes unsaved changes.
 */

/**
 * Get an object from the non-volatile ctools cache.
 *
 * This function caches in memory as well, so that multiple calls to this
 * will not result in multiple database reads.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the object being stored.
 * @param $skip_cache
 *   Skip the memory cache, meaning this must be read from the db again.
 *
 * @return
 *   The data that was cached.
 */
function ctools_object_cache_get($obj, $name, $skip_cache = FALSE) {
  $cache = &ctools_static(__FUNCTION__, array());
  $key = "$obj:$name";
  if ($skip_cache) {
    unset($cache[$key]);
  }

  if (!array_key_exists($key, $cache)) {
    $data = db_fetch_object(db_query("SELECT * FROM {ctools_object_cache} WHERE sid = '%s' AND obj = '%s' AND name = '%s'", session_id(), $obj, $name));
    if ($data) {
      $cache[$key] = unserialize(db_decode_blob($data->data));
    }
  }
  return isset($cache[$key]) ? $cache[$key] : NULL;
}

/**
 * Store an object in the non-volatile ctools cache.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the object being stored.
 * @param $cache
 *   The object to be cached. This will be serialized prior to writing.
 */
function ctools_object_cache_set($obj, $name, $cache) {
  // Store the CTools session id in the user session to force a
  // session for anonymous users in Drupal 7 and Drupal 6 Pressflow.
  // see http://drupal.org/node/562374, http://drupal.org/node/861778
  if (empty($GLOBALS['user']->uid) && empty($_SESSION['ctools_session_id'])) {
    $_SESSION['ctools_hold_session'] = TRUE;
  }

  ctools_object_cache_clear($obj, $name);
  db_query("INSERT INTO {ctools_object_cache} (sid, obj, name, data, updated) VALUES ('%s', '%s', '%s', %b, %d)", session_id(), $obj, $name, serialize($cache), time());
}

/**
 * Remove an object from the non-volatile ctools cache
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the object being removed.
 */
function ctools_object_cache_clear($obj, $name) {
  db_query("DELETE FROM {ctools_object_cache} WHERE sid = '%s' AND obj = '%s' AND name = '%s'", session_id(), $obj, $name);
  // Ensure the static cache is emptied of this obj:name set.
  $cache = &ctools_static('ctools_object_cache_get', array());
  unset($cache["$obj:$name"]);
}


/**
 * Determine if another user has a given object cached.
 *
 * This is very useful for 'locking' objects so that only one user can
 * modify them.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the object being removed.
 *
 * @return
 *   An object containing the UID and updated date if found; NULL if not.
 */
function ctools_object_cache_test($obj, $name) {
  return db_fetch_object(db_query("SELECT s.uid, c.updated FROM {ctools_object_cache} c INNER JOIN {sessions}  s ON c.sid = s.sid WHERE s.sid != '%s' AND c.obj = '%s' AND c.name = '%s' ORDER BY c.updated ASC", session_id(), $obj, $name));
}

/**
 * Get the cache status of a group of objects.
 *
 * This is useful for displaying lock status when listing a number of objects
 * an an administration UI.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $names
 *   An array of names of objects
 *
 * @return
 *   An array of objects containing the UID and updated date for each name found.
 */
function ctools_object_cache_test_objects($obj, $names) {
  $placeholders = db_placeholders($names, 'varchar');
  $args = array_merge(array($obj), $names);
  $result = db_query("SELECT c.name, s.uid, c.updated FROM {ctools_object_cache} c INNER JOIN {sessions}  s ON c.sid = s.sid WHERE c.obj = '%s' AND c.name IN ($placeholders) ORDER BY c.updated ASC", $args);

  $return = array();
  while ($test = db_fetch_object($result)) {
    $return[$test->name] = $test;
  }

  return $return;
}

/**
 * Remove an object from the non-volatile ctools cache for all session IDs.
 *
 * This is useful for clearing a lock.
 *
 * @param $obj
 *   A 32 character or less string to define what kind of object is being
 *   stored; primarily this is used to prevent collisions.
 * @param $name
 *   The name of the object being removed.
 */
function ctools_object_cache_clear_all($obj, $name) {
  db_query("DELETE FROM {ctools_object_cache} WHERE obj = '%s' AND name = '%s'", $obj, $name);
  // Ensure the static cache is emptied of this obj:name set.
  $cache = &ctools_static('ctools_object_cache_get', array());
  unset($cache["$obj:$name"]);
}

/**
 * Remove all objects in the object cache that are older than the
 * specified age.
 *
 * @param $age
 *   The minimum age of objects to remove, in seconds. For example, 86400 is
 *   one day. Defaults to 7 days.
 */
function ctools_object_cache_clean($age = NULL) {
  if (empty($age)) {
    $age = 86400 * 7; // 7 days
  }
  db_query("DELETE FROM {ctools_object_cache} WHERE updated < %d", time() - $age);
}
