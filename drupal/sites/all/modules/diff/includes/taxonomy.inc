<?php

/**
 * @file
 * Implementation of hook_diff() for taxonomy.
 */

/**
 * Implementation of hook_diff() for taxonomy.
 */
function taxonomy_diff($old_node, $new_node) {
  $result = array();
  // TODO: make taxonomy by category not only by whole taxonomy?
  $old_taxonomy = array();
  $new_taxonomy = array();
  if (isset($old_node->taxonomy) && $old_node->taxonomy) {
    foreach ($old_node->taxonomy as $term) {
      $old_taxonomy[] = $term->name;
    }
  }
  if (isset($new_node->taxonomy) && $new_node->taxonomy) {
    foreach ($new_node->taxonomy as $id => $entry) {
      if (is_array($entry)) {
        // During editing the taxonomy is built up as a list of vocabulary ids as keys
        // and a list of term ids per array entry.
        if (is_numeric($id)) {
          foreach ($entry as $tid) {
            $term = taxonomy_get_term($tid);
            $new_taxonomy[] = $term->name;
          }
        }
        else {
          // If the id is not numeric than it has to be 'tags' which denotes freetagging
          // vocabularies. These are stored as an array which map the vocabulary id to
          // a string of terms.
          foreach ($entry as $taglist) {
            // The regular expression is taken from taxonomy.module.
            preg_match_all('%(?:^|,\ *)("(?>[^"]*)(?>""[^"]* )*"|(?: [^",]*))%x', $taglist, $matches);
            foreach ($matches[1] as $term) {
              $new_taxonomy[] = $term;
            }
          }
        }
      }
      elseif (is_numeric($entry)) {
        // During editing, taxonomy is a single id.
        $term = taxonomy_get_term($entry);
        $new_taxonomy[] = $term->name;
      }
      else {
        // Not during editing the taxonomy list is a list of terms.
        $new_taxonomy[] = $entry->name;
      }
    }
  }
  $result['taxonomy'] = array(
    '#name' => t('Taxonomy'),
    '#old' => $old_taxonomy,
    '#new' => $new_taxonomy,
    '#weight' => -3,
    '#format' => array(
      'show_header' => FALSE,
    )
  );
  return $result;
}
