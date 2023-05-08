<?php
// $id$

/**
 * Functions that are shared amongst files and dependent modules go
 * here to keep the clutter down in the main module file.
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Page callbacks from hook_menu()
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 *
 */
function boinctranslate_initialize_languages() {

  require_once(getcwd() . '/includes/locale.inc');

  $api_base_url = 'https://www.transifex.com/api/2';
  $project_name = variable_get(
    'boinc_translate_transifex_project_name', ''
  );
  $operations = array();

  if ($project_name) {
    // Get all languages configured for this project at Transifex
    $path = "project/{$project_name}/languages";
    $response = boinctranslate_transifex_request($path);

    if ($response == '404 NOT FOUND') {
      drupal_set_message(
        t('Unable to get languages for %project.',
          array(
            '%project' => $project_name,
          )
        ), 'warning'
      );
    }
    elseif ($response) {
      if (is_array($response)) {

        $installed_languages = language_list();
        $available_languages = _locale_get_predefined_list();
        $transifex_languages = array();
        $disabled_languages = array();
        $process_batches = FALSE;

        // Set up Transifex languages in Drupal
        foreach ($response as $language) {
          $posix_code = $language['language_code'];
          $rfc_code = strtolower(str_replace('_', '-', $posix_code));
          $transifex_languages[$rfc_code] = $language;
          if (!isset($installed_languages[$rfc_code])) {
            // See if this language can be installed from a predefined list
            if (isset($available_languages[$rfc_code])) {
              locale_add_language(
                $rfc_code,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                FALSE
              );
              drupal_set_message(
                'Added predefined language: '.$available_languages[$rfc_code][0]
              );
              db_query("UPDATE {languages} SET enabled = 1 WHERE language = '%s'", $rfc_code);
            }
            else {
              // Retrieve language details from Transifex
              $path = "language/{$posix_code}";
              $response = boinctranslate_transifex_request($path);

              if ($response == '404 NOT FOUND') {
                drupal_set_message(
                  t('Unable to get details for language %code.',
                    array(
                      '%code' => $posix_code,
                    )
                  ), 'warning'
                );
              }
              elseif ($response) {
                if (!empty($response['name'])) {
                  // Add a custom language to Drupal and enable
                  locale_add_language(
                    $rfc_code,
                    $response['name'],
                    $response['name'],
                    $response['rtl'],
                    NULL,
                    NULL,
                    TRUE
                  );
                  drupal_set_message(
                    'Added new language: '.$response['name']
                  );
                }
                else {
                  $variables = array(
                    '%code' => $posix_code,
                  );
                  drupal_set_message(
                    t('Unable to get details for language %code.', $variables),
                    'error'
                  );
                  watchdog(
                    'boinctranslate',
                    'Unable to get details for language %code.',
                    $variables,
                    WATCHDOG_ERROR
                  );
                }
              }
              else {
               $variables = array(
                  '%code' => $posix_code,
                );
                drupal_set_message(
                  t('Invalid response while getting details for language %code.', $variables),
                  'error'
                );
                watchdog(
                  'boinctranslate',
                  'Invalid response while getting details for language %code.',
                  $variables,
                  WATCHDOG_ERROR
                );
              }
            }
            // Import any language files for the newly added language
            if ($batch = locale_batch_by_language($rfc_code, '_locale_batch_language_finished')) {
              $operations = array_merge($operations, $batch['operations']);
              $process_batches = TRUE;
            }
          }
        }
        drupal_set_message('Finished installing official BOINC languages.');
        // Disable languages that are not in Transifex
        foreach ($installed_languages as $langcode => $language) {
          if (!isset($transifex_languages[$langcode])) {
            $disabled_languages[$langcode] = $langcode;
            db_query("UPDATE {languages} SET enabled = 0 WHERE language = '%s'", $langcode);
          }
        }
        if ($disabled_languages) {
          drupal_set_message('The following languages were not found in Transifex and were disabled: ' . implode(' ', $disabled_languages));
        }
        if ($process_batches) {
          $batch = array(
            'operations' => $operations,
          );
          batch_set($batch);
          batch_process('admin/boinc/translation');
        }
      }
      else {
        $variables = array(
          '%project' => $project_name,
        );
        drupal_set_message(
          t('No languages found for %project. (Does the configured account have sufficient privileges at Transifex?)', $variables),
          'error'
        );
        watchdog(
          'boinctranslate',
          'No languages found for %project. (Does the configured account have sufficient privileges at Transifex?)',
          $variables,
          WATCHDOG_ERROR
        );
      }
    }
    else {
     $variables = array(
        '%project' => $project_name,
      );
      drupal_set_message(
        t('Invalid response while getting languages for %project.', $variables),
        'error'
      );
      watchdog(
        'boinctranslate',
        'Invalid response while getting languages for %project.',
        $variables,
        WATCHDOG_ERROR
      );
    }
  }
  drupal_goto('admin/boinc/translation');
}

/**
 *
 */
