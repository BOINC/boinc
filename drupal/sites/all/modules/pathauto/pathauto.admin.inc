<?php

/**
 * @file
 * Admin page callbacks for the Pathauto module.
 *
 * @ingroup pathauto
 */

/**
 * Form builder; Configure the Pathauto system.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function pathauto_admin_settings() {
  _pathauto_include();

  // Generate the form - settings applying to all patterns first
  $form['general'] = array(
    '#type' => 'fieldset',
    '#weight' => -20,
    '#title' => t('General settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['general']['pathauto_verbose'] = array(
    '#type' => 'checkbox',
    '#title' => t('Verbose'),
    '#default_value' => variable_get('pathauto_verbose', FALSE),
    '#description' => t('Display alias changes (except during bulk updates).'),
  );

  $form['general']['pathauto_separator'] = array(
    '#type' => 'textfield',
    '#title' => t('Separator'),
    '#size' => 1,
    '#maxlength' => 1,
    '#default_value' => variable_get('pathauto_separator', '-'),
    '#description' => t('Character used to separate words in titles. This will replace any spaces and punctuation characters. Using a space or + character can cause unexpected results.'),
  );

  $form['general']['pathauto_case'] = array(
    '#type' => 'radios',
    '#title' => t('Character case'),
    '#default_value' => variable_get('pathauto_case', 1),
    '#options' => array(t('Leave case the same as source token values.'), t('Change to lower case')),
  );

  $max_length = _pathauto_get_schema_alias_maxlength();

  $form['general']['pathauto_max_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum alias length'),
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => variable_get('pathauto_max_length', 100),
    '#min_value' => 1,
    '#max_value' => $max_length,
    '#description' => t('Maximum length of aliases to generate. 100 is recommended. See <a href="@pathauto-help">Pathauto help</a> for details.', array('@pathauto-help' => url('admin/help/pathauto'))),
    '#element_validate' => array('_pathauto_validate_numeric_element'),
  );

  $form['general']['pathauto_max_component_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum component length'),
    '#size' => 3,
    '#maxlength' => 3,
    '#default_value' => variable_get('pathauto_max_component_length', 100),
    '#min_value' => 1,
    '#max_value' => $max_length,
    '#description' => t('Maximum text length of any component in the alias (e.g., [title]). 100 is recommended. See <a href="@pathauto-help">Pathauto help</a> for details.', array('@pathauto-help' => url('admin/help/pathauto'))),
    '#element_validate' => array('_pathauto_validate_numeric_element'),
  );

  $form['general']['pathauto_max_bulk_update'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum number of objects to alias in a bulk update'),
    '#size' => 4,
    '#maxlength' => 4,
    '#default_value' => variable_get('pathauto_max_bulk_update', 50),
    '#min_value' => 0,
    '#description' => t('Maximum number of objects of a given type which should be aliased during a bulk update. The default is 50 and the recommended number depends on the speed of your server. If bulk updates "time out" or result in a "white screen" then reduce the number.'),
    '#element_validate' => array('_pathauto_validate_numeric_element'),
  );

  $actions = array(
    t('Do nothing. Leave the old alias intact.'),
    t('Create a new alias. Leave the existing alias functioning.'),
    t('Create a new alias. Delete the old alias.'));

  if (function_exists('path_redirect_save')) {
    $actions[] = t('Create a new alias. Redirect from old alias.');
  }
  elseif (variable_get('pathauto_update_action', 2) == 3) {
    // the redirect action is selected, but path_redirect is not enabled
    // let's set the variable back to the default
    variable_set('pathauto_update_action', 2);
  }

  $form['general']['pathauto_update_action'] = array(
    '#type' => 'radios',
    '#title' => t('Update action'),
    '#default_value' => variable_get('pathauto_update_action', 2),
    '#options' => $actions,
    '#description' => t('What should pathauto do when updating an existing content item which already has an alias?'),
  );

  $form['general']['pathauto_transliterate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Transliterate prior to creating alias'),
    '#default_value' => variable_get('pathauto_transliterate', FALSE),
    '#description' => t('When a pattern includes certain characters (such as those with accents) should Pathauto attempt to transliterate them into the ASCII-96 alphabet?'),
    '#disabled' => !_pathauto_get_i18n_file(),
  );
  $i18n_locations = _pathauto_get_i18n_possible_files();
  foreach ($i18n_locations as $key => $file) {
    $i18n_locations[$key] = check_plain($file);
    if (file_exists($file)) {
      $i18n_locations[$key] .= ' - <span class="admin-enabled">' . t('File exists') . '</span>';
    }
    else {
      $i18n_locations[$key] .= ' - <span class="admin-disabled">'  . t('Does not exist') . '</span>';
    }
  }
  $form['general']['pathauto_transliterate']['#description'] .= ' ' . t('Transliteration is determined by the i18n-ascii.txt file in the following possible locations, in order of precedence: !locations', array('!locations' => theme('item_list', $i18n_locations)));
  if ($form['general']['pathauto_transliterate']['#disabled']) {
    // Perhaps they've removed the file, set the transliterate option to FALSE
    variable_set('pathauto_transliterate', FALSE);
  }

  $form['general']['pathauto_reduce_ascii'] = array(
    '#type' => 'checkbox',
    '#title' => t('Reduce strings to letters and numbers from ASCII-96'),
    '#default_value' => variable_get('pathauto_reduce_ascii', FALSE),
    '#description' => t('Filters the new alias to only letters and numbers found in the ASCII-96 set.'),
  );

  // Default words to ignore
  $ignore_words = array(
    'a', 'an', 'as', 'at', 'before', 'but', 'by', 'for', 'from', 'is', 'in',
    'into', 'like', 'of', 'off', 'on', 'onto', 'per', 'since', 'than', 'the',
    'this', 'that', 'to', 'up', 'via', 'with',
  );
  $form['general']['pathauto_ignore_words'] = array(
    '#type' => 'textarea',
    '#title' => t('Strings to Remove'),
    '#default_value' => variable_get('pathauto_ignore_words', implode(',', $ignore_words)),
    '#description' => t('Words to strip out of the URL alias, separated by commas. Do not place punctuation in here and do not use WYSIWYG editors on this field.'),
    '#wysiwyg' => FALSE,
  );

  $form['punctuation'] = array(
    '#type' => 'fieldset',
    '#weight' => -10,
    '#title' => t('Punctuation settings'), '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    $details['default'] = 0;
    if ($details['value'] == variable_get('pathauto_separator', '-')) {
      $details['default'] = 1;
    }
    $form['punctuation']['pathauto_punctuation_'. $name] = array(
      '#type' => 'select',
      '#title' => $details['name'],
      '#default_value' => variable_get('pathauto_punctuation_'. $name, $details['default']),
      '#options' => array(
        '0' => t('Remove'),
        '1' => t('Replace by separator'),
        '2' => t('No action (do not replace)'),
      ),
    );
  }

  // Call the hook on all modules - an array of 'settings' objects is returned
  $all_settings = module_invoke_all('pathauto', 'settings');
  foreach ($all_settings as $settings) {
    $module = $settings->module;
    $patterndescr = $settings->patterndescr;
    $patterndefault = $settings->patterndefault;
    $groupheader = $settings->groupheader;
    $supportsfeeds = isset($settings->supportsfeeds) ? $settings->supportsfeeds : NULL;
    variable_set('pathauto_' . $module . '_supportsfeeds', $supportsfeeds);

    $form[$module] = array(
      '#type' => 'fieldset',
      '#title' => $groupheader,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    // Prompt for the default pattern for this module
    $variable = 'pathauto_' . $module . '_pattern';
    $form[$module][$variable] = array(
      '#type' => 'textfield',
      '#title' => $patterndescr,
      '#default_value' => variable_get($variable, $patterndefault),
      '#size' => 65,
      '#maxlength' => 1280,
      '#element_validate' => array('_pathauto_validate_pattern_element'),
      '#after_build' => array('_pathauto_validate_pattern_element'),
      '#token_types' => array($settings->token_type),
      '#parents' => array($variable),
    );

    // If the module supports a set of specialized patterns, set
    // them up here
    if (isset($settings->patternitems)) {
      foreach ($settings->patternitems as $itemname => $itemlabel) {
        $variable = 'pathauto_' . $module . '_' . $itemname . '_pattern';
        $form[$module][$variable] = array(
          '#type' => 'textfield',
          '#title' => $itemlabel,
          '#default_value' => variable_get($variable, ''),
          '#size' => 65,
          '#maxlength' => 1280,
          '#element_validate' => array('_pathauto_validate_pattern_element'),
          '#after_build' => array('_pathauto_validate_pattern_element'),
          '#token_types' => array($settings->token_type),
          '#parents' => array($variable),
        );
      }
    }

    // Display the user documentation of placeholders supported by
    // this module, as a description on the last pattern
    $form[$module]['token_help'] = array(
      '#title' => t('Replacement patterns'),
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Use -raw replacements for text to avoid problems with HTML entities.'),
    );

    // Use the token tree if available.
    $doc = theme('token_tree', array($settings->token_type), FALSE);
    if (empty($doc)) {
      $doc = "<dl>\n";
      foreach ($settings->placeholders as $name => $description) {
        $doc .= '<dt>'. $name .'</dt>';
        $doc .= '<dd>'. $description .'</dd>';
      }
      $doc .= "</dl>\n";
    }
    $form[$module]['token_help']['help'] = array(
      '#type' => 'markup',
      '#value' => $doc,
    );

    // If the module supports bulk updates, offer the update action here
    if ($settings->bulkname) {
      $variable = 'pathauto_' . $module . '_bulkupdate';
      if (variable_get($variable, FALSE)) {
        variable_set($variable, FALSE);
        $function = $module . '_pathauto_bulkupdate';
        call_user_func($function);
      }
      $form[$module][$variable] = array(
        '#type' => 'checkbox',
        '#title' => $settings->bulkname,
        '#default_value' => FALSE,
        '#description' => $settings->bulkdescr,
      );
    }

    // If the module supports feeds, offer to generate aliases for them
    if ($supportsfeeds) {
      $variable = 'pathauto_' . $module . '_applytofeeds';
      $current = variable_get($variable, $supportsfeeds);
      // This checks for the old style from 2.0 and earlier. TODO: At some point we can drop that.
      if (is_numeric($current)) {
        $current = $supportsfeeds;
      }

      $form[$module][$variable] = array(
        '#type' => 'textfield',
        '#title' => t('Internal feed alias text (leave blank to disable)'),
        '#size' => 65,
        '#maxlength' => 1280,
        '#default_value' => $current,
        '#description' => t('The text to use for aliases for RSS feeds. Examples are "0/feed" (used throughout Drupal core) and "feed" (used by some contributed Drupal modules, like Views).'),
      );
    }
  }

  if (isset($do_index_bulkupdate) && $do_index_bulkupdate) {
    drupal_set_message(format_plural($indexcount,
      'Bulk generation of index aliases completed, one alias generated.',
      'Bulk generation of index aliases completed, @count aliases generated.'));
  }

  return system_settings_form($form);
}

/**
 * Element validation callback for URL alias patterns.
 *
 * This function performs the following validations:
 * - Checks if the pattern has at least one token.
 * - Checks if any tokens with raw companions are being used and recommends
 *   use of the raw tokens.
 */
