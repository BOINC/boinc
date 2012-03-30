<?php

/**
 * @file
 * Miscellaneous functions for Pathauto.
 *
 * @ingroup pathauto
 */

/**
 * Check to see if there is already an alias pointing to a different item.
 *
 * @param $alias
 *   A string alias.
 * @param $source
 *   A string that is the internal path.
 * @param $language
 *   A string indicating the path's language.
 * @return
 *   TRUE if an alias exists, FALSE if not.
 */
function _pathauto_alias_exists($alias, $source, $language = '') {
  $pid = db_result(db_query_range("SELECT pid FROM {url_alias} WHERE src <> '%s' AND dst = '%s' AND language IN ('%s', '') ORDER BY language DESC, pid DESC", $source, $alias, $language, 0, 1));

  if (module_exists('path_redirect') && function_exists('path_redirect_delete_multiple')) {
    // Delete from path_redirect the exact same alias to the same node.
    path_redirect_delete_multiple(NULL, array('source' => $alias, 'redirect' => $source));

    // If there still is this alias used in path_redirect, then create a different alias.
    $redirects = path_redirect_load_multiple(NULL, array('source' => $alias));
  }

  if ($pid || !empty($redirects)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Fetches an existing URL alias given a path and optional language.
 *
 * @param $source
 *   An internal Drupal path.
 * @param $language
 *   An optional language code to look up the path in.
 * @return
 *   FALSE if no alias was found or an associative array containing the
 *   following keys:
 *   - pid: Unique path alias identifier.
 *   - alias: The URL alias.
 */
function _pathauto_existing_alias_data($source, $language = '') {
  return db_fetch_array(db_query_range("SELECT pid, dst AS alias, language FROM {url_alias} WHERE src = '%s' AND language IN ('%s', '') ORDER BY language DESC, pid DESC", $source, $language, 0, 1));
}

/**
 * Clean up a string segment to be used in an URL alias.
 *
 * Performs the following possible alterations:
 * - Remove all HTML tags.
 * - Process the string through the transliteration module.
 * - Replace or remove punctuation with the separator character.
 * - Remove back-slashes.
 * - Replace non-ascii and non-numeric characters with the separator.
 * - Remove common words.
 * - Replace whitespace with the separator character.
 * - Trim duplicate, leading, and trailing separators.
 * - Convert to lower-case.
 * - Shorten to a desired length and logical position based on word boundaries.
 *
 * This function should *not* be called on URL alias or path strings because it
 * is assumed that they are already clean.
 *
 * @param $string
 *   A string to clean.
 * @return
 *   The cleaned string.
 */
function pathauto_cleanstring($string) {
  static $strings = array();

  // Empty strings do not need any proccessing.
  if ($string === '' || $string === NULL) {
    return '';
  }

  // Check if the string has already been processed, and if so return the
  // cached result.
  if (isset($strings[$string])) {
    return $strings[$string];
  }

  // Remove all HTML tags from the string.
  $output = strip_tags(decode_entities($string));

  // Optionally remove accents and transliterate
  if (variable_get('pathauto_transliterate', FALSE)) {
    static $translations;

    if (!isset($translations)) {
      $translations = FALSE;
      if ($file = _pathauto_get_i18n_file()) {
        $translations = parse_ini_file($file);
      }
    }

    if (!empty($translations)) {
      $output = strtr($output, $translations);
    }
  }

  // Replace or drop punctuation based on user settings
  $separator = variable_get('pathauto_separator', '-');
  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $action = variable_get('pathauto_punctuation_'. $name, 0);
    // 2 is the action for "do nothing" with the punctuation
    if ($action != 2) {
      // Slightly tricky inline if which either replaces with the separator or nothing
      $output = str_replace($details['value'], ($action ? $separator : ''), $output);
    }
  }

  // Reduce strings to letters and numbers
  if (variable_get('pathauto_reduce_ascii', FALSE)) {
    $pattern = '/[^a-zA-Z0-9\/]+/';
    $output = preg_replace($pattern, $separator, $output);
  }

  // Calculate and statically cache the ignored words regex expression.
  static $ignore_words_regex;
  if (!isset($ignore_words_regex)) {
    $ignore_words = array(
      'a', 'an', 'as', 'at', 'before', 'but', 'by', 'for', 'from', 'is', 'in',
      'into', 'like', 'of', 'off', 'on', 'onto', 'per', 'since', 'than', 'the',
      'this', 'that', 'to', 'up', 'via', 'with',
    );
    $ignore_words = variable_get('pathauto_ignore_words', $ignore_words);
    $ignore_words_regex = preg_replace(array('/^[,\s]+|[,\s]+$/', '/[,\s]+/'), array('', '\b|\b'), $ignore_words);
    if ($ignore_words_regex) {
      $ignore_words_regex = '\b' . $ignore_words_regex . '\b';
    }
  }

  // Get rid of words that are on the ignore list
  if ($ignore_words_regex) {
    if (function_exists('mb_eregi_replace')) {
      $words_removed = mb_eregi_replace($ignore_words_regex, '', $output);
    }
    else {
      $words_removed = preg_replace("/$ignore_words_regex/i", '', $output);
    }
    if (drupal_strlen(trim($words_removed)) > 0) {
      $output = $words_removed;
    }
  }

  // Always replace whitespace with the separator.
  $output = preg_replace('/\s+/', $separator, $output);

  // Trim duplicates and remove trailing and leading separators.
  $output = _pathauto_clean_separators($output);

  // Optionally convert to lower case.
  if (variable_get('pathauto_case', 1)) {
    $output = drupal_strtolower($output);
  }

  // Enforce the maximum component length.
  $maxlength = min(variable_get('pathauto_max_component_length', 100), _pathauto_get_schema_alias_maxlength());
  $output = drupal_substr($output, 0, $maxlength);

  // Cache this result in the static array.
  $strings[$string] = $output;

  return $output;
}

/**
 * Trims duplicate, leading, and trailing separators from a string.
 *
 * @param $string
 *   The string to clean path separators from.
 * @param $separator
 *   The path separator to use when cleaning.
 * @return
 *   The cleaned version of the string.
 *
 * @see pathauto_cleanstring()
 * @see pathauto_clean_alias()
 */
function _pathauto_clean_separators($string, $separator = NULL) {
  $output = $string;

  if (!isset($separator)) {
    $separator = variable_get('pathauto_separator', '-');
  }

  // Clean duplicate or trailing separators.
  if (strlen($separator)) {
    // Escape the separator.
    $seppattern = preg_quote($separator, '/');

    // Trim any leading or trailing separators.
    $output = preg_replace("/^$seppattern+|$seppattern+$/", '', $output);

    // Replace trailing separators around slashes.
    if ($separator !== '/') {
      $output = preg_replace("/$seppattern+\/|\/$seppattern+/", "/", $output);
    }

    // Replace multiple separators with a single one.
    $output = preg_replace("/$seppattern+/", $separator, $output);
  }

  return $output;
}

/**
 * Clean up an URL alias.
 *
 * Performs the following alterations:
 * - Trim duplicate, leading, and trailing back-slashes.
 * - Trim duplicate, leading, and trailing separators.
 * - Shorten to a desired length and logical position based on word boundaries.
 *
 * @param $alias
 *   A string with the URL alias to clean up.
 * @return
 *   The cleaned URL alias.
 */
function pathauto_clean_alias($alias) {
  $output = $alias;

  // Trim duplicate, leading, and trailing back-slashes.
  $output = _pathauto_clean_separators($output, '/');

  // Trim duplicate, leading, and trailing separators.
  $output = _pathauto_clean_separators($output);

  // Enforce the maximum length.
  $separator = variable_get('pathauto_separator', '-');
  $maxlength = min(variable_get('pathauto_max_length', 100), _pathauto_get_schema_alias_maxlength());
  $output = drupal_substr($output, 0, $maxlength);

  return $output;
}

/**
 * Apply patterns to create an alias.
 *
 * @param $module
 *   The name of your module (e.g., 'node').
 * @param $op
 *   Operation being performed on the content being aliased
 *   ('insert', 'update', 'return', or 'bulkupdate').
 * @param $placeholders
 *   An array whose keys consist of the translated placeholders
 *   which appear in patterns (e.g., t('[title]')) and values are the
 *   actual values to be substituted into the pattern (e.g., $node->title).
 * @param $source
 *   An internal Drupal path to be aliased.
 * @param $entity_id
 *   The entity ID (node ID, user ID, etc.).
 * @param $type
 *   For modules which provided pattern items in hook_pathauto(),
 *   the relevant identifier for the specific item to be aliased
 *   (e.g., $node->type).
 * @param $language
 *   A string specify the path's language.
 * @return
 *   The alias that was created.
 *
 * @see _pathauto_set_alias()
 * @see pathauto_get_placeholders()
 */
function pathauto_create_alias($module, $op, $placeholders, $source, $entity_id, $type = NULL, $language = '') {
  // Retrieve and apply the pattern for this content type
  if (!empty($type)) {
    $pattern = trim(variable_get("pathauto_{$module}_{$type}_{$language}_pattern", ''));
    if (empty($pattern)) {
      $pattern = trim(variable_get("pathauto_{$module}_{$type}_pattern", ''));
    }
  }
  if (empty($pattern)) {
    $pattern = trim(variable_get("pathauto_{$module}_pattern", ''));
  }
  // No pattern? Do nothing (otherwise we may blow away existing aliases...)
  if (empty($pattern)) {
    return '';
  }

  if ($module == 'taxonomy') {
    // Get proper path for term.
    $term_path = taxonomy_term_path(taxonomy_get_term($entity_id));
    if ($term_path != $source) {
      // Quietly alias 'taxonomy/term/[tid]' with proper path for term.
      $update_data = _pathauto_existing_alias_data($source, $language);
      _pathauto_set_alias($source, $term_path, $module, $entity_id, $update_data['pid'], FALSE, $update_data['old_alias'], $language);
      // Set $source as proper path.
      $source = $term_path;
    }
  }

  // Special handling when updating an item which is already aliased.
  $existing_alias = NULL;
  if ($op == 'update' || $op == 'bulkupdate') {
    if ($existing_alias = _pathauto_existing_alias_data($source, $language)) {
      switch (variable_get('pathauto_update_action', 2)) {
        case 0:
          // If an alias already exists, and the update action is set to do nothing,
          // then gosh-darn it, do nothing.
          return '';
      }
    }
  }

  // Replace the placeholders with the values provided by the module.
  $alias = str_replace($placeholders['tokens'], $placeholders['values'], $pattern);

  // Check if the token replacement has not actually replaced any values. If
  // that is the case, then stop because we should not generate an alias.
  // @see token_scan()
  $pattern_tokens_removed = preg_replace('/\[([^\s]+?)\]/', '', $pattern);
  if ($alias === $pattern_tokens_removed) {
    return '';
  }

  $alias = pathauto_clean_alias($alias);

  // Allow other modules to alter the alias.
  $context = array(
    'module' => $module,
    'op' => $op,
    'source' => $source,
    'entity_id' => $entity_id,
    'type' => $type,
    'language' => $language,
    'pattern' => $pattern,
  );
  drupal_alter('pathauto_alias', $alias, $context);

  // If we have arrived at an empty string, discontinue.
  if (!drupal_strlen($alias)) {
    return '';
  }

  // If the alias already exists, generate a new, hopefully unique, variant
  if (_pathauto_alias_exists($alias, $source, $language)) {
    $maxlength = min(variable_get('pathauto_max_length', 100), _pathauto_get_schema_alias_maxlength());
    $separator = variable_get('pathauto_separator', '-');
    $original_alias = $alias;

    $i = 0;
    do {
      // Append an incrementing numeric suffix until we find a unique alias.
      $unique_suffix = $separator . $i;
      $alias = drupal_substr($original_alias, 0, $maxlength - drupal_strlen($unique_suffix, TRUE)) . $unique_suffix;
      $i++;
    } while (_pathauto_alias_exists($alias, $source, $language));

    // Alert the user why this happened.
    _pathauto_verbose(t('The automatically generated alias %original_alias conflicted with an existing alias. Alias changed to %alias.', array(
      '%original_alias' => $original_alias,
      '%alias' => $alias,
    )), $op);
  }

  // Return the generated alias if requested.
  if ($op == 'return') {
    return $alias;
  }

  // Build the new path alias array and send it off to be created.
  $path = array(
    'source' => $source,
    'alias' => $alias,
    'language' => $language,
  );
  $success = _pathauto_set_alias($path, $existing_alias, $op);

  // Also create a related feed alias if requested and supported.
  $feedappend = trim(variable_get('pathauto_' . $module . '_applytofeeds', ''));
  if (drupal_strlen($feedappend)) {
    // For forums and taxonomies, the source doesn't always form the base of the RSS feed (i.e. image galleries)
    if ($module == 'taxonomy' || $module == 'forum' && !empty($entity_id)) {
      $source = "taxonomy/term/{$entity_id}";
    }

    // Build the feed path alias array and send it off to be created.
    $path = array(
      'source' => "$source/$feedappend",
      'alias' => "$alias/feed",
      'language' => $language,
    );
    $existing_alias = _pathauto_existing_alias_data($path['source'], $path['language']);
    _pathauto_set_alias($path, $existing_alias, $op);
  }

  return $success ? $alias : NULL;
}

/**
 * Verify if the given path is a valid menu callback.
 *
 * Taken from menu_execute_active_handler().
 *
 * @param $path
 *   A string containing a relative path.
 * @return
 *   TRUE if the path already exists.
 */
function _pathauto_path_is_callback($path) {
  $menu = menu_get_item($path);
  if (isset($menu['path']) && $menu['path'] == $path) {
    return TRUE;
  }
  elseif (is_file('./' . $path) || is_dir('./' . $path)) {
    // Do not allow existing files or directories to get assigned an automatic
    // alias. Note that we do not need to use is_link() to check for symbolic
    // links since this returns TRUE for either is_file() or is_dir() already.
    return TRUE;
  }
  return FALSE;
}

/**
 * Private function for Pathauto to create an alias.
 *
 * @param $path
 *   An associative array containing the following keys:
 *   - source: The internal system path.
 *   - alias: The URL alias.
 *   - pid: (optional) Unique path alias identifier.
 *   - language: (optional) The language of the alias.
 * @param $existing_alias
 *   (optional) An associative array of the existing path alias.
 * @param $op
 *   An optional string with the operation being performed.
 * @return
 *   TRUE if the path was saved, or NULL otherwise.
 *
 * @see path_set_alias()
 */
function _pathauto_set_alias($path, $existing_alias = NULL, $op = NULL) {
  $verbose = _pathauto_verbose(NULL, $op);

  // Alert users that an existing callback cannot be overridden automatically
  if (_pathauto_path_is_callback($path['alias'])) {
    if ($verbose) {
      _pathauto_verbose(t('Ignoring alias %alias due to existing path conflict.', array('%alias' => $path['alias'])));
    }
    return;
  }
  // Alert users if they are trying to create an alias that is the same as the internal path
  if ($path['source'] == $path['alias']) {
    if ($verbose) {
      _pathauto_verbose(t('Ignoring alias %alias because it is the same as the internal path.', array('%alias' => $path['alias'])));
    }
    return;
  }

  $path += array(
    'pid' => NULL,
    'language' => '',
  );

  // Skip replacing the current alias with an identical alias
  if (empty($existing_alias) || $existing_alias['alias'] != $path['alias']) {
    // If there is already an alias, respect some update actions.
    if (!empty($existing_alias)) {
      switch (variable_get('pathauto_update_action', 2)) {
        case 0:
          // Do not create the alias.
          return;
        case 1:
          // Create a new alias instead of overwriting the existing by leaving
          // $path['pid'] empty.
          break;
        case 3:
          // Create a redirect
          if (module_exists('path_redirect') && function_exists('path_redirect_save')) {
            $redirect = array(
              'source' => $existing_alias['alias'],
              'language' => $existing_alias['language'],
              'redirect' => $path['source'],
            );
            path_redirect_save($redirect);
          }
          // Intentionally fall through to the next condition since we still
          // want to replace the existing alias.
        case 2:
          // Both the redirect and delete actions should overwrite the existing
          // alias.
          $path['pid'] = $existing_alias['pid'];
          break;
      }
    }

    // Save the path array.
    path_set_alias($path['source'], $path['alias'], $path['pid'], $path['language']);

    if ($verbose) {
      if (!empty($redirect)) {
        _pathauto_verbose(t('Created new alias %alias for %source, replacing %old_alias. %old_alias now redirects to %alias.', array('%alias' => $path['alias'], '%source' => $path['source'], '%old_alias' => $existing_alias['alias'])));
      }
      elseif (!empty($existing_alias['pid'])) {
        _pathauto_verbose(t('Created new alias %alias for %source, replacing %old_alias.', array('%alias' => $path['alias'], '%source' => $path['source'], '%old_alias' => $existing_alias['alias'])));
      }
      else {
        _pathauto_verbose(t('Created new alias %alias for %source.', array('%alias' => $path['alias'], '%source' => $path['source'])));
      }
    }

    return TRUE;
  }
}

/**
 * Output a helpful message if verbose output is enabled.
 *
 * Verbose output is only enabled when:
 * - The 'pathauto_verbose' setting is enabled.
 * - The current user has the 'notify of path changes' permission.
 * - The $op parameter is anything but 'bulkupdate' or 'return'.
 *
 * @param $message
 *   An optional string of the verbose message to display. This string should
 *   already be run through t().
 * @param $op
 *   An optional string with the operation being performed.
 * @return
 *   TRUE if verbose output is enabled, or FALSE otherwise.
 */
function _pathauto_verbose($message = NULL, $op = NULL) {
  static $verbose;

  if (!isset($verbose)) {
    $verbose = variable_get('pathauto_verbose', FALSE) && user_access('notify of path changes');
  }

  if (!$verbose || (isset($op) && in_array($op, array('bulkupdate', 'return')))) {
    return FALSE;
  }

  if ($message) {
    drupal_set_message($message);
  }

  return $verbose;
}

/**
 * Generalized function to get tokens across all Pathauto types.
 *
 * @param $object
 *   A user, node, or category object.
 * @return
 *   Tokens for that object formatted in the way that
 *   Pathauto expects to see them.
 */
function pathauto_get_placeholders($type, $object) {
  if (!function_exists('token_get_values')) {
    // TODO at some point try removing this and see if install profiles have problems again.
    watchdog('Pathauto', 'It appears that you have installed Pathauto, which depends on token, but token is either not installed or not installed properly.');
    return array('tokens' => array(), 'values' => array());
  }

  $options = array('pathauto' => TRUE);
  $full = token_get_values($type, $object, TRUE, $options);
  $tokens = token_prepare_tokens($full->tokens);
  $values = pathauto_clean_token_values($full, $options);
  return array('tokens' => $tokens, 'values' => $values);
}

/**
 * Clean tokens so they are URL friendly.
 *
 * @param $full
 *   An array of token values from token_get_values() that need to be "cleaned"
 *   for use in the URL.
 *
 * @return
 *   An array of the cleaned tokens.
 */
function pathauto_clean_token_values($full, $options = array()) {
  $replacements = array();
  foreach ($full->values as $key => $value) {
    $token = $full->tokens[$key];
    if (strpos($token, 'path') !== FALSE && is_array($value) && !empty($options['pathauto'])) {
      // If the token name contains 'path', the token value is an array, and
      // the 'pathauto' option was passed to token_get_values(), then the token
      // should have each segment cleaned, and then glued back together to
      // construct a value resembling an URL.
      $segments = array_map('pathauto_cleanstring', $value);
      $replacements[$token] = implode('/', $segments);
    }
    elseif (preg_match('/(path|alias|url|url-brief)(-raw)?$/', $token)) {
      // Token name matches an URL-type name and should be left raw.
      $replacements[$token] = $value;
    }
    else {
      // Token is not an URL, so it should have its value cleaned.
      $replacements[$token] = pathauto_cleanstring($value);
    }
  }
  return $replacements;
}

/**
 * Return an array of arrays for punctuation values.
 *
 * Returns an array of arrays for punctuation values keyed by a name, including
 * the value and a textual description.
 * Can and should be expanded to include "all" non text punctuation values.
 *
 * @return
 *   An array of arrays for punctuation values keyed by a name, including the
 *   value and a textual description.
 */
function pathauto_punctuation_chars() {
  static $punctuation;

  if (!isset($punctuation)) {
    $punctuation = array();
    $punctuation['double_quotes']      = array('value' => '"', 'name' => t('Double quotes "'));
    $punctuation['quotes']             = array('value' => "'", 'name' => t("Single quotes (apostrophe) '"));
    $punctuation['backtick']           = array('value' => '`', 'name' => t('Back tick `'));
    $punctuation['comma']              = array('value' => ',', 'name' => t('Comma ,'));
    $punctuation['period']             = array('value' => '.', 'name' => t('Period .'));
    $punctuation['hyphen']             = array('value' => '-', 'name' => t('Hyphen -'));
    $punctuation['underscore']         = array('value' => '_', 'name' => t('Underscore _'));
    $punctuation['colon']              = array('value' => ':', 'name' => t('Colon :'));
    $punctuation['semicolon']          = array('value' => ';', 'name' => t('Semicolon ;'));
    $punctuation['pipe']               = array('value' => '|', 'name' => t('Pipe |'));
    $punctuation['left_curly']         = array('value' => '{', 'name' => t('Left curly bracket {'));
    $punctuation['left_square']        = array('value' => '[', 'name' => t('Left square bracket ['));
    $punctuation['right_curly']        = array('value' => '}', 'name' => t('Right curly bracket }'));
    $punctuation['right_square']       = array('value' => ']', 'name' => t('Right square bracket ]'));
    $punctuation['plus']               = array('value' => '+', 'name' => t('Plus +'));
    $punctuation['equal']              = array('value' => '=', 'name' => t('Equal ='));
    $punctuation['asterisk']           = array('value' => '*', 'name' => t('Asterisk *'));
    $punctuation['ampersand']          = array('value' => '&', 'name' => t('Ampersand &'));
    $punctuation['percent']            = array('value' => '%', 'name' => t('Percent %'));
    $punctuation['caret']              = array('value' => '^', 'name' => t('Caret ^'));
    $punctuation['dollar']             = array('value' => '$', 'name' => t('Dollar $'));
    $punctuation['hash']               = array('value' => '#', 'name' => t('Hash #'));
    $punctuation['at']                 = array('value' => '@', 'name' => t('At @'));
    $punctuation['exclamation']        = array('value' => '!', 'name' => t('Exclamation !'));
    $punctuation['tilde']              = array('value' => '~', 'name' => t('Tilde ~'));
    $punctuation['left_parenthesis']   = array('value' => '(', 'name' => t('Left parenthesis ('));
    $punctuation['right_parenthesis']  = array('value' => ')', 'name' => t('Right parenthesis )'));
    $punctuation['question_mark']      = array('value' => '?', 'name' => t('Question mark ?'));
    $punctuation['less_than']          = array('value' => '<', 'name' => t('Less than <'));
    $punctuation['greater_than']       = array('value' => '>', 'name' => t('Greater than >'));
    $punctuation['slash']              = array('value' => '/', 'name' => t('Slash /'));
    $punctuation['back_slash']         = array('value' => '\\', 'name' => t('Backslash \\'));
  }

  return $punctuation;
}

/**
 * Fetch the maximum length of the {url_alias}.dst field from the schema.
 *
 * @return
 *   An integer of the maximum URL alias length allowed by the database.
 */
function _pathauto_get_schema_alias_maxlength() {
  static $maxlength;
  if (!isset($maxlength)) {
    $schema = drupal_get_schema('url_alias');
    $maxlength = $schema['fields']['dst']['length'];
  }
  return $maxlength;
}

/**
 * Fetch an array of non-raw tokens that have matching raw tokens.
 *
 * @return
 *   An array of tokens.
 */
function _pathauto_get_raw_tokens() {
  static $raw_tokens;

  if (!isset($raw_tokens)) {
    $raw_tokens = array();

    // Build one big list of tokens.
    foreach (token_get_list('all') as $tokens) {
      $raw_tokens = array_merge($raw_tokens, array_keys($tokens));
    }

    // Filter out any tokens without -raw as a suffix.
    foreach ($raw_tokens as $index => $token) {
      if (substr($token, -4) !== '-raw') {
        unset($raw_tokens[$index]);
      }
    }

    array_unique($raw_tokens);
  }

  return $raw_tokens;
}

/**
 * Return all the possible paths of the i18n-ascii.txt transliteration file.
 *
 * @return
 *   An array of possible file paths.
 */
function _pathauto_get_i18n_possible_files() {
  $file = 'i18n-ascii.txt';
  $files = array(
    conf_path() . '/' . $file,
    "sites/all/$file",
    drupal_get_path('module', 'pathauto') . '/' . $file,
  );
  // Always prefer $conf['pathauto_i18n_file'] if defined.
  if ($conf_file = variable_get('pathauto_i18n_file', '')) {
    array_unshift($files, $conf_file);
  }
  return $files;
}

/**
 * Fetch the path to the i18n-ascii.txt transliteration file
 *
 * @return
 *   The complete path or FALSE if not found in any of the possible paths.
 *
 * @see _pathauto_get_i18n_possible_files()
 */
function _pathauto_get_i18n_file() {
  static $i18n_file;

  if (!isset($i18n_file)) {
    $i18n_file = FALSE;
    foreach (_pathauto_get_i18n_possible_files() as $file) {
      if (file_exists($file)) {
        $i18n_file = $file;
        break;
      }
    }
  }

  return $i18n_file;
}