function boinctranslate_export_translations() {
  require_once(getcwd() . '/includes/locale.inc');
  $project_name = variable_get(
    'boinc_translate_transifex_project_name', ''
  );
  // Get resource names from local config
  $resource_config = (variable_get(
    'boinc_translate_transifex_project_resources', ''
  ));
  $resource_names = boinctranslate_parse_resources($resource_config);
  $primary_resource = reset($resource_names);

  if ($project_name AND $primary_resource) {
    // Create or update the translation source, if needed
    $source_exists = FALSE;
    $path = "project/{$project_name}/resources";
    $resources = boinctranslate_transifex_request($path);
    if ($resources AND is_array($resources)) {
      foreach ($resources as $resource) {
        if ($resource['slug'] == $primary_resource) {
          $source_exists = TRUE;
          break;
        }
      }
      if (!$source_exists) {
        // Create the source
        $path = "project/{$project_name}/resources";
        $post = array(
          'slug' => $primary_resource,
          'name' => 'Drupal-Project',
          'i18n_type' => 'PO',
          'category' => 'Drupal',
          'content' => boinctranslate_get_po('en', 'project'),
        );
        $result = boinctranslate_transifex_request($path, $post);
      }
      else {
        // Update the source
        $path = "project/{$project_name}/resource/{$primary_resource}/content";
        $post = array(
          'content' => boinctranslate_get_po('en', 'project')
        );
        $result = boinctranslate_transifex_request($path, $post, TRUE, TRUE);
      }
    }

    if (is_array($result) OR substr($result, 0, 6) != 'ERROR:') {
      $enabled_languages = locale_language_list();
      if ($source_exists) {
        drupal_set_message('Updated source translation strings at Transifex');
      }
      else {
        drupal_set_message('Established new translation resource at Transifex');
      }
      // Try to export translations for all enabled languages
      foreach ($enabled_languages as $langcode => $language_name) {
        if ($langcode == 'en') {
          continue;
        }
        $po_file = boinctranslate_get_po($langcode, 'project');
        if ($po_file) {
          $path = "project/{$project_name}/resource/{$primary_resource}/translation/{$langcode}";
          $post = array(
            'content' => $po_file,
          );
          $result = boinctranslate_transifex_request($path, $post, TRUE, TRUE);
          if (!is_array($result)
          AND substr($result, 0, 6) == 'ERROR:') {
            drupal_set_message(
              "Unable to update {$language_name} translations: {$result}",
              'warning'
            );
          }
          else {
            drupal_set_message("Updated {$language_name} translations");
            //drupal_set_message('DEBUG: <pre>'.print_r($result,1).'</pre>');
          }
        }
        else {
          drupal_set_message("No translations to export for {$language_name}");
        }
      }
    }
    else {
      drupal_set_message(
        "Unable to update the translation source: {$result}",
        'warning'
      );
    }
  }
  else {
    drupal_set_message(
      'Failed to export translations: Transifex settings are not intiailized.',
      'error'
    );
  }
  drupal_goto('admin/boinc/translation');
}

/**
 *
 */
function boinctranslate_update_official_boinc_translations() {
  require_once(getcwd() . '/includes/locale.inc');
  $project_name = variable_get(
    'boinc_translate_transifex_standard_name', ''
  );
  $drupal_resource = variable_get(
    'boinc_translate_transifex_boinc_drupal_resource', ''
  );

  if ($project_name AND $drupal_resource) {
    // Create or update the translation source, if needed
    $source_exists = FALSE;
    $path = "project/{$project_name}/resources";
    $resources = boinctranslate_transifex_request($path);
    if ($resources AND is_array($resources)) {
      foreach ($resources as $resource) {
        if ($resource['slug'] == $drupal_resource) {
          $source_exists = TRUE;
          break;
        }
      }
    }

    if ($source_exists) {
      $enabled_languages = locale_language_list();
      // Try to export translations for all enabled languages
      foreach ($enabled_languages as $langcode => $language_name) {
        if ($langcode == 'en') {
          continue;
        }
        $po_file = boinctranslate_get_po($langcode, 'boinc');
        if ($po_file) {
          $path = "project/{$project_name}/resource/{$drupal_resource}/translation/{$langcode}";
          $post = array(
            'content' => $po_file,
          );
          $result = boinctranslate_transifex_request($path, $post, TRUE, TRUE);

          if (!is_array($result)) {
            if (substr($result, 0, 6) == 'ERROR:') {
              drupal_set_message(
                "Unable to update {$language_name} official BOINC translations: {$result}",
                'warning'
              );
            }
            elseif ($result == '401 UNAUTHORIZED') {
              drupal_set_message(
                'Not authorized to update official BOINC translations',
                'warning'
              );
              break;
            }
            elseif ($result == 'success') {
              drupal_set_message("Updated {$language_name} official BOINC translations");
            }
            else {
              drupal_set_message(
                "Unexpected response for {$language_name}: {$result}",
                'warning'
              );
            }
          }
          else {
            drupal_set_message("Updated {$language_name} official BOINC translations");
            //drupal_set_message('DEBUG: <pre>'.print_r($result,1).'</pre>');
          }
        }
        else {
          drupal_set_message("No official BOINC translations to export for {$language_name}");
        }
      }
    }
    else {
      drupal_set_message(
        "The {$drupal_resource} resource does not exist in the {$project_name} project at Transifex",
        'warning'
      );
    }
  }
  else {
    drupal_set_message(
      'Failed to export official BOINC translations: Transifex settings are not intiailized.',
      'error'
    );
  }
  drupal_goto('admin/boinc/translation');
}