function _pathauto_validate_pattern_element(&$element, &$form_state) {
  // Get the current value of the element (since this can be used during both
  // form display and validation).
  $value = isset($element['#value']) ? $element['#value'] : $element['#default_value'];

  // Empty patterns need no further validation.
  if (!drupal_strlen($value)) {
    return $element;
  }

  // Check to see if the required token functions are available.
  if (!function_exists('token_scan') || !function_exists('token_element_validate_token_context')) {
    drupal_set_message(t('Please make sure you are using the latest version of the Token module.'), 'warning', FALSE);
    return $element;
  }

  // Check for at least one token.
  $tokens = token_scan($value);
  if (empty($tokens)) {
    form_error($element, t('The %name should contain at least one token to ensure unique URL aliases are created.', array('%name' => $element['#title'])));
  }
  else {
    token_element_validate_token_context($element, $form_state);
  }

  // Find any non-raw tokens that do have a raw companion token and warn.
  module_load_include('inc', 'pathauto');
  $not_raw_tokens = array();
  $raw_tokens = _pathauto_get_raw_tokens();
  foreach ($tokens as $token) {
    if (substr($token, -4) === '-raw') {
      // Skip raw tokens.
      continue;
    }
    elseif (in_array($token . '-raw', $raw_tokens)) {
      drupal_set_message(t('You are using the token [%token] which has a raw companion token [%raw_token]. For Pathauto patterns you should use the -raw version of tokens unless you really know what you are doing. See the <a href="@pathauto-help">Pathauto help</a> for more details.', array('%token' => $token, '%raw_token' => $token . '-raw', '@pathauto-help' => url('admin/help/pathauto'))), 'error', FALSE);
    }
  }

  return $element;
}

