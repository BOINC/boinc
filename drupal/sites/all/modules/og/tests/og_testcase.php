<?php
// $Id: og_testcase.php,v 1.10.4.1 2009/04/09 21:14:22 weitzman Exp $

/**
 * @file
 * Common functions for all the organic groups tests.
 */

class OgTestCase extends DrupalWebTestCase {
  /**
   * Create a group node.
   *
   * @param $type
   *   The content type name.
   * @param $selective
   *   The group's visibility (e.g. open, moderated, etc').
   * @return
   *   The newly created node id.
   */
  function addOgGroup($type, $selective = OG_OPEN) {
  	$edit = array();
    $edit['og_description'] = $this->randomName(16);
    $edit['og_selective'] = $selective;

    // Keys that should be present when the node is loaded.
    $keys = array(
      'og_selective',
      'og_description',
      'og_theme',
      'og_register',
      'og_directory',
      'og_language',
      'og_private',
    );
    $og_type = t('Group node');
    return $this->_addOgContent($type, $og_type, $edit, $keys);   	
  }

  /**
   * Create a group post.
   *
   * @param $type
   *   The content type name.
   * @param $groups
   *   An array with the group(s) id the post should belong to.
   * @return
   *   The newly created node id.
   */
  function addOgPost($type, $groups = array()) {
    $edit = array();
    foreach ($groups as $gid) {
      $edit['og_groups['. $gid .']'] = TRUE;
    }

    // Keys that should be present when the node is loaded.
    $keys = array(
      'og_groups',
      'og_groups_both',
    );
    $og_type = t('Group post');
    return $this->_addOgContent($type, $og_type, $edit, $keys);
  }

  /**
   * Helper function - create a group content.
   *
   * @param $type
   *   The type name of the content type.
   * @param $og_type
   *   The og type - group or post.
   * @param $edit
   *   An array of settings to add to the defaults.
   * @param $keys
   *   An array with the keys that need to be present in the $node object
   *   after node_load().
   * @return
   *   The newly created node id.
   */
  function _addOgContent($type, $og_type, $edit = array(), $keys = array()) {
    $edit['title'] = $this->randomName(8);
    $edit['body']= $this->randomName(32);
    $type_hyphen = str_replace('_', '-', $type);

    $this->drupalPost('node/add/'. $type_hyphen, $edit, t('Save'));

    // Check that the form has been submitted.
    $this->assertRaw(t('!type %title has been created.', array('!type' => $type, '%title' => $edit['title'])), t('%og_type created.', array('%og_type' => $og_type)));

    // Assert the node has loaded properly.
    $node = node_load(array('title' => $edit['title']));
    $node = (array)$node;
    $this->assertTrue($this->assertKeysExist($keys, $node), t('%og_type loaded properly.', array('%og_type' => $og_type)));
    // Node was casted to an array.
    return $node['nid'];
  }

  /**
   * Assert keys are in an array.
   *
   * @param $keys
   *   An array of keys that needs to be checked.
   * @param $array
   *   The array that has the keys.
   * @return
   *   True if all keys exist.
   */
  function assertKeysExist($keys, $array) {
    foreach ($keys as $key) {
      if (!array_key_exists($key, $array)) {
        return FALSE;
      }
    }
    return TRUE;
  }
}