/**
 *
 */
function boinctranslate_download_pot($type = 'boinc') {
  $po = boinctranslate_get_po(NULL, $type);
  _locale_export_po(NULL, $po);
}

/**
 *
 */
function boinctranslate_transifex_request($path, $post = NULL, $json = TRUE, $use_put = FALSE, $username = '', $password = '') {
  $debug_mode = variable_get('boinc_debug_mode', 0);

  // Transifex details
  $api_base_url = 'https://www.transifex.com/api/2';
  if (!$username) $username = variable_get('boinc_translate_transifex_user', '');
  if (!$password) $password = variable_get('boinc_translate_transifex_pass', '');

  $url = "{$api_base_url}/{$path}";
  $headers = array(
    'Authorization' => 'Basic ' . base64_encode($username . ":" . $password),
  );
  $data = NULL;

  if ($post) {
    if ($json) {
      $headers['Content-Type'] = 'application/json';
      $data = json_encode($post);
    }
    else {
      $data = drupal_query_string_encode($post);
    }
    $method = ($use_put) ? 'PUT' : 'POST';
  }
  else {
    $method = 'GET';
  }

  $response = drupal_http_request($url, $headers, $method, $data, 1, 10);

  switch ($response->code) {
  case 200:
  case 304:
    if ($json) {
      // Process as JSON
      return json_decode($response->data, TRUE);
    }
    else {
      return (string) $response->data;
    }
    break;
  case 404:
    return '404 NOT FOUND';
  case 401:
    return '401 UNAUTHORIZED';
  case 400:
    if ($debug_mode) watchdog('boinctranslate', "The following response was received when trying to communicate with the Transifex system: \n{$response}", array(), WATCHDOG_WARNING);
    return "ERROR: {$response->data}";
  case 405:
    if ($debug_mode) watchdog('boinctranslate', "The following response was received when trying to communicate with the Transifex system: \n{$response}", array(), WATCHDOG_WARNING);
    return "ERROR: User not allowed to perform this action";
  }

  return NULL;
}

/**
 *
 */
function boinctranslate_get_po($langcode, $type = 'standard') {

  require_once(getcwd() . '/includes/locale.inc');

  $all_languages = language_list();
  $language = $langcode ? $all_languages[$langcode] : NULL;
  $textgroups = array();
  $strings = array();

  switch ($type) {
  case 'standard':
    $textgroups = array(
      'default',
      'taxonomy',
      'views',
    );
    break;
  case 'boinc':
    $textgroups = array(
      'boinc',
    );
    break;
  case 'project':
    $textgroups = array(
      'blocks',
      'menu',
      'project',
    );
    break;
  default:
  }

  // Merge textgroup strings together for export as one file
  foreach ($textgroups as $textgroup) {
    $strings += _locale_export_get_strings($language, $textgroup);
  }
  ksort($strings);
  foreach ($strings as $id => $string) {
    // Set the string context
    $strings[$id]['context'] = trim($string['comment']);
  }
  if ($langcode AND $langcode != 'en') {
    // If not the source language, remove untranslated strings from the ouput
    foreach ($strings as $i => $string) {
      if (!$string['translation']) {
        unset($strings[$i]);
      }
    }
  }
  if ($strings) {
    return boinctranslate_export_po_generate($language, $strings, NULL, $type);
  }

  return NULL;
}

/**
 *
 */
