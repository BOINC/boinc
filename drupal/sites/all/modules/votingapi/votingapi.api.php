<?php
// $Id: votingapi.api.php,v 1.1.2.1 2009/06/24 18:57:26 eaton Exp $

/**
 * @file
 * Provides hook documentation for the VotingAPI module.
 */


/**
 * Adds to or changes the calculated vote results for a piece of content.
 *
 * VotingAPI calculates a number of common aggregate functions automatically,
 * including the average vote and total number of votes cast. Results are grouped
 * by 'tag', 'value_type', and then 'function' in the following format:
 * 
 *   $results[$tag][$value_type][$aggregate_function] = $value;
 *
 * If no custom tag is being used for votes, the catch-all "vote" tag should be
 * used. In cases where custom tags are used to vote on different aspects of a
 * piece of content, a catch-all "vote" value should still be calculated for use
 * on summary screens, etc.
 *
 * @param $results
 *   An alterable array of aggregatre vote results.
 * @param $content_type
 *   A string identifying the type of content being rated. Node, comment,
 *   aggregator item, etc.
 * @param $content_id
 *   The key ID of the content being rated.
 *
 * @see votingapi_recalculate_results()
 */
function hook_votingapi_results_alter(&$results, $content_type, $content_id) {
  // We're using a MySQLism (STDDEV isn't ANSI SQL), but it's OK because this is
  // an example. And no one would ever base real code on sample code. Ever. Never.
  
  $sql  = "SELECT v.tag, STDDEV(v.value) as standard_deviation ";
  $sql .= "FROM {votingapi_vote} v ";
  $sql .= "WHERE v.content_type = '%s' AND v.content_id = %d AND v.value_type = 'percent' ";
  $sql .= "GROUP BY v.tag";

  $results = db_query($sql, $content_type, $content_id);

  // VotingAPI wants the data in the following format:
  // $cache[$tag][$value_type][$aggregate_function] = $value;
  
  while ($result = db_fetch_array($results)) {
    $cache[$result['tag']]['percent']['standard_deviation'] = $result['standard_deviation'];
  }
}


/**
 * Adds to or alters metadata describing Voting tags, value_types, and functions.
 *
 * If your module uses custom tags or value_types, or calculates custom aggregate
 * functions, please implement this hook so other modules can properly interperet
 * and display your data.
 *
 * Three major bins of data are stored: tags, value_types, and aggregate result
 * functions. Each entry in these bins is keyed by the value stored in the actual
 * VotingAPI tables, and contains an array with (minimally) 'name' and
 * 'description' keys. Modules can add extra keys to their entries if desired.
 *
 * @param $data
 *   An alterable array of aggregate vote results.
 *
 * @see votingapi_metadata()
 */
function hook_votingapi_metadata_alter(&$data) {
  // Document several custom tags for rating restaurants and meals.
  $data['tags']['bread'] = array(
    'name' => t('Bread'),
    'description' => t('The quality of the food at a restaurant.'),
    'module' => 'mymodule', // This is optional; we can add it for our own purposes.
  );
  $data['tags']['circuses'] = array(
    'name' => t('Circuses'),
    'description' => t('The quality of the presentation and atmosphere at a restaurant.'),
    'module' => 'mymodule',
  );
  
  // Document two custom aggregate function. 
  $data['functions']['standard_deviation'] = array(
    'name' => t('Standard deviation'),
    'description' => t('The standard deviation of all votes cast on a given piece of content. Use this to find controversial content.'),
    'module' => 'mymodule',
  );
  $data['functions']['median'] = array(
    'name' => t('Median vote'),
    'description' => t('The median vote value cast on a given piece of content. More accurate than a pure average when there are a few outlying votes.'),
    'module' => 'mymodule',
  );
}

/**
 * Return metadata used to build Views relationships on voting data.
 *
 * VotingAPI can store votes on any entity in the Drupal database: its content_type
 * and content_id columns can be used to store "node"/1, "comment"/2, and so
 * on. This hook is used to tell VotingAPI what Views base table the content_type
 * field corresponds to, and what field in that base table contains the value in
 * votingapi's content_id table.
 *
 * @return
 *   An array of records containing 'description', 'content_type', 'base_table',
 *   and 'content_id_column' entries.
 */
function hook_votingapi_relationships() {
  $relationships[] = array(
    // 'description' is used to construct the field description in the Views UI.
    'description' => t('users'),
    
    // 'content_type' contain the value that your module stores in the voting
    // api 'content_type' column. 'node', 'comment', etc.
    'content_type' => 'user',
    
    // 'base_table' contain the name of the Views base table that stores the
    // data your votes apply to.
    'base_table' => 'user',
    
    // 'content_id_column' contains the name of the views field that represents
    // your base_table's primary key. This column will be joined against the
    // voting api 'content_id' column.
    'content_id_column' => 'uid',
    
    // VotingAPI constructs pseudo-tables so that multiple relationships can
    // point to the same base table (normal and translation-based votes nodes
    // for example. These two columns allow you to override the names of the
    // pseudo-tables. You probably don't need to change this part unless you're
    // nedjo.
    'pseudo_vote' => 'votingapi_vote_special',
    'pseudo_cache' => 'votingapi_cache_special',
  );
}



/**
 * Returns callback functions and descriptions to format a VotingAPI Views field.
 *
 * Loads all votes for a given piece of content, then calculates and caches the
 * aggregate vote results. This is only intended for modules that have assumed
 * responsibility for the full voting cycle: the votingapi_set_vote() function
 * recalculates automatically.
 *
 * @param $field
 *   A Views field object. This can be used to expose formatters only for tags,
 *   vote values, aggregate functions, etc.
 * @return
 *   An array of key-value pairs, in which each key is a callback function and
 *   each value is a human-readable description of the formatter.
 *
 * @see votingapi_set_votes()
 */
function hook_votingapi_views_formatters($field) {
  if ($field->field == 'value') {
    return array('mymodule_funky_formatter' => t('MyModule value formatter'));
  }
  if ($field->field == 'tag') {
    return array('mymodule_funky_tags' => t('MyModule tag formatter'));
  }
}
