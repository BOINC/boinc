<?php

/**
 * Implementation of hook_input_formats().
 */
function boinc_standard_input_formats() {
  $input_formats = array();

  $input_format = new stdClass;
  $input_format->api_version = 1;
  $input_format->name = 'PHP code';
  $input_format->roles = array();
  $input_format->cache = FALSE;
  $input_format->settings = array(
  );
  $input_format->filters = array(
    'php' => array(
      '0' => '0',
    ),
  );
  $input_format->machine = 'php_code';
  $input_formats['php_code'] = $input_format;

  $input_format = new stdClass;
  $input_format->api_version = 1;
  $input_format->name = 'Rich text';
  $input_format->roles = array(
    1 => 'anonymous user',
    2 => 'authenticated user',
    3 => 'administrator',
    4 => 'content editor',
    5 => 'moderator',
  );
  $input_format->cache = TRUE;
  $input_format->settings = array(
    // Settings for: tableofcontents module
    'tableofcontents' => array(
      'tableofcontents_hide_table' => 0,
      'tableofcontents_automatic' => '0',
      'tableofcontents_min_limit' => '5',
      'tableofcontents_remove_teaser' => 1,
      'tableofcontents_allow_override' => 1,
      'tableofcontents_safe_title' => 1,
      'tableofcontents_title' => 'Table of Contents',
      'tableofcontents_minlevel' => '2',
      'tableofcontents_maxlevel' => '2',
      'tableofcontents_hide_show' => 0,
      'tableofcontents_collapsed' => 0,
      'tableofcontents_id_stripping' => array(
          'digits' => 0,
          'dashes' => 0,
          'periods' => 0,
          'underscores' => 0,
          'colons' => 0,
        ),
      'tableofcontents_identifier_introducer' => 'header',
      'tableofcontents_id_separator' => '-',
      'tableofcontents_id_generator' => 'title',
      'tableofcontents_allowed_tags' => '<em> <i> <strong> <b> <u> <del> <ins> <sub> <sup> <cite> <strike> <s> <tt> <span> <font> <abbr> <acronym> <dfn> <q> <bdo> <big> <small>',
      'tableofcontents_comments' => 0,
      'tableofcontents_comments_level' => '3',
      'tableofcontents_back_to_top' => 'Back to top',
      'tableofcontents_back_to_top_location' => 'bottom',
      'tableofcontents_back_to_top_minlevel' => '2',
      'tableofcontents_back_to_top_maxlevel' => '2',
      'tableofcontents_back_to_top_anchor' => 'toc',
      'tableofcontents_scroll_back_to_top' => 0,
      'tableofcontents_numbering' => '4',
      'tableofcontents_number_headers' => 0,
      'tableofcontents_number_mode' => '0',
      'tableofcontents_number_start_letter' => '',
      'tableofcontents_number_separator' => '.',
      'tableofcontents_number_end_letter' => '.',
    ),
    // Settings for: bbcode module
    'bbcode' => array(
      'bbcode_make_links' => '1',
      'bbcode_filter_nofollow' => '0',
      'bbcode_encode_mailto' => '1',
      'bbcode_paragraph_breaks' => '2',
      'bbcode_debug' => '0',
    ),
    // Settings for: pathologic module
    'pathologic' => array(
      'filter_pathologic_local_paths' => '/',
      'filter_pathologic_absolute' => 1,
    ),
  );
  $input_format->filters = array(
    'tableofcontents' => array(
      '0' => '-10',
    ),
    'bbcode' => array(
      '0' => '-9',
    ),
    'pathologic' => array(
      '0' => '-8',
    ),
  );
  $input_format->machine = 'rich_text';
  $input_formats['rich_text'] = $input_format;

  return $input_formats;

}