function boinctranslate_export_po_generate($language = NULL, $strings = array(), $header = NULL, $type = NULL) {

  require_once(getcwd() . '/includes/locale.inc');
  global $user;

  // unset $language to indicate template creation
  if (isset($language) && $language->language == "en") {
    $language = NULL;
  }

  if (!isset($header)) {
    if (isset($type) && $type == "project") {
      $header = "# ".variable_get('site_name', 'Drupal-BOINC')." drupal localization template\n";
      $header .= "# Copyright (C) ".date("Y")." ".variable_get('site_name', 'Drupal-BOINC')."\n";
    } else {
      $header = "# BOINC drupal localization template\n";
      $header .= "# Copyright (C) ".date("Y")." University of California\n";
    }
    $header .= '# Generated by ' . $user->name . ' <' . $user->mail . ">\n";
    $header .= "#\n";
    $header .= "# This file is distributed under the same license as BOINC.\n";
    $header .= "#\n";
    $header .= "msgid \"\"\n";
    $header .= "msgstr \"\"\n";
    if (isset($type) && $type == "project") {
      $header .= "\"Project-Id-Version: ".variable_get('site_name', 'Drupal-BOINC')." ".date("Y-m-d-H:iO")."\\n\"\n";
    } else {
      $header .= "\"Project-Id-Version: BOINC unknown\\n\"\n"; // TODO: add SHA1 of source checkout here
    }
    $header .= "\"Report-Msgid-Bugs-To: BOINC translation team <boinc_loc@ssl.berkeley.edu>\\n\"\n";
    $header .= "\"POT-Creation-Date: " . date("Y-m-d H:iO") . "\\n\"\n";
    if (isset($language)) {
      $header .= "\"PO-Revision-Date: " . date("Y-m-d H:iO") . "\\n\"\n";
    }
    $header .= "\"Last-Translator: Generated automatically from Drupal translate interface\\n\"\n";
    $header .= "\"MIME-Version: 1.0\\n\"\n";
    $header .= "\"Content-Type: text/plain; charset=utf-8\\n\"\n";
    $header .= "\"Content-Transfer-Encoding: 8bit\\n\"\n";
    if (isset($language)) {
      $header .= "\"Language: ".$language->language."\\n\"";
      if ($language->formula && $language->plurals) {
        $header .= "\"Plural-Forms: nplurals=" . $language->plurals . "; plural=" . strtr($language->formula, array('$' => '')) . ";\\n\"\n";
      }
    }
    $header .= "\"X-Poedit-SourceCharset: utf-8\\n\"\n";
  }

  $output = $header . "\n";

  foreach ($strings as $lid => $string) {
    // Only process non-children, children are output below their parent.
    if (!isset($string['child'])) {
      if ($string['comment']) {
        $output .= '#: ' . $string['comment'] . "\n";
      }
      if ($string['context']) {
        $output .= 'msgctxt "' . $string['context'] . "\"\n";
      }
      $output .= 'msgid ' . _locale_export_string($string['source']);
      if (!empty($string['plural'])) {
        $plural = $string['plural'];
        $output .= 'msgid_plural ' . _locale_export_string($strings[$plural]['source']);
        if (isset($language)) {
          $translation = $string['translation'];
          for ($i = 0; $i < $language->plurals; $i++) {
            $output .= 'msgstr[' . $i . '] ' . _locale_export_string($translation);
            if ($plural) {
              $translation = _locale_export_remove_plural($strings[$plural]['translation']);
              $plural = isset($strings[$plural]['plural']) ? $strings[$plural]['plural'] : 0;
            }
            else {
              $translation = '';
            }
          }
        }
        else {
          $output .= 'msgstr[0] ""' . "\n";
          $output .= 'msgstr[1] ""' . "\n";
        }
      }
      else {
        $output .= 'msgstr ' . _locale_export_string($string['translation']);
      }
      $output .= "\n";
    }
  }
  return $output;
}

/**
 *
 */
function boinctranslate_refresh_translations() {
  require_once(getcwd() . '/includes/locale.inc');
  $errors = array();
  $languages = locale_language_list();
  $translation_resources = array();
  $operations = array();
  $debug_mode = variable_get('boinc_debug_mode', 0);

  $boinc_name = variable_get(
    'boinc_translate_transifex_standard_name', ''
  );
  $boinc_resources = boinctranslate_parse_resources(
    variable_get('boinc_translate_transifex_standard_resources', array())
  );
  // Add the Official BOINC resource to the list of BOINC resources;
  // The official resource overrides any additional resources provided, so it
  // should be added to the end so as to be processed last
  $drupal_resource = variable_get(
    'boinc_translate_transifex_boinc_drupal_resource', ''
  );
  $boinc_resources[] = $drupal_resource;
  if ($boinc_name AND $boinc_resources) {
    $translation_resources[$boinc_name] = array(
      'resources' => $boinc_resources,
      'textgroups' => array(
        'boinc',
        'default',
        'taxonomy',
        'views',
      ),
    );
  }
  $project_name = variable_get(
    'boinc_translate_transifex_project_name', ''
  );
  $project_resources = boinctranslate_parse_resources(
    variable_get('boinc_translate_transifex_project_resources', array())
  );
  if ($project_name AND $project_resources) {
    $translation_resources[$project_name] = array(
      'resources' => $project_resources,
      'textgroups' => array(
        'blocks',
        'menu',
        'project',
      ),
    );
    // Be sure any strings from the override file are added to the boinc group
    $override_file = './' . drupal_get_path('module', 'boinctranslate') . '/includes/other-boinc-translation-strings.txt';
    $other_strings = file($override_file);
    if ($other_strings) {
      foreach ($other_strings as $string) {
        $string = trim($string);
        if ($string) {
          // Ignore lines starting with '#' i.e., comments.
          if ($string[0] === "#")
            continue;

          // Use '|' as delimiter to split string and context info.
          $line = explode("|", $string);
          if ($line) {
            if (count($line)==1) {
              $tl0 = trim($line[0]);
              if ($tl0) {
                bts($tl0);
              }
            }
            elseif (count($line)>1) {
              $tl0 = trim($line[0]);
              $tl1 = trim($line[1]);
              if ($tl0 and $tl1) {
                bts($tl0, array(), NULL, $tl1);
              }
            }
          }// if ($line)

        }// if ($string)
      }// foreach
    }// if ($other_strings)
  }

  foreach ($languages as $langcode => $language) {
    if ($langcode == 'en') {
      continue;
    }

    $import_stats = array(
      'new' => 0,
      'updated' => 0,
      'deleted' => 0,
      'skipped' => 0,
    );

    foreach ($translation_resources as $project => $translation) {
      foreach ($translation['resources'] as $resource) {

        // Add this language to the batch operations
        $operations[] = array(
          'boinctranslate_refresh_translations_op',
          array(
            $project, $resource, $langcode, $language, $translation['textgroups'], $debug_mode
          ),
        );
      }
    }
    if ($batch = locale_batch_by_language($langcode)) {
      foreach ($batch['operations'] as $op) {
        $operations[] = array(
          'boinctranslate_refresh_translations_op',
          array(
            NULL, $op[1][0], $langcode, $language, array('default'), $debug_mode
          ),
        );
      }
    }
  }

  $batch = array(
    'operations' => $operations,
    'finished' => 'boinctranslate_refresh_translations_finished',
    'title' => t('Importing translations'),
    'init_message' => t('Beginning translation import...'),
    'progress_message' => t('Applied @current out of @total translation updates.'),
    'error_message' => t('Translation import has encountered an error.'),
  );

  batch_set($batch);
  batch_process();
}


