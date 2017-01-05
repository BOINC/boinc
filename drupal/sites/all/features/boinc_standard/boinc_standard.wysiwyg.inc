<?php

/**
 * Implementation of hook_default_wysiwyg().
 */
function boinc_standard_default_wysiwyg() {
  $export = array();

  $wysiwyg = new stdClass;
  $wysiwyg->disabled = FALSE; /* Edit this to true to make a default wysiwyg disabled initially */
  $wysiwyg->api_version = 2;
  $wysiwyg->editor = 'tinymce';
  $wysiwyg->settings = array(
    'default' => 1,
    'user_choose' => 1,
    'show_toggle' => 1,
    'theme' => 'advanced',
    'language' => 'en',
    'buttons' => array(
      'default' => array(
        'bold' => 1,
        'italic' => 1,
        'underline' => 1,
        'strikethrough' => 1,
        'justifyleft' => 1,
        'justifycenter' => 1,
        'justifyright' => 1,
        'justifyfull' => 1,
        'bullist' => 1,
        'numlist' => 1,
        'outdent' => 1,
        'indent' => 1,
        'undo' => 1,
        'redo' => 1,
        'link' => 1,
        'image' => 1,
        'sup' => 1,
        'sub' => 1,
        'blockquote' => 1,
        'hr' => 1,
        'removeformat' => 1,
        'charmap' => 1,
      ),
      'advimage' => array(
        'advimage' => 1,
      ),
      'emotions' => array(
        'emotions' => 1,
      ),
      'font' => array(
        'formatselect' => 1,
      ),
      'paste' => array(
        'pastetext' => 1,
      ),
      'media' => array(
        'media' => 1,
      ),
      'drupal' => array(
        'linebreaks' => 1,
      ),
    ),
    'toolbar_loc' => 'top',
    'toolbar_align' => 'left',
    'path_loc' => 'bottom',
    'resizing' => 1,
    'verify_html' => 1,
    'preformatted' => 0,
    'convert_fonts_to_spans' => 1,
    'remove_linebreaks' => 1,
    'apply_source_formatting' => 0,
    'paste_auto_cleanup_on_paste' => 1,
    'block_formats' => 'p,code,pre,h2,h3,h4,h5,h6,div,address',
    'css_setting' => 'theme',
    'css_path' => '',
    'css_classes' => 'Image with caption=standalone-image
Float left with caption=image-left
Float right with caption=image-right',
  );
  $wysiwyg->machine = 'rich_text';
  $export['rich_text'] = $wysiwyg;

  return $export;
}
