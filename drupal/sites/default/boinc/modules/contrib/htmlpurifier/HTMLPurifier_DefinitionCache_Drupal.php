<?php

require_once 'HTMLPurifier/DefinitionCache.php';

/**
 * Cache handler that stores all data in drupals builtin cache
 */
class HTMLPurifier_DefinitionCache_Drupal extends HTMLPurifier_DefinitionCache
{
  /**
   * Add an object to the cache without overwriting
   */
  function add($def, $config) {
    if (!$this->checkDefType($def)) return;
    $key = $this->generateKey($config);

    if ($this->fetchFromDrupalCache($key)) {
      // already cached
      return false;
    }
    $this->storeInDrupalCache($def, $key);
    return true;
  }

  /**
   * Unconditionally add an object to the cache, overwrites any existing object. 
   */
  function set($def, $config) {
    if (!$this->checkDefType($def)) return;
    $key = $this->generateKey($config);

    $this->storeInDrupalCache($def, $key);
    return true;
  }

  /**
   * Replace an object that already exists in the cache.
   */
  function replace($def, $config) {
    if (!$this->checkDefType($def)) return;
    $key = $this->generateKey($config);

    if (!$this->fetchFromDrupalCache($key)) {
      // object does not exist in cache
      return false;
    }

    $this->storeInDrupalCache($def, $key);
    return true;
  }

  /**
   * Retrieve an object from the cache 
   */
  function get($config) {
    $key = $this->generateKey($config);
    return $this->fetchFromDrupalCache($key);
  }

  /**
   * Delete an object from the cache 
   */
  function remove($config) {
    $key = $this->generateKey($config);
    cache_clear_all("htmlpurifier:$key", 'cache');
    return true;
  }

  function flush($config) {
    cache_clear_all("htmlpurifier:*", 'cache', true);
    return true;
  }

  function cleanup($config) {
    $res = db_query("SELECT cid FROM {cache} WHERE cid LIKE '%s%%'", 'htmlpurifier:');
    while ($row = db_fetch_object($res)) {
      $key = substr($row->cid, 13); // 13 == strlen('htmlpurifier:')
      if ($this->isOld($key, $config)) {
        cache_clear_all($row->cid, 'cache');
      }
    }
  }

  function fetchFromDrupalCache($key) {
    $cached = cache_get("htmlpurifier:$key");
    if ($cached) return unserialize($cached->data);
    return false;
  }

  function storeInDrupalCache($def, $key) {
    cache_set("htmlpurifier:$key", serialize($def), 'cache', CACHE_PERMANENT);
  }
  
}