/**
 * Batch operation for importing translations
 */
function boinctranslate_refresh_translations_op($project, $resource, $langcode, $language, $textgroups, $debug_mode, &$context) {

  require_once(getcwd() . '/includes/locale.inc');

  if ($debug_mode) {
    watchdog(
      'boinctranslate',
      'Checking for @language updates in @project:@resource',
      array('@language' => $language, '@project' => $project, '@resource' => $resource),
      WATCHDOG_INFO
    );
  }

  if ($project) {
    // Import the configured resources
    $success = FALSE;
    $message = '';
    $path = "project/{$project}/resource/{$resource}/translation/{$langcode}";
    $response = boinctranslate_transifex_request($path);

    if ($response == '404 NOT FOUND') {
      $message = "Project resource {$project}:{$resource} not found in {$language}.";
    }
    elseif ($response) {
      if (!empty($response['content'])) {
        $po_text = $response['content'];

        // Write the translation file to a temporary location
        $file = new stdClass();
        $file->filepath = file_save_data($po_text, NULL);
        $file->filename = basename($file->filepath);
        if (!$file->filepath) {
          $message = 'Unable to create temporary file in '
            . file_directory_temp() . " for {$language} translation "
            . "resource {$project}:{$resource}";
        }

        foreach ($textgroups as $textgroup) {
          // Import the translations from the file to each related textgroup
          if (!$results = _boinctranslate_locale_import_po($file, $langcode, LOCALE_IMPORT_OVERWRITE, $textgroup)) {
            $message = "The {$language} translation import of"
              . " {$project}:{$resource} failed.";
            $success = FALSE;
            break;
          }
          else {
            $success = TRUE;
          }
        }
      }
      else {
        $message = "Unable to read response for {$language} translation import"
          . " of {$project}:{$resource}.";
      }
    }
    else {
      $message = "Translation data not found in response for {$language}"
        . " translation import of {$project}:{$resource}.";
    }
  }
  else {
    // If project isn't specified, import as a local Drupal resource
    $project = 'drupal.local';
    $file = new stdClass();
    $file->filepath = $resource;
    $file->filename = basename($file->filepath);
    if (!$results = _boinctranslate_locale_import_po($file, $langcode, LOCALE_IMPORT_OVERWRITE, $textgroup)) {
      $message = "The {$language} translation import of"
        . " local file {$resource} failed.";
      $success = FALSE;
    }
    else {
      $success = TRUE;
    }
  }

  if ($success) {
    // Store some result for post-processing in the finished callback.
    $context['results']['success'][] = "{$langcode}:{$textgroup}";
    $message = "Imported {$language} translations in {$project}:{$resource} ({$results['new']} added, {$results['updated']} refreshed, {$results['deleted']} removed)";

    if ($debug_mode) {
      watchdog(
        'boinctranslate',
        $message,
        array(),
        WATCHDOG_INFO
      );
    }
  }
  else {
    $context['results']['failure'][] = "{$langcode}:{$textgroup}";
    watchdog(
      'boinctranslate',
      $message,
      array(),
      WATCHDOG_WARNING
    );
  }

  // Update our progress information.
  $context['sandbox']['progress']++;
  $context['sandbox']['language'] = $langcode;
  $context['message'] = $message;

  // Update the progress for the batch engine
  if ($context['sandbox']['progress'] >= $context['sandbox']['max']) {
    $context['finished'] = 1;
  }
  else {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  }
}

/**
 * Batch 'finished' callback
 */
function boinctranslate_refresh_translations_finished($success, $results, $operations) {
  if ($success) {
    // Let's count our successes
    $count = count($results['success']);
    $message = "Successfully completed {$count} import operations";
    watchdog(
      'boinctranslate',
      'Successfully completed @count import operations.',
      array('@count' => $count),
      WATCHDOG_INFO
    );
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = 'An error occurred while processing ' . $error_operation[0] . ' with arguments :' . print_r($error_operation[0], TRUE);
    watchdog(
      'boinctranslate',
      $message,
      array(),
      WATCHDOG_WARNING
    );
  }
  drupal_set_message($message);
  drupal_goto('admin/boinc/translation');
}

/**
 *
 */
