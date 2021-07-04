<?php

/**
 * Implementation of hook_input_formats().
 */
function boinc_standard_input_formats() {
  $input_formats = array();

  $input_format = new stdClass;
  $input_format->api_version = 1;
  $input_format->name = 'Rich text';
  $input_format->roles = array(
    1 => 'anonymous user',
    2 => 'authenticated user',
    3519698132 => 'administrator',
    1110965548 => 'community member',
    2938987599 => 'content editor',
    268174006 => 'forum control (system use only)',
    1271379760 => 'moderator',
    933038561 => 'verified contributor',
  );
  $input_format->cache = FALSE;
  $input_format->settings = array(
    // Settings for: boinctranslate module
    'boinctranslate' => array(
      // Print debugging info
      'boinctranslate_filter_debug' => '0',
    ),
    // Settings for: pathologic module
    'pathologic' => array(
      // Also considered local
      'filter_pathologic_local_paths' => '/
',
      // Output full absolute URLs
      'filter_pathologic_absolute' => 1,
    ),
    // Settings for: bbcode module
    'bbcode' => array(
      // Convert addresses to links
      'bbcode_make_links' => '1',
      // Spam link deterrent
      'bbcode_filter_nofollow' => '0',
      // Email address encoding
      'bbcode_encode_mailto' => '0',
      // Email address rewrite
      'bbcode_rewrite_email' => '1',
      // Smart paragraph and line breaks
      'bbcode_paragraph_breaks' => '2',
      // Print debugging info
      'bbcode_debug' => '0',
    ),
    // Settings for: htmlpurifier module
    'htmlpurifier' => array(
      'htmlpurifier_clear_cac' => array(
          '#description' => NULL,
          '#value' => 'Clear cache (Warning: Can result in performance degradation)',
        ),
      // Display help text
      'htmlpurifier_help' => 1,
      'htmlpurifier_config' => array(
          '#description' => NULL,
          '#value' => array(
            'Attr.EnableID' => '0',
            'AutoFormat.AutoParagraph' => '1',
            'AutoFormat.Linkify' => '1',
            'AutoFormat.RemoveEmpty' => '0',
            'Null_HTML.Allowed' => '1',
            'HTML.ForbiddenAttributes' => '',
            'HTML.ForbiddenElements' => '',
            'HTML.SafeObject' => '0',
            'Output.FlashCompat' => '0',
            'URI.DisableExternalResources' => '0',
            'URI.DisableResources' => '0',
            'Null_URI.Munge' => '1',
          ),
        ),
    ),
    // Settings for: tableofcontents module
    'tableofcontents' => array(
      // Hide the table of contents tags
      'tableofcontents_hide_table' => 0,
      // Whether an automatic table of content should be added
      'tableofcontents_automatic' => '0',
      // Number of headers before an automatic table of content is added
      'tableofcontents_min_limit' => '5',
      // Remove Table of Contents tags from teasers
      'tableofcontents_remove_teaser' => 1,
      // Allow users to override the settings within the table of contents tag itself
      'tableofcontents_allow_override' => 1,
      // Ensure title is safe (i.e. use check_plain() to avoid XSS attacks.)
      'tableofcontents_safe_title' => 1,
      // Table of Contents Title
      'tableofcontents_title' => 'Table of Contents',
      // Minimum heading level
      'tableofcontents_minlevel' => '2',
      // Maximum heading level
      'tableofcontents_maxlevel' => '3',
      // Include link to hide/show table of contents
      'tableofcontents_hide_show' => 1,
      // Start with the table of content collapsed
      'tableofcontents_collapsed' => 0,
      // Select what is stripped from the header titles
      'tableofcontents_id_stripping' => array(
          'digits' => 0,
          'dashes' => 0,
          'periods' => 0,
          'underscores' => 0,
          'colons' => 0,
        ),
      // Identifier introducer
      'tableofcontents_identifier_introducer' => 'header',
      // Identifier and number separator
      'tableofcontents_id_separator' => '-',
      // How to generate missing header identifiers
      'tableofcontents_id_generator' => 'title',
      // List of tags allowed in table headers
      'tableofcontents_allowed_tags' => '<em> <i> <strong> <b> <u> <del> <ins> <sub> <sup> <cite> <strike> <s> <tt> <span> <font> <abbr> <acronym> <dfn> <q> <bdo> <big> <small>',
      // Show attachments in the table of contents
      'tableofcontents_attachments' => 0,
      // Add the comments to the table of contents
      'tableofcontents_comments' => 0,
      // Select header level at which comments start
      'tableofcontents_comments_level' => '3',
      // Back to top label
      'tableofcontents_back_to_top' => '',
      // Back to top location
      'tableofcontents_back_to_top_location' => 'bottom',
      // Minimum level where Back to Top appears
      'tableofcontents_back_to_top_minlevel' => '2',
      // Maximum level where Back to Top appears
      'tableofcontents_back_to_top_maxlevel' => '4',
      // Back to top anchor
      'tableofcontents_back_to_top_anchor' => 'toc',
      // Scroll back to the table of contents
      'tableofcontents_scroll_back_to_top' => 0,
      // Numbering method
      'tableofcontents_numbering' => '0',
      // Add the number to the headers
      'tableofcontents_number_headers' => 0,
      // Numbering mode
      'tableofcontents_number_mode' => '0',
      // Numbering prefix
      'tableofcontents_number_start_letter' => '',
      // Numbering separator
      'tableofcontents_number_separator' => '.',
      // Numbering suffix
      'tableofcontents_number_end_letter' => '.',
    ),
  );
  $input_format->filters = array(
    'boinctranslate' => array(
      // Delta => Weight for: BOINC translation filter to help split content into chunks for translations.
      '0' => '-10',
    ),
    'pathologic' => array(
      // Delta => Weight for: Pathologic
      '0' => '-9',
    ),
    'bbcode' => array(
      // Delta => Weight for: BBCode
      '0' => '-8',
    ),
    'htmlpurifier' => array(
      // Delta => Weight for: HTML Purifier
      '0' => '-7',
    ),
    'tableofcontents' => array(
      // Delta => Weight for: Table of contents
      '0' => '-6',
    ),
    'image_caption_filter' => array(
      // Delta => Weight for: Image caption filter
      '0' => '-5',
    ),
  );
  $input_format->machine = 'rich_text';
  $input_formats['rich_text'] = $input_format;

  $input_format = new stdClass;
  $input_format->api_version = 1;
  $input_format->name = 'Plain text';
  $input_format->roles = array(
    3519698132 => 'administrator',
  );
  $input_format->cache = FALSE;
  $input_format->settings = array(
  );
  $input_format->filters = array(
    'boinctranslate' => array(
      '0' => '10',
    ),
    'filter' => array(
      '0' => '10',
    ),
  );
  $input_format->machine = 'plain_text';
  $input_formats['plain_text'] = $input_format;

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

  return $input_formats;

}
