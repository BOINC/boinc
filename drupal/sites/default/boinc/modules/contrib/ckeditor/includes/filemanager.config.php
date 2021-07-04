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
 * This file is required by the CKEeditor module if you want to enable CKFinder,
 * an advanced Ajax file browser.
 *
 */

$GLOBALS['devel_shutdown'] = FALSE;

$ckfinder_user_files_path = '';
$ckfinder_user_files_absolute_path = '';

function CheckAuthentication() {
  static $authenticated;

  if (!isset($authenticated)) {
    if (!empty($_SERVER['SCRIPT_FILENAME'])) {
      $drupal_path = dirname(dirname(dirname(dirname($_SERVER['SCRIPT_FILENAME']))));
      if (!file_exists($drupal_path .'/includes/bootstrap.inc')) {
        $drupal_path = dirname(dirname(dirname($_SERVER['SCRIPT_FILENAME'])));
        $depth = 2;
        do {
          $drupal_path = dirname($drupal_path);
          $depth ++;
        }
        while (!($bootstrap_file_found = file_exists($drupal_path .'/includes/bootstrap.inc')) && $depth<10);
      }
    }

    if (!isset($bootstrap_file_found) || !$bootstrap_file_found) {
      $drupal_path = '../../../../..';
      if (!file_exists($drupal_path .'/includes/bootstrap.inc')) {
        $drupal_path = '../..';
        do {
          $drupal_path .= '/..';
          $depth = substr_count($drupal_path, '..');
        }
        while (!($bootstrap_file_found = file_exists($drupal_path .'/includes/bootstrap.inc')) && $depth < 10);
      }
    }
    if (!isset($bootstrap_file_found) || $bootstrap_file_found) {
      $current_cwd = getcwd();
      chdir($drupal_path);
      require_once './includes/bootstrap.inc';
      drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
      $authenticated = user_access('allow CKFinder file uploads');
      if (isset($_SESSION['ckeditor']['UserFilesPath'], $_SESSION['ckeditor']['UserFilesAbsolutePath'])) {
        $GLOBALS['ckfinder_user_files_path'] = $_SESSION['ckeditor']['UserFilesPath'];
        $GLOBALS['ckfinder_user_files_absolute_path'] = $_SESSION['ckeditor']['UserFilesAbsolutePath'];
      }
      chdir($current_cwd);
    }
  }

  return $authenticated;
}

CheckAuthentication();

if (!empty($ckfinder_user_files_path)) {
  $baseUrl = $ckfinder_user_files_path;
  $baseDir = $ckfinder_user_files_absolute_path;
}
else {
  // Nothing in session? Shouldn't happen... anyway let's try to upload it in the (almost) right place
  // Path to user files relative to the document root.
  $baseUrl = strtr(base_path(), array(
    '/modules/ckeditor/ckfinder/core/connector/php' => '',
  )) . file_directory_path() .'/';
  $baseDir = resolveUrl($baseUrl);
}