/**
 * Validate a form element that should have an numeric value.
 */
function _pathauto_validate_numeric_element($element, &$form_state) {
  $value = $element['#value'];

  if (!is_numeric($value)) {
    form_error($element, t('The field %name is not a valid number.', array('%name' => $element['#title'])));
  }
  elseif (isset($element['#max_value']) && $value > $element['#max_value']) {
    form_error($element, t('The field %name cannot be greater than @max.', array('%name' => $element['#title'], '@max' => $element['#max_value'])));
  }
  elseif (isset($element['#min_value']) && $value < $element['#min_value']) {
    form_error($element, t('The field %name cannot be less than @min.', array('%name' => $element['#title'], '@min' => $element['#min_value'])));
  }
}

/**
 * Validate pathauto_admin_settings form submissions.
 */
function pathauto_admin_settings_validate($form, &$form_state) {
  module_load_include('inc', 'pathauto');

  // Perform a basic check for HTML characters in the strings to remove field.
  if (strip_tags($form_state['values']['pathauto_ignore_words']) != $form_state['values']['pathauto_ignore_words']) {
    form_set_error('pathauto_ignore_words', t('The <em>Strings to remove</em> field must not contain HTML. Make sure to disable any WYSIWYG editors for this field.'));
  }

  // Validate that the separator is not set to be removed per http://drupal.org/node/184119
  // This isn't really all that bad so warn, but still allow them to save the value.
  $separator = $form_state['values']['pathauto_separator'];
  $punctuation = pathauto_punctuation_chars();
  foreach ($punctuation as $name => $details) {
    if ($details['value'] == $separator) {
      $action = $form_state['values']['pathauto_punctuation_' . $name];
      if ($action == 0) {
        drupal_set_message(t('You have configured the @name to be the separator and to be removed when encountered in strings. This can cause problems with your patterns and especially with the catpath and termpath patterns. You should probably set the action for @name to be "replace by separator"', array('@name' => $details['name'])), 'error');
      }
    }
  }
}

