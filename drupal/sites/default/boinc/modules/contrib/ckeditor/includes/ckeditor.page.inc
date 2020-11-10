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

function ckeditor_help_delegate($path, $arg) {
  switch ($path) {
    case 'admin/settings/help#description':
      $output = t('Enables the use of CKEditor (a rich text WYSIWYG editor) instead of plain text fields.');
      break;

    case 'admin/settings/ckeditor/edit/%':
    case 'admin/settings/ckeditor/add':
      $output = '<p>'. t('CKEditor is highly configurable. The most commonly used features are listed below. You can also adjust CKEditor to your needs by changing the !ckeditor_module_config configuration file.',
        array(
          '!ckeditor_module_config' => '<code>' . drupal_get_path('module', 'ckeditor') .'/ckeditor.config.js</code>',
        )) .
      '</p><p>' .
      '<p>' .
      t('It is recommended to not edit the !ckeditor_config_file (!ckeditor_config_path) configuration file that is distributed with CKEditor, because you may overwrite it accidentally when you update the editor.',
        array(
          '!ckeditor_config_path' => '<code>' . drupal_get_path('module', 'ckeditor') .'/ckeditor/config.js</code>',
          '!ckeditor_config_file' => '<code>config.js</code>',
        )) .'</p>';
      break;

    case 'admin/settings/ckeditor/editg':
    case 'admin/settings/ckeditor/add':
      $output = '<p>' . t('The Global Profile allows you to define settings that are common for all profiles. Values defined in other profiles will be appended to the global configuration. This way you can avoid repeating some of the settings that are usually the same for each profile.') . '</p>';
      break;

    case 'admin/settings/ckeditor':
      $output =
        '<p>'. t('The CKEditor module allows Drupal to replace textarea fields with CKEditor. CKEditor is an online rich text editor that can be embedded inside web pages. It is a !wysiwyg editor which means that the text edited in it looks as similar as possible to the results end users will see after the document gets published. It brings to the Web popular editing features found in desktop word processors such as Microsoft Word and OpenOffice.org Writer. CKEditor is truly lightweight and does not require any kind of installation on the client computer.',
          array(
            '!wysiwyg' => '<acronym title="'. t('What You See Is What You Get') .'">'. t('WYSIWYG') .'</acronym>',
          )
        ) .
        '</p><p>' .
        t('More information about the editor can be found at the !ckeditorlink. A handy !userguidelink is available, too.',
          array(
            '!ckeditorlink' => l(t('CKEditor website'), 'http://ckeditor.com'),
            '!userguidelink' => l(t('User\'s Guide'), 'http://docs.cksource.com/CKEditor_3.x/Users_Guide'),
          )
        ) .
        '</p><p>' .
        t('Profiles can be defined based on user roles. A CKEditor profile may define which pages receive the CKEditor support, which buttons or themes are enabled for the editor, how the editor is displayed, and a few other editor functions. It is also possible to define the Global Profile that will store values which will be appended to all other profiles.') .
        '</p><p>' .
        t('Please note that only users with the "%accesspermission" !permissionlink will be able to use CKEditor.',
          array(
            '%accesspermission' => t('access ckeditor'),
            '!permissionlink' => l(t('permission'), 'admin/user/permissions')
          )
        ) .
        '</p>';
      break;

    case 'admin/help#ckeditor':
      $output =
        '<h3>' . t('Introduction') . '</h3>' .
        '<p>' .
        t('The CKEditor module allows Drupal to replace textarea fields with CKEditor. CKEditor is an online rich text editor that can be embedded inside web pages. It is a !wysiwyg editor which means that the text edited in it looks as similar as possible to the results end users will see after the document gets published. It brings to the Web popular editing features found in desktop word processors such as Microsoft Word and OpenOffice.org Writer. CKEditor is truly lightweight and does not require any kind of installation on the client computer.',
          array(
            '!wysiwyg' => '<acronym title="' . t('What You See Is What You Get') . '">' . t('WYSIWYG') . '</acronym>',
          )
        ) .
        '</p><p>' .
        t('More information about the editor can be found at the !ckeditorlink. A handy !userguidelink is available, too.',
          array(
            '!ckeditorlink' => l(t('CKEditor website'), 'http://ckeditor.com'),
            '!userguidelink' => l(t('User\'s Guide'), 'http://docs.cksource.com/CKEditor_3.x/Users_Guide'),
          )
        ) .
        '</p>' .
        '<h4>' . t('Configuration') . '</h4>' .
        '<ol>' .
        '<li>'.
        t('To start using CKEditor, first you need to grant permissions for use of CKEditor in the <strong>!adminpath</strong> section.',
          array(
            '!adminpath' => l(t('Administer') .' > '. t('User management') .' > '. t('Permissions'), 'admin/user/permissions'),
          )
        ) .
        '</li>' .
        '<li>' .
        t('CKEditor profiles can be configured in the <strong>!adminpath</strong> section. In each profile you can choose which textareas will be replaced by CKEditor as well as select the default toolbar and configure some more advanced settings.',
          array(
            '!adminpath' => l(t('Administer') .' > '. t('Site configuration') .' > '. t('CKEditor'), 'admin/settings/ckeditor'),
          )
        ) .
        '</li>' .
        '<li>' .
        t('For the Rich Text Editing to work you also need to configure your !filterlink for the users that may access Rich Text Editing. Either grant those users <strong>Full HTML</strong> access or use the following list of tags in the HTML filter:',
          array(
            '!filterlink' => l(t('filters'), 'admin/settings/filters'),
          )
        ) .
       '<br /><code>'. htmlspecialchars('<a> <p> <span> <div> <h1> <h2> <h3> <h4> <h5> <h6> <img> <map> <area> <hr> <br> <br /> <ul> <ol> <li> <dl> <dt> <dd> <table> <tbody> <th> <tr> <td> <em> <b> <u> <i> <strong> <del> <ins> <sub> <sup> <quote> <blockquote> <pre> <address> <code> <cite> <embed> <object> <param> <strike> <caption> <iframe>') .'</code><br />';
       $output .= t('<strong>Note:</strong> be careful when granting users access to create tags like %iframe. <br />If you are going to use CKEditor with the <strong>Filtered HTML</strong> input format, please read  the "Setting up filters" section in the !readme file.',
         array(
           '!readme' => '<code>' . l('README.txt', drupal_get_path('module', 'ckeditor') .'/README.txt') . '</code>',
           '%iframe' => '<iframe>'
        )
      ) .
      '</li>' .
      '<li>'.
      t('To have better control over line breaks, you should disable the <strong>%settingname</strong> setting in the chosen filter (recommended).',
        array(
          '%settingname' => t('Line break converter'),
        )
      ) .
      '</li>' .
      '<li>' .
      t('All configuration options described in the !apidocs that cannot be easily changed in the administration area can be set in the <strong>Advanced Options</strong> section in the CKEditor profile.',
        array(
          '!apidocs' => l(t('API documentation'), 'http://docs.cksource.com/ckeditor_api/symbols/CKEDITOR.config.html')
        )
      ) .
      '</li>' .
      '</ol>' .

      '<a name="fieldinclexcl"></a>' .
      '<h3>' . t('Defining field inclusions/exclusions') .'</h3>' .
      '<p>' .
      t('In order to display CKEditor in some specific textareas where you need it, the CKEditor module provides a powerful method to define fields that should be enhanced with CKEditor. The first choice you have to make is whether you want to display CKEditor for all textareas and define a list of exceptions. You can also decide that you do not want to display CKEditor unless it is specifically enabled. This choice is made in the <strong>%settingname</strong> setting.',
        array(
          '%settingname' => t('Use inclusion or exclusion mode'),
        )
      ) .
      '</p>' .
      '<p>'.
      t('Next, you need to define the list of exceptions. This list follows a specific syntax: the path to a field is defined by giving the content type followed by the <code>@</code> character, specifying the path followed by a dot and finally appending the field name at the end. You may decide to skip the content type. The !fieldname field on the !pathname page would thus be addressed as !result. You can use wildcards <code>*</code> and <code>?</code> anywhere in this line. If you have trouble with the field name that contains a dot, you can escape it with a backslash character (<code>\</code>).',
        array(
          '!fieldname' => '<code>edit-log</code>',
          '!pathname' => '<code>node/add/page</code>',
          '!result' => '<code>node/add/page.edit-log</code>',
        )
      ) .
      '</p>' .
      '<strong>' .
      t('Some examples:') .
      '</strong>' .
      '<ul>' .
      '<li><code>node/add/page</code>: ' .
      t('matches all fields on the !pathname page',
        array(
          '!pathname' => '<code>node/add/page</code>'
        )
      ) .
      '</li>' .
      '<li><code>*.edit-log</code>: ' .
      t('matches all fields called !fieldname on any page',
        array(
          '!fieldname' => '<code>edit-log</code>'
        )
      ) .
      '</li>' .
      '<li><code>blog@*.edit-body</code>: ' .
      t('matches all fields called !fieldname on pages related to node type !typename',
        array(
          '!fieldname' => '<code>edit-body</code>',
          '!typename' => '<code>blog</code>'
        )
      ) .
      '</li>' .
      '<li><code>forum@*.*</code>: ' .
      t('matches all fields on pages related to node type !nodetype',
        array(
          '!nodetype' => '<code>forum</code>'
        )
      ) .
      '</li>' .
      '<li><code>node/add/*</code>: ' .
      t('matches all fields on pages such as !pathname1, !pathname2, etc.',
        array(
          '!pathname1' => '<code>node/add/page</code>',
          '!pathname2' => '<code>node/add/story</code>'
        )
      ) .
      '</li>' .
      '<li><code>node/add/*.edit-log</code>: ' .
      t('matches all !fieldname fields on pages such as !pathname1, !pathname2, etc.',
        array(
          '!fieldname' => '<code>edit-log</code>',
          '!pathname1' => '<code>node/add/page</code>',
          '!pathname2' => '<code>node/add/story</code>'
        )
      ) .
      '</li>' .
      '<li><code>node/add/*.edit-user-*</code>: ' .
      t('matches fields starting with !fieldname on pages starting with !pathname',
        array(
          '!fieldname' => '<code>edit-user-</code>',
          '!pathname' => '<code>node/add/</code>'
        )
      ) .
      '</li>' .
      '</ul>' .
      '<h3>' . t('Troubleshooting') . '</h3>' .
      t('Take a look at !listlink when installing CKEditor.',
        array(
           '!listlink' => l(t('the list of common problems'), 'http://docs.cksource.com/CKEditor_for_Drupal/Troubleshooting')
        )
      ) .
      ' ' .
      t('If you are looking for more information, have any trouble with the configuration, or found an issue with the CKEditor module, please visit the !officiallink.',
        array(
          '!officiallink' => l(t('official project page'), 'http://drupal.org/project/ckeditor')
        )
      ) .
      ' ' .
      t('More information about how to customize CKEditor for your theme can be found !herelink.',
        array(
          '!herelink' => l(t('here'), 'http://drupal.fckeditor.net/tricks')
        )
      ) .
      '</p>' .

      '<h3>' . t('Uploading images and files') . '</h3>' .
      '<p>' .
      t('There are three ways for uploading files:') .
      '</p>' .
      '<ol>' .
      '<li>' .
      t('By using !ckfinder (commercial), an advanced Ajax file manager.',
        array(
          '!ckfinder' => l(t('CKFinder'), 'http://cksource.com/ckfinder'),
        )
      ) .
      '</li>' .
      '<li>' .
      t('By using a dedicated module like !imcelink, !iblink or !webfmlink.',
        array(
          '!imcelink' => l(t('IMCE'), 'http://drupal.org/project/imce'),
          '!iblink' => l(t('Image Browser'), 'http://drupal.org/project/imagebrowser'),
          '!webfmlink' => l(t('Web File Manager'), 'http://drupal.org/project/webfm'),
        )
      ) .
      '</li>' .
      '<li>' .
      t('By using the core upload module.') .
      '</li>' .
      '</ol>';

      break;
  }
  return !empty($output) ? $output : '';
}

