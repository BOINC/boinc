<?php

/**
 * Implementation of hook_ctools_plugin_api().
 */
function boinc_standard_ctools_plugin_api() {
  list($module, $api) = func_get_args();
  if ($module == "input_formats" && $api == "input_formats") {
    return array("version" => 1);
  }
  elseif ($module == "page_manager" && $api == "pages_default") {
    return array("version" => 1);
  }
  elseif ($module == "panels_mini" && $api == "panels_default") {
    return array("version" => 1);
  }
  elseif ($module == "strongarm" && $api == "strongarm") {
    return array("version" => 1);
  }
  elseif ($module == "wysiwyg" && $api == "wysiwyg") {
    return array("version" => 2);
  }
}

/**
 * Implementation of hook_flag_default_flags().
 */
function boinc_standard_flag_default_flags() {
  $flags = array();
  // Exported flag: "Comment Abuse".
  $flags['abuse_comment'] = array(
    'content_type' => 'comment',
    'title' => 'Comment Abuse',
    'global' => '0',
    'types' => array(
      '0' => 'news',
      '1' => 'forum',
    ),
    'flag_short' => 'Report',
    'flag_long' => 'Report this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Cancel report',
    'unflag_long' => 'Cancel reporting this content as offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'roles' => array(
      'flag' => array(
        '0' => 2,
      ),
      'unflag' => array(
        '0' => 2,
      ),
    ),
    'weight' => 0,
    'access_author' => '',
    'show_on_comment' => 1,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Node Abuse".
  $flags['abuse_node'] = array(
    'content_type' => 'node',
    'title' => 'Node Abuse',
    'global' => '0',
    'types' => array(
      '0' => 'profile',
      '1' => 'forum',
    ),
    'flag_short' => 'Report',
    'flag_long' => 'Report this content as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Cancel report',
    'unflag_long' => 'Cancel reporting this content as offensive.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'roles' => array(
      'flag' => array(
        '0' => 2,
      ),
      'unflag' => array(
        '0' => 2,
      ),
    ),
    'weight' => 0,
    'show_on_page' => 1,
    'show_on_teaser' => 0,
    'show_on_form' => 0,
    'access_author' => '',
    'i18n' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "User Abuse".
  $flags['abuse_user'] = array(
    'content_type' => 'user',
    'title' => 'User Abuse',
    'global' => '0',
    'types' => array(),
    'flag_short' => 'Report User',
    'flag_long' => 'Report this user as offensive.',
    'flag_message' => '',
    'unflag_short' => 'Remove Report',
    'unflag_long' => 'Remove your report about this user.',
    'unflag_message' => '',
    'unflag_denied_text' => '',
    'link_type' => 'confirm',
    'roles' => array(
      'flag' => array(
        '0' => '2',
      ),
      'unflag' => array(
        '0' => '2',
      ),
    ),
    'weight' => 0,
    'show_on_profile' => TRUE,
    'access_uid' => '',
    'flag_confirmation' => 'Are you sure you want to report this user as offensive?',
    'unflag_confirmation' => 'Are you sure you want to remove your report of this user?',
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  // Exported flag: "Subscriptions".
  $flags['subscriptions'] = array(
    'content_type' => 'node',
    'title' => 'Subscriptions',
    'global' => '0',
    'types' => array(
      '0' => 'news',
      '1' => 'forum',
    ),
    'flag_short' => 'subscribe',
    'flag_long' => 'Subscribe to this topic',
    'flag_message' => 'You are now subscribed to this topic.',
    'unflag_short' => 'unsubscribe',
    'unflag_long' => 'Unsubscribe to this topic',
    'unflag_message' => 'You are no longer subscribed to this topic.',
    'unflag_denied_text' => '',
    'link_type' => 'toggle',
    'roles' => array(
      'flag' => array(
        '0' => '2',
      ),
      'unflag' => array(
        '0' => '2',
      ),
    ),
    'weight' => 0,
    'show_on_page' => 1,
    'show_on_teaser' => 1,
    'show_on_form' => 0,
    'access_author' => '',
    'i18n' => 0,
    'module' => 'boinc_standard',
    'locked' => array(
      '0' => 'name',
    ),
    'api_version' => 2,
  );
  return $flags;

}

/**
 * Implementation of hook_node_info().
 */
function boinc_standard_node_info() {
  $items = array(
    'page' => array(
      'name' => t('Page'),
      'module' => 'features',
      'description' => t('A <em>page</em>, similar in form to a <em>story</em>, is a simple method for creating and displaying information that rarely changes, such as an "About us" section of a website. By default, a <em>page</em> entry does not allow visitor comments and is not featured on the site\'s initial home page.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'has_body' => '1',
      'body_label' => t('Body'),
      'min_word_count' => '0',
      'help' => '',
    ),
  );
  return $items;
}