function _boinctranslate_locale_import_po($file, $langcode, $mode, $group = NULL) {
  // Try to allocate enough time to parse and import the data.
  if (function_exists('set_time_limit')) {
    @set_time_limit(240);
  }

  require_once(getcwd() . '/includes/locale.inc');

  // Check if we have the language already in the database.
  if (!db_fetch_object(db_query("SELECT language FROM {languages} WHERE language = '%s'", $langcode))) {
    drupal_set_message(t('The language selected for import is not supported.'), 'error');
    return FALSE;
  }

  // Get strings from file (returns on failure after a partial import, or on success)
  $status = _boinctranslate_locale_import_read_po('db-store', $file, $mode, $langcode, $group);
  if ($status === FALSE) {
    // Error messages are set in _locale_import_read_po().
    return FALSE;
  }

  // Get status information on import process.
  list($headerdone, $additions, $updates, $deletes, $skips) = _boinctranslate_locale_import_one_string('db-report');

  if (!$headerdone) {
    drupal_set_message(t('The translation file %filename appears to have a missing or malformed header.', array('%filename' => $file->filename)), 'error');
  }

  // Clear cache and force refresh of JavaScript translations.
  _locale_invalidate_js($langcode);
  cache_clear_all('locale:', 'cache', TRUE);

  // Rebuild the menu, strings may have changed.
  menu_rebuild();

  return array(
    'new' => $additions,
    'updated' => $updates,
    'deleted' => $deletes,
    'skipped' => $skips,
  );
}

/**
 *
 */