/**
 * Menu callback; select certain alias types to delete.
 */
function pathauto_admin_delete() {
  /* TODO:
   1) all - DONE
   2) all node aliases - DONE
   4) all user aliases - DONE
   5) all taxonomy aliases - DONE
   6) by node type
   7) by taxonomy vocabulary
   8) no longer existing aliases (see http://drupal.org/node/128366 )
   9) where src like 'pattern' - DON'T DO
   10) where dst like 'pattern' - DON'T DO
  */

  $form['delete'] = array(
    '#type' => 'fieldset',
    '#title' => t('Choose aliases to delete'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  // First we do the "all" case
  $total_count = db_result(db_query('SELECT count(1) FROM {url_alias}'));
  $form['delete']['all_aliases'] = array(
    '#type' => 'checkbox',
    '#title' => t('All aliases'),
    '#default_value' => FALSE,
    '#description' => t('Delete all aliases. Number of aliases which will be deleted: %count.', array('%count' => $total_count)),
  );

  // Next, iterate over an array of objects/alias types which can be deleted and provide checkboxes
  $objects = module_invoke_all('path_alias_types');
  foreach ($objects as $internal_name => $label) {
    $count = db_result(db_query("SELECT count(1) FROM {url_alias} WHERE src LIKE '%s%%'", $internal_name));
    $form['delete'][$internal_name] = array(
      '#type' => 'checkbox',
      '#title' => $label, // This label is sent through t() in the hard coded function where it is defined
      '#default_value' => FALSE,
      '#description' => t('Delete aliases for all @label. Number of aliases which will be deleted: %count.', array('@label' => $label, '%count' => $count)),
    );
  }

  // Warn them and give a button that shows we mean business
  $form['warning'] = array('#value' => '<p>' . t('<strong>Note:</strong> there is no confirmation. Be sure of your action before clicking the "Delete aliases now!" button.<br />You may want to make a backup of the database and/or the url_alias table prior to using this feature.') . '</p>');
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Delete aliases now!'),
  );

  return $form;
}

/**
 * Process pathauto_admin_delete form submissions.
 */
function pathauto_admin_delete_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if ($value) {
      if ($key === 'all_aliases') {
        db_query('DELETE FROM {url_alias}');
        drupal_set_message(t('All of your path aliases have been deleted.'));
      }
      $objects = module_invoke_all('path_alias_types');
      if (array_key_exists($key, $objects)) {
        db_query("DELETE FROM {url_alias} WHERE src LIKE '%s%%'", $key);
        drupal_set_message(t('All of your %type path aliases have been deleted.', array('%type' => $objects[$key])));
      }
    }
  }
  $form_state['redirect'] = 'admin/build/path/delete_bulk';
}
