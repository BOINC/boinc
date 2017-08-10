<?php

/**
 * Implementation of hook_views_default_views().
 */
function friends_views_default_views() {
  $views = array();

  // Exported view: boinc_friends
  $view = new view;
  $view->name = 'boinc_friends';
  $view->description = 'Customized views for friend relationships';
  $view->tag = 'boinc';
  $view->base_table = 'users';
  $view->core = 6;
  $view->api_version = '2';
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'flag_friend_content_rel' => array(
      'label' => 'flag friend',
      'required' => 1,
      'flag' => 'friend',
      'user_scope' => 'any',
      'id' => 'flag_friend_content_rel',
      'table' => 'users',
      'field' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
    'ops' => array(
      'label' => 'Flag link',
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Override',
      ),
    ),
    'uid' => array(
      'label' => 'Flag friend links',
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'friend_uid' => array(
      'id' => 'friend_uid',
      'table' => 'flag_friend',
      'field' => 'friend_uid',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'operator' => '=',
      'value' => '1',
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status',
      'table' => 'users',
      'field' => 'status',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('empty', bts('No Friends have been added.', array(), NULL, 'boinc:friends-page'));
  $handler->override_option('empty_format', '5');
  $handler->override_option('use_pager', '1');
  $handler->override_option('distinct', 1);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 0,
    'order' => 'asc',
    'summary' => '',
    'columns' => array(
      'name' => 'name',
      'ops' => 'ops',
      'uid' => 'uid',
    ),
    'info' => array(
      'name' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'ops' => array(
        'separator' => '',
      ),
      'uid' => array(
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler = $view->new_display('page', 'Page (friends)', 'page_1');
  $handler->override_option('relationships', array());
  $handler->override_option('fields', array(
    'picture' => array(
      'label' => 'Friend',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'exclude' => 1,
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => '',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 1,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'friend_uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'breadcrumb' => '',
      'default_argument_type' => 'current_user',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'flag_friend_include_arg_user' => 0,
      'id' => 'friend_uid',
      'table' => 'flag_friend',
      'field' => 'friend_uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(
        2 => 0,
        3 => 0,
      ),
      'validate_argument_type' => 'tid',
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => '',
      'validate_user_argument_type' => 'uid',
      'validate_user_roles' => array(
        2 => 0,
        3 => 0,
        5 => 0,
        4 => 0,
        6 => 0,
      ),
      'me_redirect' => 0,
      'me_validate_user_argument_type' => 'uid',
      'me_validate_user_roles' => array(
        2 => 0,
        3 => 0,
        5 => 0,
        4 => 0,
        6 => 0,
      ),
      'default_taxonomy_tid_term_page' => 0,
      'default_taxonomy_tid_node' => 0,
      'default_taxonomy_tid_limit' => 0,
      'default_taxonomy_tid_vids' => array(
        2 => 0,
        3 => 0,
      ),
      'validate_argument_node_type' => array(
        'page' => 0,
        'profile' => 0,
        'forum' => 0,
        'panel' => 0,
        'news' => 0,
        'story' => 0,
        'team' => 0,
      ),
      'validate_argument_transform' => 0,
      'validate_user_restrict_roles' => 0,
      'me_validate_user_restrict_roles' => 0,
    ),
  ));
  $handler->override_option('title', 'Friends');
  $handler->override_option('path', 'account/%/friends/all');
  $handler->override_option('menu', array(
    'type' => 'default tab',
    'title' => bts('View All Friends', array(), NULL, 'boinc:friends-page'),
    'description' => '',
    'weight' => '-10',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'normal',
    'title' => 'Friends',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler = $view->new_display('page', 'Page (pending)', 'page_2');
  $handler->override_option('fields', array(
    'picture' => array(
      'label' => 'User',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'exclude' => 1,
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'message' => array(
      'label' => 'Message',
      'exclude' => 0,
      'id' => 'message',
      'table' => 'flag_friend_message',
      'field' => 'message',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
    'uid' => array(
      'label' => 'Flag friend links',
      'alter' => array(
        'alter_text' => 1,
        'text' => '<ul class="tab-list">
  <li class="alt tab">[uid]</li>',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'flag_friend',
      'field' => 'uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'ops' => array(
      'label' => '',
      'alter' => array(
        'alter_text' => 1,
        'text' => '  <li class="first alt tab">[ops]</li>
</ul>',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('arguments', array(
    'content_id' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'user',
      'default_argument' => '',
      'validate_type' => 'php',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'content_id',
      'table' => 'flag_content',
      'field' => 'content_id',
      'relationship' => 'flag_friend_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => 'if ($argument !== $GLOBALS[\'user\']->uid && !user_access(\'administer users\')) {
                                      return FALSE;
                                    }
                                    return TRUE;',
    ),
  ));
  $handler->override_option('title', 'Friend Requests');
  $handler->override_option('header', 'These are users who would like to be your friend.');
  $handler->override_option('header_format', '5');
  $handler->override_option('header_empty', 0);
  $handler->override_option('empty', 'No Friend Requests.');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 0,
    'order' => 'asc',
    'summary' => '',
    'columns' => array(
      'picture' => 'picture',
      'name' => 'name',
      'message' => 'message',
      'ops' => 'ops',
      'uid' => 'ops',
    ),
    'info' => array(
      'picture' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'message' => array(
        'separator' => '',
      ),
      'ops' => array(
        'separator' => '',
      ),
      'uid' => array(
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler->override_option('path', 'account/%/friends/pending');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => bts('Friend Requests', array(), NULL, 'boinc:friends-page'),
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler = $view->new_display('page', 'Page (flagged)', 'page_3');
  $handler->override_option('relationships', array(
    'flag_content_rel' => array(
      'label' => 'flag',
      'required' => 1,
      'flag' => 'friend',
      'user_scope' => 'any',
      'id' => 'flag_content_rel',
      'table' => 'users',
      'field' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'uid' => array(
      'label' => 'Flag user',
      'required' => 0,
      'id' => 'uid',
      'table' => 'flag_content',
      'field' => 'uid',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('fields', array(
    'picture' => array(
      'label' => 'User',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'exclude' => 1,
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'name' => array(
      'label' => 'Name',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'message' => array(
      'label' => 'Message',
      'exclude' => 0,
      'id' => 'message',
      'table' => 'flag_friend_message',
      'field' => 'message',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
    'ops' => array(
      'label' => '',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'link_type' => '',
      'exclude' => 0,
      'id' => 'ops',
      'table' => 'flag_content',
      'field' => 'ops',
      'relationship' => 'flag_content_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('arguments', array(
    'uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'breadcrumb' => '',
      'default_argument_type' => 'user',
      'default_argument' => '',
      'validate_type' => 'php',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'uid',
      'table' => 'users',
      'field' => 'uid',
      'validate_user_argument_type' => 'uid',
      'validate_user_roles' => array(
        2 => 0,
      ),
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'uid',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_argument_transform' => 0,
      'validate_user_restrict_roles' => 0,
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => 'if ($argument !== $GLOBALS[\'user\']->uid && !user_access(\'administer users\')) {
    return FALSE;
  }
  return TRUE;',
    ),
  ));
  $handler->override_option('title', 'Awaiting Friend Approvals');
  $handler->override_option('header', 'These are users who you have requested to be friends with.');
  $handler->override_option('header_format', '5');
  $handler->override_option('header_empty', 1);
  $handler->override_option('empty', 'No Friend Requests.');
  $handler->override_option('path', 'account/%/friends/flagged');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => bts('Awaiting Friend Approvals', array(), NULL, 'boinc:friends-page'),
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler = $view->new_display('block', 'Current user\'s Friends block', 'block_1');
  $handler->override_option('relationships', array(
    'content_profile_rel' => array(
      'label' => 'Content Profile',
      'required' => 0,
      'type' => 'profile',
      'id' => 'content_profile_rel',
      'table' => 'users',
      'field' => 'content_profile_rel',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Name',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'link_to_user' => 0,
      'overwrite_anonymous' => 0,
      'anonymous_text' => '',
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
    'field_image_fid' => array(
      'label' => '',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'link_to_node' => 0,
      'label_type' => 'none',
      'format' => 'image_plain',
      'multiple' => array(
        'group' => TRUE,
        'multiple_number' => '',
        'multiple_from' => '',
        'multiple_reversed' => FALSE,
      ),
      'exclude' => 0,
      'id' => 'field_image_fid',
      'table' => 'node_data_field_image',
      'field' => 'field_image_fid',
      'relationship' => 'content_profile_rel',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
    'uid' => array(
      'label' => 'Uid',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'absolute' => 0,
        'link_class' => '',
        'alt' => '',
        'rel' => '',
        'prefix' => '',
        'suffix' => '',
        'target' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'html' => 0,
        'strip_tags' => 0,
      ),
      'empty' => '',
      'hide_empty' => 0,
      'empty_zero' => 0,
      'hide_alter_empty' => 1,
      'link_to_user' => 0,
      'exclude' => 0,
      'id' => 'uid',
      'table' => 'users',
      'field' => 'uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'friend_uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'current_user',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'friend_uid',
      'table' => 'flag_friend',
      'field' => 'friend_uid',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_php' => '',
    ),
  ));
  $handler->override_option('header_format', '3');
  $handler->override_option('header_empty', 0);
  $handler->override_option('items_per_page', 6);
  $handler->override_option('use_pager', '0');
  $handler->override_option('use_more', 1);
  $handler->override_option('use_more_always', 1);
  $handler->override_option('style_plugin', 'default');
  $handler->override_option('style_options', array());
  $handler->override_option('block_description', '');
  $handler->override_option('block_caching', -1);

  $views[$view->name] = $view;

  return $views;
}