/**
 * AJAX callback - XSS filter
 */
function ckeditor_filter_xss() {
  global $user, $theme;
  header('Content-Type: text/plain; charset=utf-8');

  $GLOBALS['devel_shutdown'] = FALSE;

  if (!isset($_POST['text']) || !is_string($_POST['text']) || !isset($_POST['textarea_id']) || !is_string($_POST['textarea_id']) || !isset($_POST['query']) || !is_string($_POST['query']) || !isset($_POST['theme']) || !is_string($_POST['theme']) || !isset($_POST['input_format']) || !isset($_POST['token']) || !drupal_valid_token($_POST['token'], 'ckeditorAjaxCall', FALSE)) {
    exit;
  }
  $filter_format_id = filter_resolve_format((int) $_POST['input_format']);
  if (!filter_access($filter_format_id)) {
    exit;
  }
  module_load_include('inc', 'ckeditor', 'includes/ckeditor.lib');
  $theme = $_POST['theme'];
  $profile = ckeditor_user_get_profile($user, $_POST['textarea_id'], $_POST['query']);
  if ($profile == FALSE) {
    exit;
  }

  $settings = $profile->settings;

  $text = $_POST['text'];
  $text = strtr($text, array('<!--' => '__COMMENT__START__', '-->' => '__COMMENT__END__'));

  $filters = array();
  foreach ($settings['filters'] as $module_delta => $active) {
    if (!$active || !is_string($module_delta)) {
      continue;
    }
    $filter = new stdClass();
    $filter->module = strtok($module_delta, "/");
    $filter->delta = strtok("/");
    $filter->format = $filter_format_id;

    if (!module_hook($filter->module, 'filter')) {
      continue;
    }

    $filters[] = $filter;

    $text = module_invoke($filter->module, 'filter', 'prepare', $filter->delta, $filter->format, $text);
  }

  foreach ($filters as $filter) {
    // Built-in filter module, a special case where we would like to strip XSS and nothing more
    if ($filter->module == 'filter' && $filter->delta == 0) {
      preg_match_all("|</?([a-z][a-z0-9]*)(?:\b[^>]*)>|i", $text, $matches);
      if ($matches[1]) {

        // Sources of inspiration:
        // http://www.w3.org/TR/html4/index/elements.html
        // http://www.w3.org/TR/html-markup/elements.html
        // https://developer.mozilla.org/en-US/docs/Web/HTML/Element

        $base_allowed_tags = array('a','abbr','acronym','address','area','article','aside','audio','b','base','basefont',
          'bdi','bdo','big','blockquote','body','br','button','canvas','caption','center','cite','code','col','colgroup',
          'command','datalist','dd','del','details','dfn','dialog','dir','div','dl','dt','em','fieldset','figcaption',
          'figure','font','footer','form','h1','h2','h3','h4','h5','h6','head','header','hgroup','hr','html','i','img',
          'input','ins','isindex','kbd','keygen','label','legend','li','main','map','mark','menu','menuitem','meter',
          'nav','noframes','noscript','ol','optgroup','option','output','p','param','pre','progress','q','rp','rt',
          'ruby','s','samp','section','select','small','source','span','strike','strong','sub','summary','sup','table',
          'tbody','td','textarea','tfoot','th','thead','time','title','tr','track','tt','u','ul','var','video','wbr',
        );

        // Get tags allowed in filter settings
        $filter_allowed_tags = preg_split('/\s+|<|>/', variable_get("allowed_html_$filter_format_id", ''), -1, PREG_SPLIT_NO_EMPTY);

        // Combine allowed tags
        $tags = array_merge($base_allowed_tags, $filter_allowed_tags);

        // Tags provided by hook
        $hooks_allowed_tags = module_invoke_all('ckeditor_filter_xss_allowed_tags');
        if ( !empty($hooks_allowed_tags) && is_array($hooks_allowed_tags) ){
          foreach($hooks_allowed_tags as $tag ){
            if ( !empty($tag) && is_string($tag) && !in_array($tag,$tags) ){
              array_push($tags,$tag);
            }
          }
        }
        $text = filter_xss($text, $tags);
      }
    }
    else {
      $text = module_invoke($filter->module, 'filter', 'process', $filter->delta, $filter->format, $text);
    }
  }

  $text = strtr($text, array('__COMMENT__START__' => '<!--', '__COMMENT__END__' => '-->'));

  echo $text;
  exit;
}
