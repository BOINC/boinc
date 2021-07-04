<?php
/**
 * CKEditor - The text editor for the Internet - http://ckeditor.com
 * Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
 *
 * == BEGIN LICENSE ==
 *
 * Licensed under the terms of any of the following licenses of your
 * choice:
 *
 *  - GNU General Public License Version 2 or later (the "GPL")
 *    http://www.gnu.org/licenses/gpl.html
 *
 *  - GNU Lesser General Public License Version 2.1 or later (the "LGPL")
 *    http://www.gnu.org/licenses/lgpl.html
 *
 *  - Mozilla Public License Version 1.1 or later (the "MPL")
 *    http://www.mozilla.org/MPL/MPL-1.1.html
 *
 * == END LICENSE ==
 *
 * @file
 * CKEditor Module for Drupal 6.x
 *
 * This module allows Drupal to replace textarea fields with CKEditor.
 *
 * CKEditor is an online rich text editor that can be embedded inside web pages.
 * It is a WYSIWYG (What You See Is What You Get) editor which means that the
 * text edited in it looks as similar as possible to the results end users will
 * see after the document gets published. It brings to the Web popular editing
 * features found in desktop word processors such as Microsoft Word and
 * OpenOffice.org Writer. CKEditor is truly lightweight and does not require any
 * kind of installation on the client computer.
 */

/**
 * Guess the absolute server path to the document root
 * Usually it should return $_SERVER['DOCUMENT_ROOT']
 *
 * @todo Improve me!!
 * Returns absolute path or false on failure
 *
 * @return string|boolean
 */
function ckeditor_get_document_root_full_path() {
  $found=0;
  $index_dir  = realpath(dirname($_SERVER['SCRIPT_FILENAME'])); // {/dir1/dir2/home/drupal}/index.php
  if (getcwd() == $index_dir) {
    $found++;
  }
  $drupal_dir = base_path();
  $index_dir  = str_replace('\\', "/", $index_dir);
  $drupal_dir = str_replace('\\', "/", $drupal_dir);
  $document_root_dir   = $index_dir .'/'. str_repeat('../', substr_count($drupal_dir, '/')-1);
  $document_root_dir   = realpath($document_root_dir);
  $document_root_dir   = rtrim($document_root_dir, '/\\');
  if ($document_root_dir == $_SERVER['DOCUMENT_ROOT']) {
    $found++;
  }
  $document_root_dir   = str_replace('\\', '/', $document_root_dir);

  if ($document_root_dir != '') {
    $found++;
  }
  if (file_exists($document_root_dir)) {
    $found++;
  }
  if (file_exists($document_root_dir . base_path() .'includes/bootstrap.inc')) {
    $found++;
  }

  if ($found >= 3) {
    return $document_root_dir;
  }
  else{
    return FALSE;
  }
}

/**
 * Emulates the asp Server.mapPath function.
 * Given an url path return the physical directory that it corresponds to.
 *
 * Returns absolute path or false on failure
 *
 * @param string $path
 * @return @return string|boolean
 */
function ckeditor_resolve_url($path) {
  if (function_exists('apache_lookup_uri')) {
    $info = @apache_lookup_uri($path);
    if (!$info) {
      return FALSE;
    }
    return $info->filename . $info->path_info ;
  }

  $document_root = ckeditor_get_document_root_full_path();
  if ($document_root !== FALSE) {
    return $document_root . $path;
  }

  return FALSE;
}

function ckeditor_load_skin_options() {
  $arr = array();
  $editor_local_path  = ckeditor_path(TRUE);
  $skin_dir     = $editor_local_path .'/skins';
  if (is_dir($skin_dir)) {
    $dh = @opendir($skin_dir);
    if (FALSE !== $dh) {
      while (($file = readdir($dh)) !== FALSE ) {
        if (in_array($file, array(".", "..", "CVS", ".svn"))) {
          continue;
        }
        if (is_dir($skin_dir . DIRECTORY_SEPARATOR . $file)) {
          $arr[$file] = drupal_ucfirst($file);
        }
      }
      closedir( $dh );
    }
  }

  //oops, we have no information about skins, let's use only default
  if (empty($arr)) {
    $arr = array(
      'kama' => 'Kama',
    );
  }
  asort($arr);

  return $arr;
}