function _boinctranslate_locale_import_read_po($op, $file, $mode = NULL, $lang = NULL, $group = 'default') {

  require_once(getcwd() . '/includes/locale.inc');

  $fd = fopen($file->filepath, "rb"); // File will get closed by PHP on return
  if (!$fd) {
    watchdog(
      'boinctranslate',
      'The translation import for %lang failed, because %filename could not be read.',
      array('%lang' => $lang, '%filename' => $file->filename),
      WATCHDOG_WARNING
    );
    _locale_import_message('The translation import failed, because the file %filename could not be read.', $file);
    return FALSE;
  }

  $context = "COMMENT"; // Parser context: COMMENT, MSGID, MSGID_PLURAL, MSGSTR and MSGSTR_ARR
  $current = array(); // Current entry being read
  $plural = 0; // Current plural form
  $lineno = 0; // Current line

  while (!feof($fd)) {
    $line = fgets($fd, 10 * 1024); // A line should not be this long
    if ($lineno == 0) {
      // The first line might come with a UTF-8 BOM, which should be removed.
      $line = str_replace("\xEF\xBB\xBF", '', $line);
    }
    $lineno++;
    $line = trim(strtr($line, array("\\\n" => "")));

    if (!strncmp("#", $line, 1)) { // A comment
      if ($context == "COMMENT") { // Already in comment context: add
        $current["#"][] = substr($line, 1);
      }
      elseif (($context == "MSGSTR") || ($context == "MSGSTR_ARR")) { // End current entry, start a new one
        _boinctranslate_locale_import_one_string($op, $current, $mode, $lang, $file, $group);
        $current = array();
        $current["#"][] = substr($line, 1);
        $context = "COMMENT";
      }
      else { // Parse error
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: "msgstr" was expected but not found on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: "msgstr" was expected but not found on line %line.', $file, $lineno);
        return FALSE;
      }
    }
    elseif (!strncmp("msgctxt", $line, 7)) {
      if (($context == "MSGSTR") || ($context == "MSGSTR_ARR")) { // End current entry, start a new one
        _boinctranslate_locale_import_one_string($op, $current, $mode, $lang, $file, $group);
        $current = array();
      }
      elseif (($context == "MSGID") || ($context == "MSGCTXT")) { // Already in this context? Parse error
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: "msgctxt" is unexpected on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: "msgid" is unexpected on line %line.', $file, $lineno);
        return FALSE;
      }
      $line = trim(substr($line, 7));
      $quoted = _locale_import_parse_quoted($line);
      if ($quoted === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      $current["msgctxt"] = $quoted;
      $context = "MSGCTXT";
    }
    elseif (!strncmp("msgid_plural", $line, 12)) {
      if ($context != "MSGID") { // Must be plural form for current entry
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: "msgid_plural" was expected but not found on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: "msgid_plural" was expected but not found on line %line.', $file, $lineno);
        return FALSE;
      }
      $line = trim(substr($line, 12));
      $quoted = _locale_import_parse_quoted($line);
      if ($quoted === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      $current["msgid"] = $current["msgid"] . "\0" . $quoted;
      $context = "MSGID_PLURAL";
    }
    elseif (!strncmp("msgid", $line, 5)) {
      if (($context == "MSGSTR") || ($context == "MSGSTR_ARR")) { // End current entry, start a new one
        _boinctranslate_locale_import_one_string($op, $current, $mode, $lang, $file, $group);
        $current = array();
      }
      elseif ($context == "MSGID") { // Already in this context? Parse error
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: "msgid" is unexpected on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: "msgid" is unexpected on line %line.', $file, $lineno);
        return FALSE;
      }
      $line = trim(substr($line, 5));
      $quoted = _locale_import_parse_quoted($line);
      if ($quoted === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      $current["msgid"] = $quoted;
      $context = "MSGID";
    }
    elseif (!strncmp("msgstr[", $line, 7)) {
      if (($context != "MSGID") && ($context != "MSGID_PLURAL") && ($context != "MSGSTR_ARR")) { // Must come after msgid, msgid_plural, or msgstr[]
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: "msgstr[]" is unexpected on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: "msgstr[]" is unexpected on line %line.', $file, $lineno);
        return FALSE;
      }
      if (strpos($line, "]") === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      $frombracket = strstr($line, "[");
      $plural = substr($frombracket, 1, strpos($frombracket, "]") - 1);
      $line = trim(strstr($line, " "));
      $quoted = _locale_import_parse_quoted($line);
      if ($quoted === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      $current["msgstr"][$plural] = $quoted;
      $context = "MSGSTR_ARR";
    }
    elseif (!strncmp("msgstr", $line, 6)) {
      if ($context != "MSGID") { // Should come just after a msgid block
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: "msgstr" is unexpected on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: "msgstr" is unexpected on line %line.', $file, $lineno);
        return FALSE;
      }
      $line = trim(substr($line, 6));
      $quoted = _locale_import_parse_quoted($line);
      if ($quoted === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      $current["msgstr"] = $quoted;
      $context = "MSGSTR";
    }
    elseif ($line != "") {
      $quoted = _locale_import_parse_quoted($line);
      if ($quoted === FALSE) {
        watchdog(
          'boinctranslate',
          'The translation file %filename for language %lang contains a syntax error on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains a syntax error on line %line.', $file, $lineno);
        return FALSE;
      }
      if (($context == "MSGID") || ($context == "MSGID_PLURAL")) {
        $current["msgid"] .= $quoted;
      }
      elseif ($context == "MSGSTR") {
        $current["msgstr"] .= $quoted;
      }
      elseif ($context == "MSGSTR_ARR") {
        $current["msgstr"][$plural] .= $quoted;
      }
      else {
        watchdog(
          'boinctranslate',
          'The translation file %filename for %lang contains an error: there is an unexpected string on line %line.',
          array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
          WATCHDOG_WARNING
        );
        _locale_import_message('The translation file %filename contains an error: there is an unexpected string on line %line.', $file, $lineno);
        return FALSE;
      }
    }
  }

  // End of PO file, flush last entry
  if (($context == "MSGSTR") || ($context == "MSGSTR_ARR")) {
    _boinctranslate_locale_import_one_string($op, $current, $mode, $lang, $file, $group);
  }
  elseif ($context != "COMMENT") {
    watchdog(
      'boinctranslate',
      'The translation file %filename for %lang ended unexpectedly at line %line.',
      array('%filename' => $file->filename, '%lang' => $lang, '%line' => $lineno),
      WATCHDOG_WARNING
    );
    _locale_import_message('The translation file %filename ended unexpectedly at line %line.', $file, $lineno);
    return FALSE;
  }

}

/**
 *
 */
function _boinctranslate_locale_import_one_string($op, $value = NULL, $mode = NULL, $lang = NULL, $file = NULL, $group = 'default') {

  require_once(getcwd() . '/includes/locale.inc');

  static $report = array(
    'additions' => 0,
    'updates' => 0,
    'deletes' => 0,
    'skips' => 0,
  );
  static $headerdone = FALSE;
  static $strings = array();

  switch ($op) {
    // Return stored strings
    case 'mem-report':
      return $strings;

      // Store string in memory (only supports single strings)
    case 'mem-store':
      $strings[$value['msgid']] = $value['msgstr'];
      return;

      // Called at end of import to inform the user
    case 'db-report':
      return array(
        $headerdone,
        $report['additions'],
        $report['updates'],
        $report['deletes'],
        $report['skips'],
      );

      // Store the string we got in the database.
    case 'db-store':
      // We got header information.
      if ($value['msgid'] == '') {
        $languages = language_list();
        if (($mode != LOCALE_IMPORT_KEEP) || empty($languages[$lang]->plurals)) {
          // Since we only need to parse the header if we ought to update the
          // plural formula, only run this if we don't need to keep existing
          // data untouched or if we don't have an existing plural formula.
          $header = _locale_import_parse_header($value['msgstr']);

          // Get and store the plural formula if available.
          if (isset($header["Plural-Forms"]) && $p = _locale_import_parse_plural_forms($header["Plural-Forms"], $file->filename)) {
            list($nplurals, $plural) = $p;
            db_query("UPDATE {languages} SET plurals = %d, formula = '%s' WHERE language = '%s'", $nplurals, $plural, $lang);
          }
        }
        $headerdone = TRUE;
      }

      else {
        // Some real string to import.
        $comments = _locale_import_shorten_comments(empty($value['#']) ? array() : $value['#']);

        if (strpos($value['msgid'], "\0")) {
          // This string has plural versions.
          $english = explode("\0", $value['msgid'], 2);
          $entries = array_keys($value['msgstr']);
          for ($i = 3; $i <= count($entries); $i++) {
            $english[] = $english[1];
          }
          $translation = array_map('_locale_import_append_plural', $value['msgstr'], $entries);
          $english = array_map('_locale_import_append_plural', $english, $entries);
          foreach ($translation as $key => $trans) {
            if ($key == 0) {
              $plid = 0;
            }
            $plid = _boinctranslate_locale_import_one_string_db($report, $lang, $english[$key], $trans, $group, $comments, $mode, $plid, $key);
          }
        }

        else {
          // A simple string to import.
          $english = $value['msgid'];
          $translation = $value['msgstr'];
          _boinctranslate_locale_import_one_string_db($report, $lang, $english, $translation, $group, $comments, $mode);
        }
      }
  } // end of db-store operation
}

/**
 * Modify the _locale_import_one_string_db() function so that it does not add
 * translation strings that do not exist on the site.
 *
 * Function will attempt t overwrite strings in textgroups other than
 * 'boinc' or 'project', without using the location field as a search
 * parameter. All found rows in locales_source are overwritten by the
 * translations.
 *
 * Textgroups 'boinc' and 'project' use the location string to
 * uniquely identify rows in the locales_source database.
 */
function _boinctranslate_locale_import_one_string_db(&$report, $langcode, $source, $translation, $textgroup, $location, $mode, $plid = NULL, $plural = NULL) {

  $ignoreoverwrite = FALSE;
  $lid = 0;

  // Use different DB query depending on the textgroup.
  $uselocation = array("boinc", "project");
  if (in_array($textgroup, $uselocation)) {
    $resource = db_query("SELECT lid FROM {locales_source} WHERE location = '%s' AND source = '%s' AND textgroup = '%s'", $location, $source, $textgroup);
  }
  else {
    $resource = db_query("SELECT lid FROM {locales_source} WHERE source = '%s' AND textgroup = '%s'", $source, $textgroup);

    // Parse the location string for the string ignoreoverwrite, which
    // ignores the LOCAL_IMPORT_OVERWRITE directive below. $parts[2]
    // is hardcoded, it is the third field, separated by ':' in the
    // location string.
    $parts = explode(':', $location);
    if (!empty($parts[2])) {
      if (preg_match('/(ignoreoverwrite)/', $parts[2])) {
        $ignoreoverwrite = TRUE;
      }
    }
  }// if (in_array($textgroup, $uselocation))

  if (!empty($translation)) {
    // Skip this string unless it passes a check for dangerous code.
    // Text groups other than default still can contain HTML tags
    // (i.e. translatable blocks).
    if ($textgroup == "default" && !locale_string_is_safe($translation)) {
      $report['skips']++;
      $lid = 0;
    }
    elseif ($resource) {
      // We have this source string saved already. Loop over the db results from locales_source table.
      while ($row = db_fetch_array($resource)) {

        $lid = $row['lid'];
        // Check of if one or more translations exist for this lid in locales_target table.
        $exists = (bool) db_result(db_query("SELECT lid FROM {locales_target} WHERE lid = %d AND language = '%s'", $lid, $langcode));
        if (!$exists) {
          // No translation in this language, insert translation into locales_target table.
          db_query("INSERT INTO {locales_target} (lid, language, translation, plid, plural) VALUES (%d, '%s', '%s', %d, %d)", $lid, $langcode, $translation, $plid, $plural);
          $report['additions']++;
        }
        else if ( ($mode == LOCALE_IMPORT_OVERWRITE) and (!$ignoreoverwrite) ) {
          // Translation exists, only overwrite if instructed.
          db_query("UPDATE {locales_target} SET translation = '%s', plid = %d, plural = %d WHERE language = '%s' AND lid = %d", $translation, $plid, $plural, $langcode, $lid);
          $report['updates']++;
        }
        else {
          $report['skips']++;
        }// end if !$exists

      }// while
    }
  }
  elseif ($mode == LOCALE_IMPORT_OVERWRITE AND $resource) {
    // Loop over db results from locales_source table.
    while ($row = db_fetch_array($resource)) {
      $lid = $row['lid'];
      $exists = (bool) db_result(db_query("SELECT lid FROM {locales_target} WHERE lid = %d AND language = '%s'", $lid, $langcode));
      if ($exists) {
        // Empty translation, remove existing if instructed.
        db_query("DELETE FROM {locales_target} WHERE language = '%s' AND lid = %d AND plid = %d AND plural = %d", $langcode, $lid, $plid, $plural);
        $report['deletes']++;
      }// if $exists
    }// while
  }
  else {
    $report['skips']++;
  }

  return $lid;
}



/**
 * Parse valid resources out of configuration
 */
function boinctranslate_parse_resources($resource_text) {
  $resources = array();
  $resource_array = explode(
    "\n", $resource_text
  );
  foreach ($resource_array as $resource) {
    $resource = trim($resource);
    if ($resource AND $resource[0] != '#') {
      $resources[] = $resource;
    }
  }
  return $resources;
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Helper Functions for boinctranslate_filter
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */
function _boinctranslate_filter_tips_long() {
    return t("<a id=\"filter-boinctranslate\"></a>
<h2>BOINC Translate Filter Help Information</h2>

<p>Use this filter to split the body of a node into chunks to make translations easier. Add the token '#SPLIT_TOKEN#', without the quotes in the editor when creating or editing a Web page. The filter will then split the text chunks using these tokens as delimiters. Each chunk is translated separately. Example: if a long page as five paragraphs, and each is a chunk, and paragraphs 2 and 3 are edited, chunks 1, 4, and 5 will not require re-translation.</p>

<p>The editor/admin may add as many of these tokens as you wish. You may add and remove them at your discretion. It is permitted for a page as no tokens, as the entire content will be a single chunk- such as for a shorter page.</p>
");
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Misc utility/helper functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Split a string by newline, and trim each line, implode back into a
 * string. Used for cleaning whitespace title & description elements
 * from project preferences XML upload before adding to translation
 * database.
 */
function _boinctranslate_supertrim($instr) {
  return implode("\n", array_map('trim', preg_split('/\r\n|\r|\n/', $instr) ) );
}
