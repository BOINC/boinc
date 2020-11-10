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
 * CKEditor is an online rich text editor that can be embedded inside web pages.
 * It is a WYSIWYG (What You See Is What You Get) editor which means that the
 * text edited in it looks as similar as possible to the results end users will
 * see after the document gets published. It brings to the Web popular editing
 * features found in desktop word processors such as Microsoft Word and
 * OpenOffice.org Writer. CKEditor is truly lightweight and does not require any
 * kind of installation on the client computer.
 * This module allows Drupal to replace textarea fields with CKEditor.
 *
 *
 */

function ckeditor_user_delegate($type, $edit, &$user, $category = NULL) {
  if ($type == 'form' && $category == 'account' && user_access('access ckeditor', $user)) {
    module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');

    $profile = ckeditor_user_get_profile($user);
    if (!$profile) {
      // No profile has been assigned.
      return NULL;
    }

    $lang_options = ckeditor_load_lang_options();

    // because the settings are saved as strings we need to test for the string 'true'
    if (isset($profile->settings['allow_user_conf']) && $profile->settings['allow_user_conf'] == 't') {
      $form['ckeditor'] = array(
        '#type' => 'fieldset',
        '#title' => t('Rich text editor settings'),
        '#weight' => 10,
        '#collapsible' => TRUE,
        '#collapsed' => TRUE
      );

      $form['ckeditor']['ckeditor_default'] = array(
        '#type' => 'radios',
        '#title' => t('Default state'),
        '#default_value' => isset($user->ckeditor_default) ? $user->ckeditor_default : (isset($profile->settings['default']) ? $profile->settings['default'] : 'f'),
        '#options' => array(
          't' => t('Enabled'),
          'f' => t('Disabled')
        ),
        '#description' => t('Should rich text editing be enabled or disabled by default in textarea fields? If disabled, the rich text editor may still be enabled by using toggle or popup window.'),
      );

      $form['ckeditor']['ckeditor_show_toggle'] = array(
        '#type' => 'radios',
        '#title' => t('Show the disable/enable rich text editor toggle'),
        '#default_value' => isset($user->ckeditor_show_toggle) ? $user->ckeditor_show_toggle : (isset($profile->settings['show_toggle']) ? $profile->settings['show_toggle'] : 't'),
        '#options' => array(
          't' => t('Yes'),
          'f' => t('No')
        ),
        '#description' => t('Whether or not to show the disable/enable rich text editor toggle below the textarea. Works only if CKEditor is not running in a popup window (see below).'),
      );

      if (user_access('administer ckeditor')) {
        $form['ckeditor']['ckeditor_show_fieldnamehint'] = array(
          '#type' => 'radios',
          '#title' => t('Show field name hint below each rich text editor'),
          '#default_value' => !empty($user->ckeditor_show_fieldnamehint) ? $user->ckeditor_show_fieldnamehint : 't',
          '#options' => array(
            't' => t('Yes'),
            'f' => t('No')
          ),
        );
      }

      $form['ckeditor']['ckeditor_popup'] = array(
        '#type' => 'radios',
        '#title' => t('Use CKEditor in a popup window'),
        '#default_value' => isset($user->ckeditor_popup) ? $user->ckeditor_popup : (isset($profile->settings['popup']) ? $profile->settings['popup'] : 'f'),
        '#options' => array(
          'f' => t('No'),
          't' => t('Yes')
        ),
        '#description' => t('If this option is enabled, a link to a popup window will be used instead of a textarea replace.'),
      );

      $form['ckeditor']['ckeditor_width'] = array(
        '#type' => 'textfield',
        '#title' => t('Editor width'),
        '#default_value' => isset($user->ckeditor_width) ? $user->ckeditor_width : (isset($profile->settings['width']) ? $profile->settings['width'] : '100%'),
        '#description' => t('Editor interface width in pixels or percent.') .' '. t('Examples') .': 400 '. t('or') .' 100%.',
        '#size' => 40,
        '#maxlength' => 128,
      );

      $form['ckeditor']['ckeditor_lang'] = array(
        '#type' => 'select',
        '#title' => t('Language'),
        '#default_value' => isset($user->ckeditor_lang) ? $user->ckeditor_lang : (isset($profile->settings['lang']) ? $profile->settings['lang'] : 'en'),
        '#options' => $lang_options,
        '#description' => t('The language for the CKEditor interface.')
      );

      $form['ckeditor']['ckeditor_auto_lang'] = array(
        '#type' => 'radios',
        '#title' => t('Auto-detect language'),
        '#default_value' => isset($user->ckeditor_auto_lang) ? $user->ckeditor_auto_lang : (isset($profile->settings['auto_lang']) ? $profile->settings['auto_lang'] : 't'),
        '#options' => array(
          't' => t('Yes'),
          'f' => t('No')
        ),
        '#description' => t('Automatically detect the user language.')
      );

      return array('ckeditor' => $form);
    }
  }

  if ($type == 'validate') {
    if (isset($edit['ckeditor_default'], $edit['ckeditor_popup']) && $edit['ckeditor_default'] == 't' && $edit['ckeditor_popup'] == 't') {
      form_set_error('ckeditor_popup', t('If CKEditor is enabled by default, the popup window must be disabled.'));
    }

    if (isset($edit['ckeditor_show_toggle'], $edit['ckeditor_popup']) && $edit['ckeditor_show_toggle'] == 't' && $edit['ckeditor_popup'] == 't') {
      form_set_error('ckeditor_popup', t('If toggle is enabled, the popup window must be disabled.'));
    }

    if (isset($edit['ckeditor_width']) && !preg_match('/^\d+%?$/', $edit['ckeditor_width'])) {
      form_set_error('ckeditor_width', t('Enter a valid width value.') .' '. t('Example') .': 400 '. t('or') .' 100%.');
    }
  }

  return NULL;
}
