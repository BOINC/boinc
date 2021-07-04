<?php

/**
 * Implementation of hook_strongarm().
 */
function boinc_standard_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'admin_theme';
  $strongarm->value = 'garland';
  $export['admin_theme'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_debug_4';
  $strongarm->value = '0';
  $export['bbcode_debug_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_debug_6';
  $strongarm->value = '0';
  $export['bbcode_debug_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_encode_mailto_4';
  $strongarm->value = '0';
  $export['bbcode_encode_mailto_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_encode_mailto_6';
  $strongarm->value = '0';
  $export['bbcode_encode_mailto_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_filter_nofollow_4';
  $strongarm->value = '0';
  $export['bbcode_filter_nofollow_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_filter_nofollow_6';
  $strongarm->value = '0';
  $export['bbcode_filter_nofollow_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_make_links_4';
  $strongarm->value = '1';
  $export['bbcode_make_links_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_make_links_6';
  $strongarm->value = '1';
  $export['bbcode_make_links_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_paragraph_breaks_4';
  $strongarm->value = '2';
  $export['bbcode_paragraph_breaks_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_paragraph_breaks_6';
  $strongarm->value = '2';
  $export['bbcode_paragraph_breaks_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_rewrite_email_4';
  $strongarm->value = '1';
  $export['bbcode_rewrite_email_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bbcode_rewrite_email_6';
  $strongarm->value = '1';
  $export['bbcode_rewrite_email_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'boinctranslate_filter_debug_4';
  $strongarm->value = '0';
  $export['boinctranslate_filter_debug_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'boinctranslate_filter_nodetypes';
  $strongarm->value = array(
    'page' => 'page',
    'story' => 'story',
    'news' => 0,
  );
  $export['boinctranslate_filter_nodetypes'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_anonymous_page';
  $strongarm->value = 0;
  $export['comment_anonymous_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_controls_page';
  $strongarm->value = '3';
  $export['comment_controls_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_mode_page';
  $strongarm->value = '4';
  $export['comment_default_mode_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_order_page';
  $strongarm->value = '1';
  $export['comment_default_order_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_default_per_page_page';
  $strongarm->value = '50';
  $export['comment_default_per_page_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_form_location_page';
  $strongarm->value = '0';
  $export['comment_form_location_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_page';
  $strongarm->value = 0;
  $export['comment_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_preview_page';
  $strongarm->value = '1';
  $export['comment_preview_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'comment_subject_field_page';
  $strongarm->value = '1';
  $export['comment_subject_field_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'content_extra_weights_page';
  $strongarm->value = array(
    'apachesolr' => '100',
    'title' => '-5',
    'body_field' => '0',
    'revision_information' => '20',
    'author' => '20',
    'options' => '25',
    'comment_settings' => '30',
    'language' => '0',
    'translation' => '30',
    'menu' => '-2',
    'path' => '30',
    'attachments' => '30',
  );
  $export['content_extra_weights_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'error_level';
  $strongarm->value = '0';
  $export['error_level'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'filter_default_format';
  $strongarm->value = '4';
  $export['filter_default_format'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'filter_pathologic_absolute_4';
  $strongarm->value = 1;
  $export['filter_pathologic_absolute_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'filter_pathologic_absolute_6';
  $strongarm->value = 1;
  $export['filter_pathologic_absolute_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'filter_pathologic_local_paths_4';
  $strongarm->value = '/
';
  $export['filter_pathologic_local_paths_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'filter_pathologic_local_paths_6';
  $strongarm->value = '/';
  $export['filter_pathologic_local_paths_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'format';
  $strongarm->value = '4';
  $export['format'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_clear_cache';
  $strongarm->value = 'Clear cache (Warning: Can result in performance degradation)';
  $export['htmlpurifier_clear_cache'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_config_4';
  $strongarm->value = array(
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
  );
  $export['htmlpurifier_config_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_help_4';
  $strongarm->value = 1;
  $export['htmlpurifier_help_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_help_6';
  $strongarm->value = 1;
  $export['htmlpurifier_help_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_version_check_failed';
  $strongarm->value = FALSE;
  $export['htmlpurifier_version_check_failed'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_version_current';
  $strongarm->value = '4.9.1';
  $export['htmlpurifier_version_current'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'htmlpurifier_version_ours';
  $strongarm->value = '4.10.0';
  $export['htmlpurifier_version_ours'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imce_profiles';
  $strongarm->value = array(
    1 => array(
      'name' => 'User-1',
      'usertab' => 1,
      'filesize' => 0,
      'quota' => 0,
      'tuquota' => 0,
      'extensions' => '*',
      'dimensions' => '1200x1200',
      'filenum' => 0,
      'directories' => array(
        0 => array(
          'name' => '.',
          'subnav' => 1,
          'browse' => 1,
          'upload' => 1,
          'thumb' => 1,
          'delete' => 1,
          'resize' => 1,
        ),
      ),
      'thumbnails' => array(
        0 => array(
          'name' => 'Small',
          'dimensions' => '90x90',
          'prefix' => 'small_',
          'suffix' => '',
        ),
        1 => array(
          'name' => 'Medium',
          'dimensions' => '120x120',
          'prefix' => 'medium_',
          'suffix' => '',
        ),
        2 => array(
          'name' => 'Large',
          'dimensions' => '180x180',
          'prefix' => 'large_',
          'suffix' => '',
        ),
      ),
    ),
    2 => array(
      'name' => 'Sample profile',
      'usertab' => 1,
      'filesize' => 1,
      'quota' => 2,
      'tuquota' => 0,
      'extensions' => 'gif png jpg jpeg',
      'dimensions' => '800x600',
      'filenum' => 1,
      'directories' => array(
        0 => array(
          'name' => 'u%uid',
          'subnav' => 0,
          'browse' => 1,
          'upload' => 1,
          'thumb' => 1,
          'delete' => 0,
          'resize' => 0,
        ),
      ),
      'thumbnails' => array(
        0 => array(
          'name' => 'Thumb',
          'dimensions' => '90x90',
          'prefix' => 'thumb_',
          'suffix' => '',
        ),
      ),
    ),
    3 => array(
      'name' => 'Editor',
      'usertab' => 1,
      'filesize' => '1',
      'quota' => '2',
      'tuquota' => '0',
      'extensions' => 'gif png jpg jpeg',
      'dimensions' => '800x600',
      'filenum' => '1',
      'directories' => array(
        0 => array(
          'name' => 'u%uid',
          'subnav' => 0,
          'browse' => 1,
          'upload' => 1,
          'thumb' => 1,
          'delete' => 0,
          'resize' => 0,
        ),
      ),
      'thumbnails' => array(
        0 => array(
          'name' => 'Small',
          'dimensions' => '90x90',
          'prefix' => 'small_',
          'suffix' => '',
        ),
        1 => array(
          'name' => 'Medium',
          'dimensions' => '120x120',
          'prefix' => 'medium_',
          'suffix' => '',
        ),
        2 => array(
          'name' => 'Large',
          'dimensions' => '180x180',
          'prefix' => 'large_',
          'suffix' => '',
        ),
      ),
    ),
  );
  $export['imce_profiles'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imce_roles_profiles';
  $strongarm->value = array(
    3519698132 => array(
      'weight' => '0',
      'pid' => '0',
    ),
    1110965548 => array(
      'weight' => '0',
      'pid' => '0',
    ),
    268174006 => array(
      'weight' => '0',
      'pid' => '0',
    ),
    2938987599 => array(
      'weight' => '0',
      'pid' => '3',
    ),
    1257454314 => array(
      'weight' => '0',
      'pid' => '0',
    ),
    1271379760 => array(
      'weight' => '0',
      'pid' => '0',
    ),
    933038561 => array(
      'weight' => '0',
      'pid' => '0',
    ),
    2 => array(
      'weight' => 11,
      'pid' => '0',
    ),
    1 => array(
      'weight' => 12,
      'pid' => '0',
    ),
  );
  $export['imce_roles_profiles'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'language_content_type_page';
  $strongarm->value = '2';
  $export['language_content_type_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_admin_title';
  $strongarm->value = 'Tertiary menu';
  $export['menu_block_1_admin_title'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_depth';
  $strongarm->value = '0';
  $export['menu_block_1_depth'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_expanded';
  $strongarm->value = 0;
  $export['menu_block_1_expanded'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_follow';
  $strongarm->value = 0;
  $export['menu_block_1_follow'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_level';
  $strongarm->value = '3';
  $export['menu_block_1_level'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_parent';
  $strongarm->value = 'primary-links:0';
  $export['menu_block_1_parent'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_sort';
  $strongarm->value = 0;
  $export['menu_block_1_sort'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_1_title_link';
  $strongarm->value = 0;
  $export['menu_block_1_title_link'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_block_ids';
  $strongarm->value = array(
    0 => 1,
  );
  $export['menu_block_ids'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_default_node_menu';
  $strongarm->value = 'primary-links';
  $export['menu_default_node_menu'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_expanded';
  $strongarm->value = array(
    0 => 'primary-links',
  );
  $export['menu_expanded'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_primary_links_source';
  $strongarm->value = 'primary-links';
  $export['menu_primary_links_source'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'menu_secondary_links_source';
  $strongarm->value = 'primary-links';
  $export['menu_secondary_links_source'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'node_admin_theme';
  $strongarm->value = 0;
  $export['node_admin_theme'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'node_options_page';
  $strongarm->value = array(
    0 => 'status',
  );
  $export['node_options_page'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'page_manager_user_view_disabled';
  $strongarm->value = FALSE;
  $export['page_manager_user_view_disabled'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'panels_legacy_rendering_mode';
  $strongarm->value = FALSE;
  $export['panels_legacy_rendering_mode'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_ignore_words';
  $strongarm->value = 'a,an,as,at,before,but,by,for,from,is,in,into,like,of,off,on,onto,per,since,than,the,this,that,to,up,via,with';
  $export['pathauto_ignore_words'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_indexaliases';
  $strongarm->value = FALSE;
  $export['pathauto_indexaliases'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_indexaliases_bulkupdate';
  $strongarm->value = FALSE;
  $export['pathauto_indexaliases_bulkupdate'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_max_component_length';
  $strongarm->value = '100';
  $export['pathauto_max_component_length'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_max_length';
  $strongarm->value = '100';
  $export['pathauto_max_length'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_modulelist';
  $strongarm->value = array(
    0 => 'node',
    1 => 'user',
    2 => 'taxonomy',
  );
  $export['pathauto_modulelist'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_node_applytofeeds';
  $strongarm->value = '';
  $export['pathauto_node_applytofeeds'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_node_bulkupdate';
  $strongarm->value = FALSE;
  $export['pathauto_node_bulkupdate'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_node_image_pattern';
  $strongarm->value = '';
  $export['pathauto_node_image_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_node_page_pattern';
  $strongarm->value = '';
  $export['pathauto_node_page_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_node_pattern';
  $strongarm->value = 'content/[title-raw]';
  $export['pathauto_node_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_node_story_pattern';
  $strongarm->value = '';
  $export['pathauto_node_story_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_punctuation_hyphen';
  $strongarm->value = 1;
  $export['pathauto_punctuation_hyphen'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_punctuation_quotes';
  $strongarm->value = 0;
  $export['pathauto_punctuation_quotes'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_separator';
  $strongarm->value = '-';
  $export['pathauto_separator'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_taxonomy_2_pattern';
  $strongarm->value = '';
  $export['pathauto_taxonomy_2_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_taxonomy_applytofeeds';
  $strongarm->value = FALSE;
  $export['pathauto_taxonomy_applytofeeds'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_taxonomy_bulkupdate';
  $strongarm->value = FALSE;
  $export['pathauto_taxonomy_bulkupdate'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_taxonomy_pattern';
  $strongarm->value = 'category/[vocab-raw]/[catpath-raw]';
  $export['pathauto_taxonomy_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_taxonomy_supportsfeeds';
  $strongarm->value = '0/feed';
  $export['pathauto_taxonomy_supportsfeeds'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_update_action';
  $strongarm->value = '2';
  $export['pathauto_update_action'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_user_bulkupdate';
  $strongarm->value = FALSE;
  $export['pathauto_user_bulkupdate'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_user_pattern';
  $strongarm->value = 'users/[user-raw]';
  $export['pathauto_user_pattern'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_user_supportsfeeds';
  $strongarm->value = '';
  $export['pathauto_user_supportsfeeds'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pathauto_verbose';
  $strongarm->value = FALSE;
  $export['pathauto_verbose'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'site_frontpage';
  $strongarm->value = 'home';
  $export['site_frontpage'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_allowed_tags_4';
  $strongarm->value = '<em> <i> <strong> <b> <u> <del> <ins> <sub> <sup> <cite> <strike> <s> <tt> <span> <font> <abbr> <acronym> <dfn> <q> <bdo> <big> <small>';
  $export['tableofcontents_allowed_tags_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_allowed_tags_6';
  $strongarm->value = '<em> <i> <strong> <b> <u> <del> <ins> <sub> <sup> <cite> <strike> <s> <tt> <span> <font> <abbr> <acronym> <dfn> <q> <bdo> <big> <small>';
  $export['tableofcontents_allowed_tags_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_allow_override_4';
  $strongarm->value = 1;
  $export['tableofcontents_allow_override_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_allow_override_6';
  $strongarm->value = 1;
  $export['tableofcontents_allow_override_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_attachments_4';
  $strongarm->value = 0;
  $export['tableofcontents_attachments_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_automatic_4';
  $strongarm->value = '0';
  $export['tableofcontents_automatic_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_automatic_6';
  $strongarm->value = '0';
  $export['tableofcontents_automatic_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_4';
  $strongarm->value = '';
  $export['tableofcontents_back_to_top_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_6';
  $strongarm->value = '';
  $export['tableofcontents_back_to_top_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_anchor_4';
  $strongarm->value = 'toc';
  $export['tableofcontents_back_to_top_anchor_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_anchor_6';
  $strongarm->value = 'toc';
  $export['tableofcontents_back_to_top_anchor_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_location_4';
  $strongarm->value = 'bottom';
  $export['tableofcontents_back_to_top_location_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_location_6';
  $strongarm->value = 'bottom';
  $export['tableofcontents_back_to_top_location_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_maxlevel_4';
  $strongarm->value = '4';
  $export['tableofcontents_back_to_top_maxlevel_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_maxlevel_6';
  $strongarm->value = '4';
  $export['tableofcontents_back_to_top_maxlevel_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_minlevel_4';
  $strongarm->value = '2';
  $export['tableofcontents_back_to_top_minlevel_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_back_to_top_minlevel_6';
  $strongarm->value = '2';
  $export['tableofcontents_back_to_top_minlevel_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_collapsed_4';
  $strongarm->value = 0;
  $export['tableofcontents_collapsed_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_collapsed_6';
  $strongarm->value = 0;
  $export['tableofcontents_collapsed_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_comments_4';
  $strongarm->value = 0;
  $export['tableofcontents_comments_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_comments_6';
  $strongarm->value = 0;
  $export['tableofcontents_comments_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_comments_level_4';
  $strongarm->value = '3';
  $export['tableofcontents_comments_level_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_comments_level_6';
  $strongarm->value = '3';
  $export['tableofcontents_comments_level_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_hide_show_4';
  $strongarm->value = 1;
  $export['tableofcontents_hide_show_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_hide_show_6';
  $strongarm->value = 1;
  $export['tableofcontents_hide_show_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_hide_table_4';
  $strongarm->value = 0;
  $export['tableofcontents_hide_table_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_hide_table_6';
  $strongarm->value = 0;
  $export['tableofcontents_hide_table_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_identifier_introducer_4';
  $strongarm->value = 'header';
  $export['tableofcontents_identifier_introducer_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_identifier_introducer_6';
  $strongarm->value = 'header';
  $export['tableofcontents_identifier_introducer_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_id_generator_4';
  $strongarm->value = 'title';
  $export['tableofcontents_id_generator_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_id_generator_6';
  $strongarm->value = 'title';
  $export['tableofcontents_id_generator_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_id_separator_4';
  $strongarm->value = '-';
  $export['tableofcontents_id_separator_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_id_separator_6';
  $strongarm->value = '-';
  $export['tableofcontents_id_separator_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_id_stripping_4';
  $strongarm->value = array(
    'digits' => 0,
    'dashes' => 0,
    'periods' => 0,
    'underscores' => 0,
    'colons' => 0,
  );
  $export['tableofcontents_id_stripping_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_id_stripping_6';
  $strongarm->value = array(
    'digits' => 0,
    'dashes' => 0,
    'periods' => 0,
    'underscores' => 0,
    'colons' => 0,
  );
  $export['tableofcontents_id_stripping_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_maxlevel_4';
  $strongarm->value = '3';
  $export['tableofcontents_maxlevel_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_maxlevel_6';
  $strongarm->value = '3';
  $export['tableofcontents_maxlevel_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_minlevel_4';
  $strongarm->value = '2';
  $export['tableofcontents_minlevel_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_minlevel_6';
  $strongarm->value = '2';
  $export['tableofcontents_minlevel_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_min_limit_4';
  $strongarm->value = '5';
  $export['tableofcontents_min_limit_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_min_limit_6';
  $strongarm->value = '5';
  $export['tableofcontents_min_limit_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_automatic_';
  $strongarm->value = '0';
  $export['tableofcontents_nodetype_toc_automatic_'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_automatic_forum';
  $strongarm->value = '0';
  $export['tableofcontents_nodetype_toc_automatic_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_automatic_profile';
  $strongarm->value = '0';
  $export['tableofcontents_nodetype_toc_automatic_profile'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_automatic_team';
  $strongarm->value = '0';
  $export['tableofcontents_nodetype_toc_automatic_team'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_automatic_team_forum';
  $strongarm->value = '0';
  $export['tableofcontents_nodetype_toc_automatic_team_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_remove_from_teaser_';
  $strongarm->value = 1;
  $export['tableofcontents_nodetype_toc_remove_from_teaser_'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_remove_from_teaser_forum';
  $strongarm->value = 1;
  $export['tableofcontents_nodetype_toc_remove_from_teaser_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_remove_from_teaser_profile';
  $strongarm->value = 1;
  $export['tableofcontents_nodetype_toc_remove_from_teaser_profile'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_remove_from_teaser_team';
  $strongarm->value = 1;
  $export['tableofcontents_nodetype_toc_remove_from_teaser_team'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_remove_from_teaser_team_forum';
  $strongarm->value = 1;
  $export['tableofcontents_nodetype_toc_remove_from_teaser_team_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_vtoc_';
  $strongarm->value = 0;
  $export['tableofcontents_nodetype_toc_vtoc_'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_vtoc_forum';
  $strongarm->value = 0;
  $export['tableofcontents_nodetype_toc_vtoc_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_vtoc_profile';
  $strongarm->value = 0;
  $export['tableofcontents_nodetype_toc_vtoc_profile'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_vtoc_team';
  $strongarm->value = 0;
  $export['tableofcontents_nodetype_toc_vtoc_team'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_nodetype_toc_vtoc_team_forum';
  $strongarm->value = 0;
  $export['tableofcontents_nodetype_toc_vtoc_team_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_numbering_4';
  $strongarm->value = '0';
  $export['tableofcontents_numbering_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_numbering_6';
  $strongarm->value = '0';
  $export['tableofcontents_numbering_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_end_letter_4';
  $strongarm->value = '.';
  $export['tableofcontents_number_end_letter_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_end_letter_6';
  $strongarm->value = '.';
  $export['tableofcontents_number_end_letter_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_headers_4';
  $strongarm->value = 0;
  $export['tableofcontents_number_headers_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_headers_6';
  $strongarm->value = 0;
  $export['tableofcontents_number_headers_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_mode_4';
  $strongarm->value = '0';
  $export['tableofcontents_number_mode_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_mode_6';
  $strongarm->value = '0';
  $export['tableofcontents_number_mode_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_separator_4';
  $strongarm->value = '.';
  $export['tableofcontents_number_separator_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_separator_6';
  $strongarm->value = '.';
  $export['tableofcontents_number_separator_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_start_letter_4';
  $strongarm->value = '';
  $export['tableofcontents_number_start_letter_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_number_start_letter_6';
  $strongarm->value = '';
  $export['tableofcontents_number_start_letter_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_from_teaser_admin_forum';
  $strongarm->value = 1;
  $export['tableofcontents_remove_from_teaser_admin_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_from_teaser_forum';
  $strongarm->value = 1;
  $export['tableofcontents_remove_from_teaser_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_from_teaser_profile';
  $strongarm->value = 1;
  $export['tableofcontents_remove_from_teaser_profile'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_from_teaser_team';
  $strongarm->value = 1;
  $export['tableofcontents_remove_from_teaser_team'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_from_teaser_team_forum';
  $strongarm->value = 1;
  $export['tableofcontents_remove_from_teaser_team_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_teaser_4';
  $strongarm->value = 1;
  $export['tableofcontents_remove_teaser_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_remove_teaser_6';
  $strongarm->value = 1;
  $export['tableofcontents_remove_teaser_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_safe_title_4';
  $strongarm->value = 1;
  $export['tableofcontents_safe_title_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_safe_title_6';
  $strongarm->value = 1;
  $export['tableofcontents_safe_title_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_scroll_back_to_top_4';
  $strongarm->value = 0;
  $export['tableofcontents_scroll_back_to_top_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_scroll_back_to_top_6';
  $strongarm->value = 0;
  $export['tableofcontents_scroll_back_to_top_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_title_4';
  $strongarm->value = 'Table of Contents';
  $export['tableofcontents_title_4'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_title_6';
  $strongarm->value = 'Table of Contents';
  $export['tableofcontents_title_6'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_toc_automatic_admin_forum';
  $strongarm->value = '0';
  $export['tableofcontents_toc_automatic_admin_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_toc_automatic_forum';
  $strongarm->value = '0';
  $export['tableofcontents_toc_automatic_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_toc_automatic_profile';
  $strongarm->value = '0';
  $export['tableofcontents_toc_automatic_profile'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_toc_automatic_team';
  $strongarm->value = '0';
  $export['tableofcontents_toc_automatic_team'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_toc_automatic_team_forum';
  $strongarm->value = '0';
  $export['tableofcontents_toc_automatic_team_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_vtoc_admin_forum';
  $strongarm->value = 0;
  $export['tableofcontents_vtoc_admin_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_vtoc_forum';
  $strongarm->value = 0;
  $export['tableofcontents_vtoc_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_vtoc_profile';
  $strongarm->value = 0;
  $export['tableofcontents_vtoc_profile'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_vtoc_team';
  $strongarm->value = 0;
  $export['tableofcontents_vtoc_team'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tableofcontents_vtoc_team_forum';
  $strongarm->value = 0;
  $export['tableofcontents_vtoc_team_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'tabtamer_tab_settings';
  $strongarm->value = array(
    'node/%/view' => array(
      'weight' => '-10',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/view',
      'tabtamer_title' => '',
    ),
    'node/%/edit' => array(
      'weight' => '1',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/edit',
      'tabtamer_title' => '',
    ),
    'node/%/revisions' => array(
      'weight' => '2',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/revisions',
      'tabtamer_title' => '',
    ),
    'node/%/panel_layout' => array(
      'weight' => '2',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/panel_layout',
      'tabtamer_title' => '',
    ),
    'node/%/panel_content' => array(
      'weight' => '3',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/panel_content',
      'tabtamer_title' => '',
    ),
    'node/%/revisions/list' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/revisions/list',
      'tabtamer_title' => '',
    ),
    'node/%/revisions/view/%/%' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/revisions/view/%/%',
      'tabtamer_title' => '',
    ),
    'node/%/revisions/view/latest' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'node/%node/revisions/view/latest',
      'tabtamer_title' => '',
    ),
    'search/node/%' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'search/node/%menu_tail',
      'tabtamer_title' => '',
    ),
    'search/user/%' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'search/user/%menu_tail',
      'tabtamer_title' => '',
    ),
    'user/login' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/login',
      'tabtamer_title' => '',
    ),
    'user/register' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/register',
      'tabtamer_title' => '',
    ),
    'user/password' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/password',
      'tabtamer_title' => '',
    ),
    'user/%/view' => array(
      'weight' => '-10',
      'action' => 'hidden',
      'tabtamer_original_path' => 'user/%user/view',
      'tabtamer_title' => '',
    ),
    'user/%/edit' => array(
      'weight' => '0',
      'action' => 'hidden',
      'tabtamer_original_path' => 'user/%user_category/edit',
      'tabtamer_title' => '',
    ),
    'user/%/imce' => array(
      'weight' => '10',
      'action' => 'hidden',
      'tabtamer_original_path' => 'user/%user/imce',
      'tabtamer_title' => '',
    ),
    'user/%/roles' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/%user/roles',
      'tabtamer_title' => '',
    ),
    'user/%/messages' => array(
      'weight' => '0',
      'action' => 'hidden',
      'tabtamer_original_path' => 'user/%/messages',
      'tabtamer_title' => '',
    ),
    'user/%/friends' => array(
      'weight' => '0',
      'action' => 'hidden',
      'tabtamer_original_path' => 'user/%/friends',
      'tabtamer_title' => '',
    ),
    'user/%/edit/account' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/%user_category/edit/account',
      'tabtamer_title' => '',
    ),
    'user/%/edit/profile' => array(
      'weight' => '1',
      'action' => 'hidden',
      'tabtamer_original_path' => 'user/%user_category/edit/profile',
      'tabtamer_title' => '',
    ),
    'user/%/friends/pending' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/%views_arg/friends/pending',
      'tabtamer_title' => '',
    ),
    'user/%/friends/flagged' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'user/%views_arg/friends/flagged',
      'tabtamer_title' => '',
    ),
    'account/profile/view' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/profile/view',
      'tabtamer_title' => '',
    ),
    'account/profile/edit' => array(
      'weight' => '5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/profile/edit',
      'tabtamer_title' => '',
    ),
    'account/prefs/computing' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/prefs/computing',
      'tabtamer_title' => '',
    ),
    'account/prefs/project' => array(
      'weight' => '5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/prefs/project',
      'tabtamer_title' => '',
    ),
    'account/prefs/community' => array(
      'weight' => '10',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/prefs/community',
      'tabtamer_title' => '',
    ),
    'account/prefs/privacy' => array(
      'weight' => '15',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/prefs/privacy',
      'tabtamer_title' => '',
    ),
    'account/obsolete' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/obsolete',
      'tabtamer_title' => '',
    ),
    'account/tasks/all' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/tasks/all',
      'tabtamer_title' => '',
    ),
    'account/tasks/error' => array(
      'weight' => '5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/tasks/error',
      'tabtamer_title' => '',
    ),
    'account/tasks/active' => array(
      'weight' => '1',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/tasks/active',
      'tabtamer_title' => '',
    ),
    'account/tasks/invalid' => array(
      'weight' => '4',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/tasks/invalid',
      'tabtamer_title' => '',
    ),
    'account/tasks/pending' => array(
      'weight' => '2',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/tasks/pending',
      'tabtamer_title' => '',
    ),
    'account/tasks/valid' => array(
      'weight' => '3',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/tasks/valid',
      'tabtamer_title' => '',
    ),
    'account/computers/all' => array(
      'weight' => '1',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/computers/all',
      'tabtamer_title' => '',
    ),
    'account/computers/active' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/computers/active',
      'tabtamer_title' => '',
    ),
    'account/%/computers' => array(
      'weight' => '0',
      'action' => 'hidden',
      'tabtamer_original_path' => 'account/%views_arg/computers',
      'tabtamer_title' => '',
    ),
    'account/%/computers/all' => array(
      'weight' => '1',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/%views_arg/computers/all',
      'tabtamer_title' => '',
    ),
    'account/%/friends/all' => array(
      'weight' => '-10',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/%/friends/all',
      'tabtamer_title' => '',
    ),
    'account/%/friends/pending' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/%views_arg/friends/pending',
      'tabtamer_title' => '',
    ),
    'account/%/friends/flagged' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'account/%views_arg/friends/flagged',
      'tabtamer_title' => '',
    ),
    'join/new' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'join/new',
      'tabtamer_title' => '',
    ),
    'join/boinc' => array(
      'weight' => '5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'join/boinc',
      'tabtamer_title' => '',
    ),
    'messages/list' => array(
      'weight' => '-10',
      'action' => 'enabled',
      'tabtamer_original_path' => 'messages/list',
      'tabtamer_title' => '',
    ),
    'messages/view/%' => array(
      'weight' => '-5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'messages/view/%privatemsg_thread',
      'tabtamer_title' => '',
    ),
    'messages/new' => array(
      'weight' => '-3',
      'action' => 'enabled',
      'tabtamer_original_path' => 'messages/new',
      'tabtamer_title' => '',
    ),
    'news' => array(
      'weight' => 0,
      'action' => 'enabled',
      'tabtamer_original_path' => 'news',
      'tabtamer_title' => '',
    ),
    'workunit/%/tasks' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'workunit/%views_arg/tasks',
      'tabtamer_title' => '',
    ),
    'workunit/%/tasks/error' => array(
      'weight' => '5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'workunit/%views_arg/tasks/error',
      'tabtamer_title' => '',
    ),
    'workunit/%/tasks/active' => array(
      'weight' => '1',
      'action' => 'enabled',
      'tabtamer_original_path' => 'workunit/%views_arg/tasks/active',
      'tabtamer_title' => '',
    ),
    'workunit/%/tasks/invalid' => array(
      'weight' => '4',
      'action' => 'enabled',
      'tabtamer_original_path' => 'workunit/%views_arg/tasks/invalid',
      'tabtamer_title' => '',
    ),
    'workunit/%/tasks/pending' => array(
      'weight' => '2',
      'action' => 'enabled',
      'tabtamer_original_path' => 'workunit/%views_arg/tasks/pending',
      'tabtamer_title' => '',
    ),
    'workunit/%/tasks/valid' => array(
      'weight' => '3',
      'action' => 'enabled',
      'tabtamer_original_path' => 'workunit/%views_arg/tasks/valid',
      'tabtamer_title' => '',
    ),
    'host/%/tasks' => array(
      'weight' => '0',
      'action' => 'enabled',
      'tabtamer_original_path' => 'host/%views_arg/tasks',
      'tabtamer_title' => '',
    ),
    'host/%/tasks/error' => array(
      'weight' => '5',
      'action' => 'enabled',
      'tabtamer_original_path' => 'host/%views_arg/tasks/error',
      'tabtamer_title' => '',
    ),
    'host/%/tasks/active' => array(
      'weight' => '1',
      'action' => 'enabled',
      'tabtamer_original_path' => 'host/%views_arg/tasks/active',
      'tabtamer_title' => '',
    ),
    'host/%/tasks/invalid' => array(
      'weight' => '4',
      'action' => 'enabled',
      'tabtamer_original_path' => 'host/%views_arg/tasks/invalid',
      'tabtamer_title' => '',
    ),
    'host/%/tasks/pending' => array(
      'weight' => '2',
      'action' => 'enabled',
      'tabtamer_original_path' => 'host/%views_arg/tasks/pending',
      'tabtamer_title' => '',
    ),
    'host/%/tasks/valid' => array(
      'weight' => '3',
      'action' => 'enabled',
      'tabtamer_original_path' => 'host/%views_arg/tasks/valid',
      'tabtamer_title' => '',
    ),
  );
  $export['tabtamer_tab_settings'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'theme_boinc_settings';
  $strongarm->value = array(
    'zen_block_editing' => 1,
    'zen_breadcrumb' => 'yes',
    'zen_breadcrumb_separator' => ' › ',
    'zen_breadcrumb_home' => 1,
    'zen_breadcrumb_trailing' => 1,
    'zen_breadcrumb_title' => 0,
    'zen_rebuild_registry' => 1,
    'zen_wireframes' => 0,
    'boinc_stats_chart_color' => '#FAA341',
    'boinc_stats_chart_bcolor' => '#FFFFFF',
    'toggle_logo' => 1,
    'toggle_name' => 1,
    'toggle_slogan' => 0,
    'toggle_mission' => 1,
    'toggle_node_user_picture' => 0,
    'toggle_comment_user_picture' => 1,
    'toggle_search' => 0,
    'toggle_favicon' => 1,
    'toggle_primary_links' => 1,
    'toggle_secondary_links' => 1,
    'default_logo' => 1,
    'logo_path' => '',
    'logo_upload' => '',
    'default_favicon' => 1,
    'favicon_path' => '',
    'favicon_upload' => '',
  );
  $export['theme_boinc_settings'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'theme_default';
  $strongarm->value = 'einstein';
  $export['theme_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'theme_einstein_settings';
  $strongarm->value = array(
    'zen_block_editing' => 1,
    'zen_breadcrumb' => 'yes',
    'zen_breadcrumb_separator' => ' › ',
    'zen_breadcrumb_home' => 1,
    'zen_breadcrumb_trailing' => 1,
    'zen_breadcrumb_title' => 0,
    'zen_rebuild_registry' => 0,
    'zen_wireframes' => 0,
    'toggle_logo' => 1,
    'toggle_name' => 0,
    'toggle_slogan' => 0,
    'toggle_mission' => 1,
    'toggle_node_user_picture' => 0,
    'toggle_comment_user_picture' => 1,
    'toggle_search' => 0,
    'toggle_favicon' => 1,
    'toggle_primary_links' => 1,
    'toggle_secondary_links' => 1,
    'default_logo' => 1,
    'logo_path' => '',
    'logo_upload' => '',
    'default_favicon' => 1,
    'favicon_path' => '',
    'favicon_upload' => '',
    'boinc_stats_chart_color' => '#009CDB',
    'boinc_stats_chart_bcolor' => '#EEEEEE',
  );
  $export['theme_einstein_settings'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'theme_settings';
  $strongarm->value = array(
    'toggle_node_info_page' => FALSE,
  );
  $export['theme_settings'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'theme_zen_settings';
  $strongarm->value = array(
    'zen_block_editing' => '1',
    'zen_breadcrumb' => 'yes',
    'zen_breadcrumb_separator' => ' › ',
    'zen_breadcrumb_home' => '1',
    'zen_breadcrumb_trailing' => '1',
    'zen_breadcrumb_title' => '0',
    'zen_layout' => 'zen-columns-liquid',
    'zen_rebuild_registry' => '0',
    'zen_wireframes' => '0',
  );
  $export['theme_zen_settings'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_extensions_2938987599';
  $strongarm->value = 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp';
  $export['upload_extensions_2938987599'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_extensions_3519698132';
  $strongarm->value = 'jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp';
  $export['upload_extensions_3519698132'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_extensions_default';
  $strongarm->value = 'jpg jpeg gif png txt pdf';
  $export['upload_extensions_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_forum';
  $strongarm->value = '0';
  $export['upload_forum'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_list_default';
  $strongarm->value = '0';
  $export['upload_list_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_max_resolution';
  $strongarm->value = '0';
  $export['upload_max_resolution'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_uploadsize_2938987599';
  $strongarm->value = '2';
  $export['upload_uploadsize_2938987599'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_uploadsize_3519698132';
  $strongarm->value = '2';
  $export['upload_uploadsize_3519698132'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_uploadsize_default';
  $strongarm->value = '2';
  $export['upload_uploadsize_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_usersize_2938987599';
  $strongarm->value = '1000';
  $export['upload_usersize_2938987599'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_usersize_3519698132';
  $strongarm->value = '1000';
  $export['upload_usersize_3519698132'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'upload_usersize_default';
  $strongarm->value = '10';
  $export['upload_usersize_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_email_verification';
  $strongarm->value = 1;
  $export['user_email_verification'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_password_reset_body';
  $strongarm->value = 'Hello,

A request to reset the password for your account has been made at !site.

You may now log in to !uri_brief by clicking on this link or copying and pasting it in your browser:

!login_url

This is a one-time login, so it can be used only once. It expires after one day and nothing will happen if it\'s not used.

After logging in, you will be redirected to !edit_uri so you can change your password.

--  !site team';
  $export['user_mail_password_reset_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_password_reset_subject';
  $strongarm->value = 'Replacement login information for !mailto at !site';
  $export['user_mail_password_reset_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_register_admin_created_body';
  $strongarm->value = 'Hello,

A site administrator at !site has created an account for you. You may now log in to !login_uri using your email and your passphrase.

You may also log in by clicking on this link or copying and pasting it in your browser:

!login_url

This is a one-time login, so it can be used only once.

After logging in, you will be redirected to !edit_uri so you can change your password.

--  !site team';
  $export['user_mail_register_admin_created_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_register_admin_created_subject';
  $strongarm->value = 'An administrator created an account for you at !site';
  $export['user_mail_register_admin_created_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_register_no_approval_required_body';
  $strongarm->value = 'Hello,

Thank you for registering at !site. You may now log in to !uri_brief by clicking on this link or copying and pasting it in your browser:

!login_url

This is a one-time login, so it can be used only once. It expires after one day and nothing will happen if it\'s not used.

After logging in, you will be redirected to !edit_uri, where you can choose your password.

--  !site team';
  $export['user_mail_register_no_approval_required_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_register_no_approval_required_subject';
  $strongarm->value = 'Account details for !mailto at !site';
  $export['user_mail_register_no_approval_required_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_register_pending_approval_body';
  $strongarm->value = 'Hello,

Thank you for registering at !site. Your application for an account is currently pending approval. Once it has been approved, you will receive another e-mail containing information about how to log in, set your password, and other details.

--  !site team';
  $export['user_mail_register_pending_approval_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_register_pending_approval_subject';
  $strongarm->value = 'Account details for !mailto at !site (pending admin approval)';
  $export['user_mail_register_pending_approval_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_activated_body';
  $strongarm->value = 'Hello,

Your account at !site has been activated.

You may now log in to !uri_brief by clicking on this link or copying and pasting it in your browser:

!login_url

This is a one-time login, so it can be used only once. It expires after one day and nothing will happen if it\'s not used.

After logging in, you will be redirected to !edit_uri so you can change your password.

--  !site team
';
  $export['user_mail_status_activated_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_activated_notify';
  $strongarm->value = 1;
  $export['user_mail_status_activated_notify'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_activated_subject';
  $strongarm->value = 'Account details for !mailto at !site (approved)';
  $export['user_mail_status_activated_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_blocked_body';
  $strongarm->value = 'Hello,

Your account on !site has been blocked.

--  !site team';
  $export['user_mail_status_blocked_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_blocked_notify';
  $strongarm->value = 0;
  $export['user_mail_status_blocked_notify'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_blocked_subject';
  $strongarm->value = 'Account details for !mailto at !site (blocked)';
  $export['user_mail_status_blocked_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_deleted_body';
  $strongarm->value = 'Hello,

Your account on !site has been deleted.

--  !site team';
  $export['user_mail_status_deleted_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_deleted_notify';
  $strongarm->value = 0;
  $export['user_mail_status_deleted_notify'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_mail_status_deleted_subject';
  $strongarm->value = 'Account details for !mailto at !site (deleted)';
  $export['user_mail_status_deleted_subject'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_pictures';
  $strongarm->value = '1';
  $export['user_pictures'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_picture_default';
  $strongarm->value = '';
  $export['user_picture_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_picture_dimensions';
  $strongarm->value = '85x85';
  $export['user_picture_dimensions'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_picture_file_size';
  $strongarm->value = '30';
  $export['user_picture_file_size'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_picture_guidelines';
  $strongarm->value = '';
  $export['user_picture_guidelines'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_picture_path';
  $strongarm->value = 'pictures';
  $export['user_picture_path'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_register';
  $strongarm->value = '1';
  $export['user_register'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_registration_help';
  $strongarm->value = '';
  $export['user_registration_help'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'user_signatures';
  $strongarm->value = '1';
  $export['user_signatures'] = $strongarm;

  return $export;
}