/**
 * Return default skin for CKEditor
 *
 * @return string
 */
function ckeditor_default_skin() {
  $skin_options = ckeditor_load_skin_options();
  if (array_key_exists('moono', $skin_options)) {
    return 'moono';
  }
  if (array_key_exists('kama', $skin_options)) {
    return 'kama';
  }
  //if any default theme not exists select first from the list
  return key(reset($skin_options));
}

function ckeditor_load_lang_options() {
  $arr = array();
  $editor_local_path = ckeditor_path(TRUE);
  $lang_file = $editor_local_path . '/lang/_languages.js';
  if (file_exists($lang_file)) {
    $f = fopen($lang_file, 'r');
    $file = fread($f, filesize($lang_file));
    $tmp = explode('{', $file);
    if (isset($tmp[2])) {
      $tmp = explode('}', $tmp[2]);
    }
    $langs = explode(',', $tmp[0]);
    foreach ($langs AS $lang) {
      preg_match("/'?(\w+-?\w+)'?:'([\w\s\(\)]+)'/i", $lang, $matches);
      if (isset($matches[1]) && isset($matches[2]))
        $arr[$matches[1]] = $matches[2];
    }
  }

  //oops, we have no information about languages, let's use those available in CKEditor 2.4.3
  if (empty($arr)) {
    $arr = array(
      'af' => 'Afrikaans',
      'ar' => 'Arabic',
      'bg' => 'Bulgarian',
      'bn' => 'Bengali/Bangla',
      'bs' => 'Bosnian',
      'ca' => 'Catalan',
      'cs' => 'Czech',
      'da' => 'Danish',
      'de' => 'German',
      'el' => 'Greek',
      'en' => 'English',
      'en-au' => 'English (Australia)',
      'en-ca' => 'English (Canadian)',
      'en-uk' => 'English (United Kingdom)',
      'eo' => 'Esperanto',
      'es' => 'Spanish',
      'et' => 'Estonian',
      'eu' => 'Basque',
      'fa' => 'Persian',
      'fi' => 'Finnish',
      'fo' => 'Faroese',
      'fr' => 'French',
      'gl' => 'Galician',
      'he' => 'Hebrew',
      'hi' => 'Hindi',
      'hr' => 'Croatian',
      'hu' => 'Hungarian',
      'it' => 'Italian',
      'ja' => 'Japanese',
      'km' => 'Khmer',
      'ko' => 'Korean',
      'lt' => 'Lithuanian',
      'lv' => 'Latvian',
      'mn' => 'Mongolian',
      'ms' => 'Malay',
      'nb' => 'Norwegian Bokmal',
      'nl' => 'Dutch',
      'no' => 'Norwegian',
      'pl' => 'Polish',
      'pt' => 'Portuguese (Portugal)',
      'pt-br' => 'Portuguese (Brazil)',
      'ro' => 'Romanian',
      'ru' => 'Russian',
      'sk' => 'Slovak',
      'sl' => 'Slovenian',
      'sr' => 'Serbian (Cyrillic)',
      'sr-latn' => 'Serbian (Latin)',
      'sv' => 'Swedish',
      'th' => 'Thai',
      'tr' => 'Turkish',
      'uk' => 'Ukrainian',
      'vi' => 'Vietnamese',
      'zh' => 'Chinese Traditional',
      'zh-cn' => 'Chinese Simplified',
    );
  }

  asort($arr);

  return $arr;
}

/**
 * List of CKEditor plugins
 *
 * @return array
 */
