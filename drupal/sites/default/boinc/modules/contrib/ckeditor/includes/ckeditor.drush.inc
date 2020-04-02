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
 * @file
 * Drush integration for the CKEditor module.
 */

/**
 * Implements hook_drush_command().
 */
function ckeditor_drush_command() {
  $items['ckeditor-download'] = array(
    'callback' => 'ckeditor_drush_download',
    'description' => dt('Downloads the required CKEditor library from svn.ckeditor.com.'),
    'arguments' => array(
      'path' => dt('Optional. The path to the download folder. If omitted, Drush will use the default location (<code>sites/all/libraries/ckeditor</code>).'),
    ),
  );
  return $items;
}

/**
 * Downloads
 */
function ckeditor_drush_download() {
  $args = func_get_args();
  if ($args[0]) {
    $path = $args[0];
  }
  else {
    $path = drush_get_context('DRUSH_DRUPAL_ROOT') . '/sites/all/libraries/ckeditor';
  }

  if (drush_shell_exec('svn checkout http://svn.ckeditor.com/CKEditor/releases/stable/ ' . $path)) {
    drush_log(dt('CKEditor was downloaded to @path.', array('@path' => $path)), 'success');
  }
  else {
    drush_log(dt('Drush was unable to download CKEditor to @path.', array('@path' => $path)), 'error');
  }
}

/**
 * Implements drush_MODULE_post_COMMAND().
 */
function drush_ckeditor_post_enable() {
  $modules = func_get_args();
  if (in_array('ckeditor', $modules)) {
    ckeditor_drush_download();
  }
}