function ckeditor_load_plugins($render = FALSE) {
  $arr = array();
  $base_path = '%base_path%';
  $editor_path = '%editor_path%';
  $ckeditor_path = '%ckeditor_path%';
  $plugin_dir = '%plugin_dir%';
  $plugin_dir_additional = '%plugin_dir_extra%';
  $pattern = '#\.addButton\([\s]*[\'"](.*?)[\'"][\s]*\,[\s]*\{[\s]*(.*?)[\s]*\}#s';

  /*
   * External plugins
   */
  if (module_exists('ckeditor_swf') && file_exists(drupal_get_path('module', 'ckeditor_swf') . '/plugins/swf/plugin.js')) {
    $arr['ckeditor_swf'] = array(
      'name' => 'swf',
      'desc' => t('Support for the CKEditor SWF module'),
      'path' => $base_path . drupal_get_path('module', 'ckeditor_swf') . '/plugins/swf/',
      'buttons' => FALSE,
      'default' => 't'
      );
  }

  if (module_exists('ckeditor_link') && file_exists(drupal_get_path('module', 'ckeditor_link') . '/plugins/link/plugin.js')) {
    $arr['ckeditor_link'] = array(
      'name' => 'drupal_path',
      'desc' => t('Support for the CKEditor Link module'),
      'path' => $base_path . drupal_get_path('module', 'ckeditor_link') . '/plugins/link/',
      'buttons' => FALSE,
      'default' => 't'
    );
  }

  if (module_exists('linkit') && file_exists(drupal_get_path('module', 'linkit') . '/editors/ckeditor/plugin.js')) {
    $arr['linkit'] = array(
      'name' => 'Linkit',
      'desc' => t('Support for the Linkit module <em>(buttons: Linkit)</em>'),
      'path' => $base_path . drupal_get_path('module', 'linkit') . '/editors/ckeditor/',
      'buttons' => array(
        'Linkit' => array(
          'title' => 'Linkit',
          'icon' => $base_path . drupal_get_path('module', 'linkit') . '/editors/ckeditor/linkit.png'
        )
      ),
      'default' => 't'
    );
  }

  /*
   * CKEditor build-in plugins
   */
  $_editor_path = ckeditor_path(TRUE) . '/';
  if (file_exists($_editor_path . 'plugins/tableresize/plugin.js')) {
    $arr['tableresize'] = array(
      'name' => 'tableresize',
      'desc' => t('Table Resize plugin'),
      'path' => $base_path . $editor_path . 'plugins/tableresize/',
      'buttons' => FALSE,
      'default' => 't'
    );
  }

  if (file_exists($_editor_path . 'plugins/autogrow/plugin.js')) {
    $arr['autogrow'] = array(
      'name' => 'autogrow',
      'desc' => t('Auto Grow plugin'),
      'path' => $base_path . $editor_path . 'plugins/autogrow/',
      'buttons' => FALSE,
      'default' => 'f'
    );
  }

  if (file_exists($_editor_path . 'plugins/stylesheetparser/plugin.js')) {
    $arr['stylesheetparser'] = array(
      'name' => 'stylesheetparser',
      'desc' => t('Stylesheet Parser plugin'),
      'path' => $base_path . $editor_path . 'plugins/stylesheetparser/',
      'buttons' => FALSE,
      'default' => 'f'
  );
}

  /*
   * CKEditor module plugins
   */
  $_plugin_dir = drupal_get_path('module', 'ckeditor') . '/plugins/';
  if (is_dir($_plugin_dir) && $handle = opendir($_plugin_dir)) {
    while (false !== ($file = readdir($handle))) {
      if (is_dir($_plugin_dir . $file) && file_exists($_plugin_dir . $file . '/plugin.js')) {
        $source = file_get_contents($_plugin_dir . $file . '/plugin.js');
        $buttons = array();
        if (preg_match_all($pattern, $source, $matches)) {
          foreach ($matches[1] as $i => $button_name) {
            if (preg_match('#(icon)[\s]*\:[\s]*([^\,\n]*)#', $matches[2][$i], $matches2)) {
              $buttons[$button_name] = array();
              $buttons[$button_name]['label'] = $button_name;
              $matches2[2] = str_replace(array('this.path', '+', '\'', '"'), array('', '', '', ''), $matches2[2]);
              $buttons[$button_name]['icon'] = trim($matches2[2]);
            }
          }
        }
        if (preg_match('#@file ([^\n\r]*)#', $source, $matches)) {
          $arr[$file] = array(
            'name' => $file,
            'desc' => t($matches[1]),
            'path' => $base_path . $plugin_dir . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
        else {
          $arr[$file] = array(
            'name' => $file,
            'desc' => t('Plugin file: ' . $file),
            'path' => $base_path . $plugin_dir . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
      }
    }

    closedir($handle);
  }

  /*
   * CKEditor module plugins - additional directory
   */
  $_plugin_dir_additional = ckeditor_plugins_path(TRUE) . '/';
  if ($_plugin_dir != $_plugin_dir_additional && is_dir($_plugin_dir_additional) && $handle = opendir($_plugin_dir_additional)) {
    while (false !== ($file = readdir($handle))) {
      if (is_dir($_plugin_dir_additional . $file) && file_exists($_plugin_dir_additional . $file . '/plugin.js')) {
        $source = file_get_contents($_plugin_dir_additional . $file . '/plugin.js');
        $buttons = array();
        if (preg_match_all($pattern, $source, $matches)) {
          foreach ($matches[1] as $i => $button_name) {
            if (preg_match('#(icon)[\s]*\:[\s]*([^\,\n]*)#', $matches[2][$i], $matches2)) {
              $buttons[$button_name] = array();
              $buttons[$button_name]['label'] = $button_name;
              $matches2[2] = str_replace(array('this.path', '+', '\'', '"'), array('', '', '', ''), $matches2[2]);
              $buttons[$button_name]['icon'] = trim($matches2[2]);
            }
          }
        }
        if (preg_match('#@file ([^\n\r]*)#', $source, $matches)) {
          $arr[$file] = array(
            'name' => $file,
            'desc' => t($matches[1]),
            'path' => $base_path . $plugin_dir_additional . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
        else {
          $arr[$file] = array(
            'name' => $file,
            'desc' => t('Plugin file: ' . $file),
            'path' => $base_path . $plugin_dir_additional . $file . '/',
            'buttons' => (count($buttons) > 0) ? $buttons : FALSE,
            'default' => 'f'
          );
        }
      }
    }

    closedir($handle);
  }

  /*
   * CKEditor plugins registered by hook
   */
  $plugins = module_invoke_all('ckeditor_plugin');

  foreach ($plugins as $i => $plugin) {
    if (file_exists($plugin['path'] . 'plugin.js')) {
      $source = file_get_contents($plugin['path'] . 'plugin.js');
      $plugins[$i]['path'] = $base_path . $plugin['path'];
      if (!isset($plugin['buttons']) || count($plugin['buttons']) == 0) {
        $buttons = array();
        if (preg_match_all($pattern, $source, $matches)) {
          foreach ($matches[1] as $j => $button_name) {
            if (preg_match('#(icon)[\s]*\:[\s]*([^\,\n]*)#', $matches[2][$j], $matches2)) {
              $buttons[$button_name] = array();
              $buttons[$button_name]['label'] = $button_name;
              $matches2[2] = str_replace(array('this.path', '+', '\'', '"'), array('', '', '', ''), $matches2[2]);
              $buttons[$button_name]['icon'] = trim($matches2[2]);
            }
          }
        }
        $plugins[$i]['buttons'] = (count($buttons) > 0) ? $buttons : FALSE;
      }
    }
    else {
      unset($plugins[$i]);
    }
  }
  $arr = array_merge($arr, $plugins);

  if (isset($arr['linktomenu']) && module_exists('linktocontent') == FALSE) {
    unset($arr['linktomenu']);
  }

  if (isset($arr['linktonode']) && module_exists('linktocontent') == FALSE) {
    unset($arr['linktonode']);
  }

  if (isset($arr['imce']) && module_exists('imce') == FALSE) {
    unset($arr['imce']);
  }

  //remove page break button if there is no module to do this
  if (isset($arr['drupalbreaks']['buttons']['DrupalPageBreak']) && !module_exists('paging') && !module_exists('pagebreak')) {
    unset($arr['drupalbreaks']['buttons']['DrupalPageBreak']);
  }

  if (isset($arr['drupalbreaks'])) {
    $arr['drupalbreaks']['default'] = 't';
  }

  ksort($arr);

  if ($render === TRUE) {
    $arr = ckeditor_plugins_render($arr);
  }

  return $arr;
}

/**
 * Render CKEditor plugins path
 */
function ckeditor_plugins_render($plugins) {
  $render = array();
  $render["%base_path%"] = base_path();
  $render["%editor_path%"] = ckeditor_path(TRUE) . '/';
  $render["%ckeditor_path%"] = drupal_get_path('module', 'ckeditor') . '/';
  $render["%plugin_dir%"] = $render["%ckeditor_path%"] . 'plugins/';
  $render["%plugin_dir_extra%"] = ckeditor_plugins_path(TRUE) . '/';

  foreach ((array) $plugins as $i => $plugin) {
    $plugins[$i]['path'] = str_replace(array_keys($render), array_values($render), $plugin['path']);
  }

  return $plugins;
}


function ckeditor_user_get_setting($user, $profile, $setting) {
  $default = array(
    'default' => 't',
    'show_toggle' => 't',
    'popup' => 'f',
    'toolbar' => 'default',
    'width' => '100%',
    'lang' => 'en',
    'auto_lang' => 't',
  );

  if ($profile->settings['allow_user_conf']) {
    $status = isset($user->{'ckeditor_'. $setting}) ? $user->{'ckeditor_'. $setting} : (isset($profile->settings[$setting]) ? $profile->settings[$setting] : $default[$setting]);
  }
  else {
    $status = isset($profile->settings[$setting]) ? $profile->settings[$setting] : $default[$setting];
  }

  return $status;
}

function ckeditor_user_get_profile($user, $element_id = 'edit-body', $url = NULL) {
  $rids = array();
  if (is_null($url)) {
      $url = $_GET['q'];
  }
  // Since ckeditor_profile_load() makes a db hit, only call it when we're pretty sure
  // we're gonna render ckeditor.
  $sorted_roles = ckeditor_sorted_roles();
  foreach (array_keys($sorted_roles) as $rid) {
    if (isset($user->roles[$rid])) {
      $rids[] = $rid;
    }
  }

  if ($user->uid == 1 && !sizeof($rids)) {
    $r = db_fetch_object(db_query_range("SELECT r.rid FROM {ckeditor_role} r ORDER BY r.rid DESC", 1));
    $rids[] = $r->rid;
  }

  $profile_names = array();
  if (sizeof($rids)) {
    $result = db_query("SELECT r.rid, s.name FROM {ckeditor_settings} s INNER JOIN {ckeditor_role} r ON r.name = s.name WHERE r.rid IN (". implode(",", $rids) .")");
    while (($row = db_fetch_array($result))) {
      if (!isset($profile_names[$row['rid']])) {
        $profile_names[$row['rid']] = array();
      }
      array_push($profile_names[$row['rid']], $row['name']);
    }
  }

  foreach ($rids as $rid) {
    if (!empty($profile_names[$rid])) {
      foreach ($profile_names[$rid] as $profile_name) {
        $profile = ckeditor_profile_load($profile_name);

        $conf = $profile->settings;
        $enabled = ckeditor_is_enabled(empty($conf['excl_mode']) ? '0' : $conf['excl_mode'], empty($conf['excl_regex']) ? '' : $conf['excl_regex'], $element_id, $url);

        if ($enabled) {
          return $profile;
        }
      }
    }
  }

  return FALSE;
}

/**
 * sort roles according to precedence settings. previously sorted roles are followed by latest added roles.
 */
function ckeditor_sorted_roles($clear = FALSE) {
  static $order;
  if (isset($order) && $clear !== TRUE) {
    return $order;
  }
  $order = array();
  $roles = user_roles(0, 'access ckeditor');

  $result = db_query("SELECT settings FROM {ckeditor_settings} WHERE name='CKEditor Global Profile'");
  $data = db_fetch_object($result);
  if (!empty($data->settings)) {
    $settings = unserialize($data->settings);
    if (isset($settings['rank']) && !empty($settings['rank']))
    foreach ($settings['rank'] as $rid) {
      if (isset($roles[$rid])) {
        $order[$rid] = $roles[$rid];
        unset($roles[$rid]);
      }
    }
  }
  krsort($roles);//sort the remaining unsorted roles by id, descending.
  $order += $roles;
  return $order;
}

/**
 * @param int $excl_mode 1/include, exclude otherwise
 * @param string $excl_regex paths (drupal paths with ids attached)
 * @param string $element_id current ID
 * @param string $get_q current path
 *
 * @return boolean
 *    returns true if CKEditor is enabled
 */
function ckeditor_is_enabled($excl_mode, $excl_regex, $element_id, $get_q) {
  global $theme;

  $front = variable_get('site_frontpage', 'node');
  $excl_regex = str_replace('<front>', $front, $excl_regex);
  $nodetype = ckeditor_get_nodetype($get_q);
  $element_id = str_replace('.', '\.', $element_id);

  $match = !empty($excl_regex) && preg_match($excl_regex, $theme . ':' . $nodetype .'@'. $get_q .'.'. $element_id);

  return ($excl_mode == '0' xor $match);
}

function _ckeditor_script_path() {
  $jspath = FALSE;
  $module_path=drupal_get_path('module', 'ckeditor');

  if (file_exists($module_path . '/ckeditor/ckeditor.js')) {
    $jspath='%m/ckeditor';
  }
  elseif (file_exists($module_path . '/ckeditor/ckeditor/ckeditor.js')) {
    $jspath='%m/ckeditor/ckeditor';
  }
  elseif (file_exists('sites/all/libraries/ckeditor/ckeditor.js')) {
    $jspath='%b/sites/all/libraries/ckeditor';
  }
  return $jspath;
}

/**
 * Determines whether the CKEditor sources are present
 *
 * It checks if ckeditor.js is present.
 *
 * This function is used by ckeditor_requirements()
 *
 * @return boolean True if CKEditor is installed
 */
function _ckeditor_requirements_isinstalled() {
  $editor_path = ckeditor_path(TRUE);
  $jspath = $editor_path .'/ckeditor.js';
  $jsp=file_exists($jspath);
  if (!$jsp && ($editor_path = _ckeditor_script_path())) {
    $result = db_query("SELECT name, settings FROM {ckeditor_settings} WHERE name = 'CKEditor Global Profile'");
    if ($rs=db_fetch_array($result)) {
      $rs['settings']=unserialize($rs['settings']);
      $rs['settings']['ckeditor_path']=$editor_path;
      $rs['settings']=serialize($rs['settings']);
      db_query("UPDATE {ckeditor_settings} SET settings='%s' WHERE name = 'CKEditor Global Profile'", $rs['settings']);
      $jsp=TRUE;
      ckeditor_path(TRUE, TRUE);
    }
  }
  return $jsp;
}
