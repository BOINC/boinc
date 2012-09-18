<?php

/**
 * Implementation of hook_views_default_views().
 */
function og_views_views_default_views() {
  /*
   * View 'og'
   */
  $view = new view;
  $view->name = 'og';
  $view->description = 'Groups directory';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Group',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'description' => array(
      'id' => 'description',
      'table' => 'og',
      'field' => 'description',
      'label' => 'Description',
      'relationship' => 'none',
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Manager',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'post_count' => array(
      'label' => 'Posts',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'exclude' => 0,
      'id' => 'post_count',
      'table' => 'og',
      'field' => 'post_count',
      'relationship' => 'none',
    ),
    'member_count' => array(
      'label' => 'Members',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'exclude' => 0,
      'id' => 'member_count',
      'table' => 'og',
      'field' => 'member_count',
      'relationship' => 'none',
    ),
    'created' => array(
      'label' => 'Created',
      'date_format' => 'custom',
      'custom_date_format' => 'M y',
      'exclude' => 1,
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'relationship' => 'none',
    ),
    'subscribe' => array(
      'id' => 'subscribe',
      'table' => 'og',
      'field' => 'subscribe',
      'label' => 'Join Link',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'operator' => 'contains',
      'value' => '',
      'group' => 0,
      'exposed' => TRUE,
      'expose' => array(
        'operator' => 'title_oper',
        'identifier' => 'title',
        'label' => 'Search for a group by name',
        'optional' => 1,
        'remember' => 0,
      ),
      'case' => 0,
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Hide',
      ),
    ),
    'type_groups_all' => array(
      'id' => 'type_groups_all',
      'table' => 'og',
      'field' => 'type_groups_all',
      'operator' => '=',
      'value' => '',
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'directory' => array(
      'id' => 'directory',
      'table' => 'og',
      'field' => 'directory',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Groups');
  $handler->override_option('empty', 'No groups');
  $handler->override_option('empty_format', '1');
  $handler->override_option('use_ajax', '1');
  $handler->override_option('items_per_page', 90);
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 1,
    'order' => 'desc',
    'columns' => array(
      'title' => 'title',
      'description' => 'description',
      'name' => 'name',
      'post_count' => 'post_count',
      'member_count' => 'member_count',
      'created' => 'created',
      'subscribe' => 'subscribe',
    ),
    'info' => array(
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'description' => array(
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'post_count' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'member_count' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'created' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'subscribe' => array(
        'separator' => '',
      ),
    ),
    'default' => 'created',
  ));
  $handler = $view->new_display('feed', 'Feed', 'feed');
  $handler->override_option('sorts', array(
    'created' => array(
      'order' => 'DESC',
      'granularity' => 'second',
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('items_per_page', 20);
  $handler->override_option('style_plugin', 'rss');
  $handler->override_option('style_options', array(
    'mission_description' => FALSE,
    'description' => '',
  ));
  $handler->override_option('row_plugin', 'node_rss');
  $handler->override_option('row_options', array(
    'item_length' => 'default',
  ));
  $handler->override_option('path', 'og/all/feed');
  $handler->override_option('menu', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('displays', array(
    'page' => 'page',
    'default' => 0,
  ));
  $handler->override_option('sitename_title', FALSE);
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'og/all');
  $handler->override_option('menu', array(
    'type' => 'default tab',
    'title' => 'Groups',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'normal',
    'title' => 'Groups',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $views[$view->name] = $view;

  /*
   * View 'og_ghp_ron'
   */
  $view = new view;
  $view->name = 'og_ghp_ron';
  $view->description = 'OG: Group home page - River of news. Default';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('sorts', array(
    'sticky' => array(
      'id' => 'sticky',
      'table' => 'node',
      'field' => 'sticky',
      'order' => 'DESC',
      'relationship' => 'none',
    ),
    'created' => array(
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'order' => 'DESC',
      'granularity' => 'second',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'group_nid' => array(
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'default_action' => 'not found',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '%1',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'blog' => 0,
        'poll' => 0,
        'book' => 0,
        'group1' => 0,
        'group2' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
      'default_argument_user' => 0,
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(
        '1' => 0,
        '2' => 0,
        '3' => 0,
        '4' => 0,
      ),
      'validate_argument_type' => 'tid',
      'not' => 0,
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('items_per_page', 20);
  $handler->override_option('use_pager', '1');
  $handler->override_option('distinct', 0);
  $handler->override_option('row_plugin', 'node');
  $handler->override_option('row_options', array(
    'teaser' => 1,
    'links' => 1,
  ));
  $handler = $view->new_display('feed', 'Feed', 'feed');
  $handler->override_option('style_plugin', 'rss');
  $handler->override_option('style_options', array(
    'mission_description' => 0,
    'description' => 'rss desc',
  ));
  $handler->override_option('row_plugin', 'node_rss');
  $handler->override_option('row_options', array(
    'item_length' => 'default',
  ));
  $handler->override_option('path', 'not_used');
  $handler->override_option('menu', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('displays', array(
    'default' => 0,
  ));
  $handler->override_option('sitename_title', FALSE);
  $views[$view->name] = $view;

  /*
   * View 'og_members'
   */
  $view = new view;
  $view->name = 'og_members';
  $view->description = 'Member listing';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'users';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Name',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'managelink' => array(
      'id' => 'managelink',
      'table' => 'og_uid',
      'field' => 'managelink',
      'label' => 'Manage',
      'relationship' => 'none',
    ),
    'managelinkadmin' => array(
      'id' => 'managelinkadmin',
      'table' => 'og_uid',
      'field' => 'managelinkadmin',
      'label' => 'Admin',
      'relationship' => 'none',
    ),
    'is_manager' => array(
      'label' => '',
      'alter' => array(
        'alter_text' => 0,
        'text' => '',
        'make_link' => 0,
        'path' => '',
        'alt' => '',
        'prefix' => '',
        'suffix' => '',
        'help' => '',
        'trim' => 0,
        'max_length' => '',
        'word_boundary' => 1,
        'ellipsis' => 1,
        'strip_tags' => 0,
        'html' => 0,
      ),
      'exclude' => 0,
      'id' => 'is_manager',
      'table' => 'og_uid',
      'field' => 'is_manager',
      'relationship' => 'none',
    ),
    'is_admin' => array(
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'label' => '',
      'relationship' => 'none',
    ),
    'is_active' => array(
      'id' => 'is_active',
      'table' => 'og_uid',
      'field' => 'is_active',
      'label' => '',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'is_admin' => array(
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'order' => 'DESC',
      'relationship' => 'none',
    ),
    'is_active' => array(
      'id' => 'is_active',
      'table' => 'og_uid',
      'field' => 'is_active',
      'order' => 'ASC',
      'relationship' => 'none',
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'order' => 'ASC',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'nid' => array(
      'id' => 'nid',
      'table' => 'og_uid',
      'field' => 'nid',
      'default_action' => 'not found',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => 'Members of %1',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'blog' => 0,
        'poll' => 0,
        'book' => 0,
        'group1' => 0,
        'group2' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
      'default_argument_user' => 0,
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(
        '1' => 0,
        '2' => 0,
        '3' => 0,
        '4' => 0,
      ),
      'validate_argument_type' => 'tid',
      'not' => 0,
      'validate_argument_is_member' => 1,
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'users',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'none',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', 90);
  $handler->override_option('use_pager', '1');
  $handler->override_option('distinct', 0);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'override' => 0,
    'order' => 'asc',
    'columns' => array(
      'name' => 'name',
      'managelink' => 'managelink',
      'managelinkadmin' => 'managelinkadmin',
      'is_manager' => 'name',
      'is_admin' => 'name',
      'is_active' => 'name',
    ),
    'info' => array(
      'name' => array(
        'sortable' => 0,
        'separator' => '&nbsp;',
      ),
      'managelink' => array(
        'separator' => '',
      ),
      'managelinkadmin' => array(
        'separator' => '',
      ),
      'is_manager' => array(
        'separator' => '',
      ),
      'is_admin' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'is_active' => array(
        'sortable' => 0,
        'separator' => '',
      ),
    ),
    'default' => '-1',
  ));
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'og/users/%/list');
  $handler->override_option('menu', array(
    'type' => 'default tab',
    'title' => 'List',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'normal',
    'title' => 'List',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $views[$view->name] = $view;

  /*
   * View 'og_members_block'
   */
  $view = new view;
  $view->name = 'og_members_block';
  $view->description = 'Member listing block';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'users';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'nid' => array(
      'id' => 'nid',
      'table' => 'og_uid',
      'field' => 'nid',
      'label' => 'Group',
      'required' => 0,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'picture' => array(
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'label' => '',
      'relationship' => 'none',
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => '',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'is_admin' => array(
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'label' => '',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'created' => array(
      'id' => 'created',
      'table' => 'og_uid',
      'field' => 'created',
      'order' => 'DESC',
      'granularity' => 'second',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'nid' => array(
      'id' => 'nid',
      'table' => 'og_uid',
      'field' => 'nid',
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'php',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_fixed' => '39',
      'default_argument_php' => 'if ($node = og_get_group_context()) {
                return $node->nid;
              }
              else {
                return NULL;
              }',
      'validate_argument_node_type' => array(
        'blog' => 0,
        'poll' => 0,
        'book' => 0,
        'group1' => 0,
        'group2' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
      'validate_argument_vocabulary' => array(
        '1' => 0,
        '2' => 0,
        '3' => 0,
        '4' => 0,
      ),
      'validate_argument_type' => 'tid',
      'default_argument_user' => 0,
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'not' => 0,
      'validate_argument_is_member' => 1,
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'users',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'is_active' => array(
      'id' => 'is_active',
      'table' => 'og_uid',
      'field' => 'is_active',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'is_admin' => array(
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'operator' => 'in',
      'value' => array(
        '0' => '0',
        '1' => '1',
      ),
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'none',
    'role' => array(),
    'perm' => '',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Recent members');
  $handler->override_option('empty_format', '1');
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'list');
  $handler->override_option('style_options', array(
    'type' => 'ul',
  ));
  $handler->override_option('row_options', array(
    'inline' => array(
      'name' => 'name',
      'is_admin' => 'is_admin',
    ),
    'separator' => '&nbsp;',
  ));
  $handler = $view->new_display('block', 'Block Admins', 'block');
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'users',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'is_active' => array(
      'id' => 'is_active',
      'table' => 'og_uid',
      'field' => 'is_active',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'is_admin' => array(
      'operator' => 'in',
      'value' => array(
        '1' => '1',
      ),
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('title', 'Group admins');
  $handler->override_option('block_description', 'Group admins');
  $handler->override_option('block_caching', -1);
  $handler = $view->new_display('block', 'Block Admins, Members', 'block_1');
  $handler->override_option('block_description', 'Group members');
  $handler->override_option('block_caching', -1);
  $views[$view->name] = $view;

  /*
   * View 'og_members_faces'
   */
  $view = new view;
  $view->name = 'og_members_faces';
  $view->description = 'Member listing in grid with user pictures';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'users';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'picture' => array(
      'id' => 'picture',
      'table' => 'users',
      'field' => 'picture',
      'label' => '',
      'relationship' => 'none',
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => '',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'is_admin' => array(
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'label' => '',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'is_admin' => array(
      'id' => 'is_admin',
      'table' => 'og_uid',
      'field' => 'is_admin',
      'order' => 'DESC',
      'relationship' => 'none',
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'order' => 'ASC',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'nid' => array(
      'id' => 'nid',
      'table' => 'og_uid',
      'field' => 'nid',
      'default_action' => 'not found',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => 'Members of %1',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'blog' => 0,
        'poll' => 0,
        'book' => 0,
        'group1' => 0,
        'group2' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(
        '1' => 0,
        '2' => 0,
        '3' => 0,
        '4' => 0,
      ),
      'validate_argument_type' => 'tid',
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
      'validate_argument_is_member' => 1,
      'not' => 0,
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'users',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'is_active' => array(
      'id' => 'is_active',
      'table' => 'og_uid',
      'field' => 'is_active',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'none',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Group faces');
  $handler->override_option('empty_format', '1');
  $handler->override_option('use_ajax', '1');
  $handler->override_option('items_per_page', 25);
  $handler->override_option('use_pager', '1');
  $handler->override_option('distinct', 0);
  $handler->override_option('style_plugin', 'grid');
  $handler->override_option('style_options', array(
    'columns' => '5',
    'alignment' => 'horizontal',
  ));
  $handler->override_option('row_options', array(
    'inline' => array(
      'name' => 'name',
      'is_manager' => 'is_manager',
      'is_admin' => 'is_admin',
    ),
    'separator' => '',
  ));
  $views[$view->name] = $view;

  /*
   * View 'og_my'
   */
  $view = new view;
  $view->name = 'og_my';
  $view->description = 'List user\'s subscribed groups and provide link to manage subscription';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Group',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Manager',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'managelinkmy' => array(
      'label' => 'Manage',
      'exclude' => 0,
      'id' => 'managelinkmy',
      'table' => 'og_uid',
      'field' => 'managelinkmy',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'type_groups_all' => array(
      'id' => 'type_groups_all',
      'table' => 'og',
      'field' => 'type_groups_all',
      'operator' => '=',
      'value' => '',
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'uid' => array(
      'operator' => '=',
      'value' => '1',
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'uid',
      'table' => 'og_uid',
      'field' => 'uid',
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
      'override' => array(
        'button' => 'Override',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'role',
    'role' => array(
      '2' => 2,
      '1' => 0,
    ),
    'perm' => 'view advanced help topic',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'My groups');
  $handler->override_option('empty', 'Not a member of any groups.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', 50);
  $handler->override_option('use_pager', '1');
  $handler->override_option('use_more', 1);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'override' => 1,
    'order' => 'asc',
    'columns' => array(
      'title' => 'title',
      'name' => 'name',
      'managelink' => 'managelink',
    ),
    'info' => array(
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'managelink' => array(
        'separator' => '',
      ),
    ),
    'default' => 'title',
  ));
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'og/my');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'My groups',
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
  $handler = $view->new_display('block', 'Block', 'block_1');
  $handler->override_option('fields', array(
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Group',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'post_count_new' => array(
      'label' => 'New',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'exclude' => 0,
      'id' => 'post_count_new',
      'table' => 'og',
      'field' => 'post_count_new',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'title' => array(
      'order' => 'ASC',
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('use_pager', '0');
  $handler->override_option('block_description', 'My groups');
  $handler->override_option('block_caching', -1);
  $views[$view->name] = $view;

  /*
   * View 'og_mytracker'
   */
  $view = new view;
  $view->name = 'og_mytracker';
  $view->description = 'Shows all activity in subscribed groups.';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'group_nid' => array(
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'label' => 'Group',
      'required' => 1,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'type' => array(
      'id' => 'type',
      'table' => 'node',
      'field' => 'type',
      'label' => 'Type',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Author',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'comment_count' => array(
      'id' => 'comment_count',
      'table' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => 'Replies',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'relationship' => 'none',
    ),
    'group_nid' => array(
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'label' => 'Groups',
      'type' => 'separator',
      'separator' => ', ',
      'empty' => '',
      'relationship' => 'none',
    ),
    'last_comment_timestamp' => array(
      'id' => 'last_comment_timestamp',
      'table' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => 'Last post',
      'date_format' => 'small',
      'custom_date_format' => '',
      'relationship' => 'none',
    ),
    'timestamp' => array(
      'id' => 'timestamp',
      'table' => 'history_user',
      'field' => 'timestamp',
      'label' => '',
      'comments' => 1,
      'relationship' => 'none',
      'link_to_node' => 0,
    ),
    'new_comments' => array(
      'id' => 'new_comments',
      'table' => 'node',
      'field' => 'new_comments',
      'label' => '',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => 'new',
      'link_to_comment' => 1,
      'no_empty' => 1,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'uid' => array(
      'operator' => '=',
      'value' => 1,
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'uid',
      'table' => 'og_uid',
      'field' => 'uid',
      'relationship' => 'group_nid',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Recent posts in my groups');
  $handler->override_option('empty', 'There are no posts in your groups.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', 25);
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'override' => 1,
    'order' => 'desc',
    'columns' => array(
      'type' => 'type',
      'title' => 'title',
      'name' => 'name',
      'comment_count' => 'comment_count',
      'group_nid' => 'group_nid',
      'last_comment_timestamp' => 'last_comment_timestamp',
      'timestamp' => 'title',
      'new_comments' => 'comment_count',
    ),
    'info' => array(
      'type' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'comment_count' => array(
        'sortable' => 1,
        'separator' => '<br />',
      ),
      'group_nid' => array(
        'separator' => '',
      ),
      'last_comment_timestamp' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'timestamp' => array(
        'separator' => '',
      ),
      'new_comments' => array(
        'separator' => '',
      ),
    ),
    'default' => 'last_comment_timestamp',
  ));
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'group/mytracker');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'My Recent',
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
  $handler = $view->new_display('feed', 'Feed', 'feed');
  $handler->override_option('style_plugin', 'rss');
  $handler->override_option('style_options', array(
    'mission_description' => FALSE,
    'description' => '',
  ));
  $handler->override_option('row_plugin', 'node_rss');
  $handler->override_option('row_options', array(
    'item_length' => 'default',
  ));
  $handler->override_option('path', 'group/tracker/feed');
  $handler->override_option('menu', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('displays', array(
    'default' => 'default',
    'page' => 'page',
  ));
  $handler->override_option('sitename_title', FALSE);
  $views[$view->name] = $view;

  /*
   * View 'og_recent_type_term'
   */
  $view = new view;
  $view->name = 'og_recent_type_term';
  $view->description = 'Recent posts for current term filtered by node type and/or taxonomy term';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'timestamp' => array(
      'id' => 'timestamp',
      'table' => 'history_user',
      'field' => 'timestamp',
      'label' => '',
      'comments' => 1,
      'relationship' => 'none',
      'link_to_node' => 0,
    ),
  ));
  $handler->override_option('sorts', array(
    'sticky' => array(
      'id' => 'sticky',
      'table' => 'node',
      'field' => 'sticky',
      'order' => 'DESC',
      'relationship' => 'none',
    ),
    'created' => array(
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'order' => 'DESC',
      'granularity' => 'second',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'type' => array(
      'id' => 'type',
      'table' => 'node',
      'field' => 'type',
      'default_action' => 'ignore',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '%1',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'blog' => 0,
        'poll' => 0,
        'book' => 0,
        'group1' => 0,
        'group2' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_php' => '',
    ),
    'tid' => array(
      'id' => 'tid',
      'table' => 'term_node',
      'field' => 'tid',
      'default_action' => 'ignore',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '%1 - %2',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'blog' => 0,
        'poll' => 0,
        'book' => 0,
        'group1' => 0,
        'group2' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'picg' => array(
      'id' => 'picg',
      'table' => 'og_ancestry',
      'field' => 'picg',
      'operator' => '=',
      'value' => '',
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('use_pager', 'mini');
  $handler->override_option('style_plugin', 'list');
  $handler->override_option('style_options', array(
    'type' => 'ul',
  ));
  $handler->override_option('row_options', array(
    'inline' => array(
      'title' => 'title',
      'timestamp' => 'timestamp',
    ),
    'separator' => '&nbsp;',
  ));
  $views[$view->name] = $view;

  /*
   * View 'og_search'
   */
  $view = new view;
  $view->name = 'og_search';
  $view->description = 'Search within a particular group.';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('sorts', array(
    'score' => array(
      'order' => 'DESC',
      'id' => 'score',
      'table' => 'search_index',
      'field' => 'score',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('arguments', array(
    'group_nid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => 'Search in group %1',
      'breadcrumb' => '',
      'default_argument_type' => 'og_group_node',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'book' => 0,
        'category' => 0,
        'country' => 0,
        'destination' => 0,
        'group' => 0,
        'image' => 0,
        'page' => 0,
        'poi' => 0,
        'post' => 0,
        'story' => 0,
        'webform' => 0,
      ),
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
      'default_argument_user' => 0,
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_user_argument_type' => 'uid',
      'validate_user_roles' => array(
        '2' => 0,
      ),
      'override' => array(
        'button' => 'Override',
      ),
      'default_options_div_prefix' => '',
      'validate_argument_transform' => 0,
      'validate_user_restrict_roles' => 0,
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_is_member' => '0',
      'validate_argument_group_node_type' => array(
        'country' => 0,
        'group' => 0,
      ),
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'keys' => array(
      'operator' => 'required',
      'value' => '',
      'group' => '0',
      'exposed' => TRUE,
      'expose' => array(
        'use_operator' => 0,
        'operator' => 'keys_op',
        'identifier' => 'keys',
        'label' => '',
        'optional' => 1,
        'remember' => 1,
      ),
      'id' => 'keys',
      'table' => 'search_index',
      'field' => 'keys',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'search content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Search');
  $handler->override_option('empty', 'No posts found.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('use_pager', '1');
  $handler->override_option('row_plugin', 'node');
  $handler->override_option('row_options', array(
    'teaser' => 1,
    'links' => 1,
  ));
  $handler = $view->new_display('page', 'Page', 'page_1');
  $handler->override_option('arguments', array(
    'group_nid' => array(
      'default_action' => 'not found',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => TRUE,
        'override' => FALSE,
        'items_per_page' => 25,
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => 'Search results for %1',
      'breadcrumb' => '',
      'default_argument_type' => 'og_group_node',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'add_table' => 0,
      'relationship' => 'none',
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'book' => 0,
        'category' => 0,
        'country' => 0,
        'destination' => 0,
        'group' => 0,
        'image' => 0,
        'page' => 0,
        'poi' => 0,
        'post' => 0,
        'story' => 0,
        'webform' => 0,
      ),
      'validate_argument_php' => '',
      'require_value' => 0,
      'reduce_duplicates' => 0,
      'default_argument_user' => 0,
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_user_argument_type' => 'uid',
      'validate_user_roles' => array(
        '2' => 0,
      ),
      'override' => array(
        'button' => 'Use default',
      ),
      'default_options_div_prefix' => '',
      'validate_argument_transform' => 0,
      'validate_user_restrict_roles' => 0,
      'validate_argument_node_flag_name' => '*relationship*',
      'validate_argument_node_flag_test' => 'flaggable',
      'validate_argument_node_flag_id_type' => 'id',
      'validate_argument_user_flag_name' => '*relationship*',
      'validate_argument_user_flag_test' => 'flaggable',
      'validate_argument_user_flag_id_type' => 'id',
      'validate_argument_is_member' => '0',
      'validate_argument_group_node_type' => array(
        'country' => 0,
        'group' => 0,
      ),
    ),
  ));
  $handler->override_option('path', 'og/search');
  $handler->override_option('menu', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $views[$view->name] = $view;

  /*
   * View 'og_tracker'
   */
  $view = new view;
  $view->name = 'og_tracker';
  $view->description = 'Shows active posts across the whole site.';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'type' => array(
      'id' => 'type',
      'table' => 'node',
      'field' => 'type',
      'label' => 'Type',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Author',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'comment_count' => array(
      'id' => 'comment_count',
      'table' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => 'Replies',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'relationship' => 'none',
    ),
    'group_nid' => array(
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'label' => 'Groups',
      'type' => 'separator',
      'separator' => ', ',
      'empty' => '',
      'relationship' => 'none',
    ),
    'last_comment_timestamp' => array(
      'id' => 'last_comment_timestamp',
      'table' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => 'Last post',
      'date_format' => 'small',
      'custom_date_format' => '',
      'relationship' => 'none',
    ),
    'timestamp' => array(
      'id' => 'timestamp',
      'table' => 'history_user',
      'field' => 'timestamp',
      'label' => '',
      'comments' => 1,
      'relationship' => 'none',
      'link_to_node' => 0,
    ),
    'new_comments' => array(
      'id' => 'new_comments',
      'table' => 'node',
      'field' => 'new_comments',
      'label' => '',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => 'new',
      'link_to_comment' => 1,
      'no_empty' => 1,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'last_comment_timestamp' => array(
      'id' => 'last_comment_timestamp',
      'table' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'order' => 'DESC',
      'granularity' => 'second',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Recent posts across whole site');
  $handler->override_option('items_per_page', 25);
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'override' => 1,
    'order' => 'desc',
    'columns' => array(
      'type' => 'type',
      'title' => 'title',
      'name' => 'name',
      'comment_count' => 'comment_count',
      'group_nid' => 'group_nid',
      'last_comment_timestamp' => 'last_comment_timestamp',
      'timestamp' => 'title',
      'new_comments' => 'comment_count',
    ),
    'info' => array(
      'type' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'comment_count' => array(
        'sortable' => 1,
        'separator' => '<br />',
      ),
      'group_nid' => array(
        'separator' => '',
      ),
      'last_comment_timestamp' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'timestamp' => array(
        'separator' => '',
      ),
      'new_comments' => array(
        'separator' => '',
      ),
    ),
    'default' => 'last_comment_timestamp',
  ));
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'group/tracker');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'Recent posts',
    'description' => '',
    'weight' => '5',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler = $view->new_display('feed', 'Feed', 'feed');
  $handler->override_option('style_plugin', 'rss');
  $handler->override_option('style_options', array(
    'mission_description' => FALSE,
    'description' => '',
  ));
  $handler->override_option('row_plugin', 'node_rss');
  $handler->override_option('row_options', array(
    'item_length' => 'default',
  ));
  $handler->override_option('path', 'group/tracker/feed');
  $handler->override_option('menu', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('displays', array(
    'page' => 'page',
    'default' => 0,
  ));
  $handler->override_option('sitename_title', FALSE);
  $views[$view->name] = $view;

  /*
   * View 'og_unread'
   */
  $view = new view;
  $view->name = 'og_unread';
  $view->description = 'Shows unread posts in subscribed groups.';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('relationships', array(
    'group_nid' => array(
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'label' => 'Group',
      'required' => 1,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'type' => array(
      'id' => 'type',
      'table' => 'node',
      'field' => 'type',
      'label' => 'Type',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'relationship' => 'none',
      'link_to_node' => 1,
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Author',
      'link_to_user' => 1,
      'relationship' => 'none',
    ),
    'comment_count' => array(
      'id' => 'comment_count',
      'table' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => 'Replies',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'relationship' => 'none',
    ),
    'group_nid' => array(
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'label' => 'Groups',
      'type' => 'separator',
      'separator' => ', ',
      'empty' => '',
      'relationship' => 'none',
    ),
    'last_comment_timestamp' => array(
      'id' => 'last_comment_timestamp',
      'table' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => 'Last post',
      'date_format' => 'small',
      'custom_date_format' => '',
      'relationship' => 'none',
    ),
    'new_comments' => array(
      'id' => 'new_comments',
      'table' => 'node',
      'field' => 'new_comments',
      'label' => '',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => 'new',
      'link_to_comment' => 1,
      'no_empty' => 1,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'timestamp' => array(
      'id' => 'timestamp',
      'table' => 'history_user',
      'field' => 'timestamp',
      'operator' => '=',
      'value' => '',
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'relationship' => 'none',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
    'uid' => array(
      'operator' => '=',
      'value' => 1,
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'uid',
      'table' => 'og_uid',
      'field' => 'uid',
      'relationship' => 'group_nid',
      'expose_button' => array(
        'button' => 'Expose',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'role',
    'role' => array(
      '2' => 2,
    ),
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Unread posts in my groups');
  $handler->override_option('empty', 'There are no new posts in your groups.');
  $handler->override_option('empty_format', '1');
  $handler->override_option('items_per_page', 25);
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'override' => 1,
    'order' => 'desc',
    'columns' => array(
      'type' => 'type',
      'title' => 'title',
      'name' => 'name',
      'comment_count' => 'comment_count',
      'group_nid' => 'group_nid',
      'last_comment_timestamp' => 'last_comment_timestamp',
      'new_comments' => 'comment_count',
    ),
    'info' => array(
      'type' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'comment_count' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'group_nid' => array(
        'separator' => '',
      ),
      'last_comment_timestamp' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'new_comments' => array(
        'separator' => '',
      ),
    ),
    'default' => 'last_comment_timestamp',
  ));
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'group/myunread');
  $handler->override_option('menu', array(
    'type' => 'default tab',
    'title' => 'My Unread',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'normal',
    'title' => 'My Unread',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler = $view->new_display('feed', 'Feed', 'feed');
  $handler->override_option('style_plugin', 'rss');
  $handler->override_option('style_options', array(
    'mission_description' => FALSE,
    'description' => '',
  ));
  $handler->override_option('row_plugin', 'node_rss');
  $handler->override_option('row_options', array(
    'item_length' => 'default',
  ));
  $handler->override_option('path', 'group/myunread/feed');
  $handler->override_option('menu', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'description' => '',
    'weight' => 0,
    'name' => 'navigation',
  ));
  $handler->override_option('displays', array(
    'page' => 'page',
    'default' => 0,
  ));
  $handler->override_option('sitename_title', FALSE);
  $views[$view->name] = $view;

  /*
   * View 'posts_by_term'
   */
  $view = new view;
  $view->name = 'posts_by_term';
  $view->description = 'Shows group posts categorized by taxonomy term';
  $view->tag = 'og';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'type' => array(
      'id' => 'type',
      'table' => 'node',
      'field' => 'type',
      'label' => 'Type',
    ),
    'name_1' => array(
      'label' => 'Term',
      'link_to_taxonomy' => 0,
      'exclude' => 0,
      'id' => 'name_1',
      'table' => 'term_data',
      'field' => 'name',
      'relationship' => 'none',
    ),
    'title' => array(
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'label' => 'Title',
      'link_to_node' => TRUE,
    ),
    'name' => array(
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'label' => 'Author',
      'link_to_user' => TRUE,
    ),
    'comment_count' => array(
      'id' => 'comment_count',
      'table' => 'node_comment_statistics',
      'field' => 'comment_count',
      'label' => 'Replies',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
    ),
    'last_comment_timestamp' => array(
      'id' => 'last_comment_timestamp',
      'table' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'label' => 'Last Post',
      'date_format' => 'small',
      'custom_date_format' => '',
    ),
    'timestamp' => array(
      'id' => 'timestamp',
      'table' => 'history_user',
      'field' => 'timestamp',
      'label' => '',
      'comments' => 1,
      'relationship' => 'none',
      'link_to_node' => 0,
      'comment' => 1,
    ),
    'new_comments' => array(
      'id' => 'new_comments',
      'table' => 'node',
      'field' => 'new_comments',
      'label' => '',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => ' new',
      'link_to_comment' => 1,
      'no_empty' => 1,
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('sorts', array(
    'last_comment_timestamp' => array(
      'id' => 'last_comment_timestamp',
      'table' => 'node_comment_statistics',
      'field' => 'last_comment_timestamp',
      'order' => 'ASC',
      'granularity' => 'second',
    ),
  ));
  $handler->override_option('arguments', array(
    'group_nid' => array(
      'default_action' => 'ignore',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => 1,
        'override' => 0,
        'items_per_page' => '25',
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '%1',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'og_group_types',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'group_nid',
      'table' => 'og_ancestry',
      'field' => 'group_nid',
      'relationship' => 'none',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'abc' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(
        '1' => 0,
      ),
      'validate_argument_type' => 'tid',
      'validate_argument_is_member' => 0,
      'validate_argument_php' => '',
      'override' => array(
        'button' => 'Override',
      ),
    ),
    'tid' => array(
      'default_action' => 'summary asc',
      'style_plugin' => 'default_summary',
      'style_options' => array(
        'count' => 1,
        'override' => 0,
        'items_per_page' => '25',
      ),
      'wildcard' => 'all',
      'wildcard_substitution' => 'All',
      'title' => '',
      'default_argument_type' => 'fixed',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'add_table' => 0,
      'require_value' => 1,
      'reduce_duplicates' => 0,
      'set_breadcrumb' => 0,
      'id' => 'tid',
      'table' => 'term_node',
      'field' => 'tid',
      'relationship' => 'none',
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'abc' => 0,
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(
        '1' => 0,
      ),
      'validate_argument_type' => 'tid',
      'validate_argument_is_member' => 0,
      'validate_argument_php' => '',
      'override' => array(
        'button' => 'Override',
      ),
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'operator' => '=',
      'value' => '1',
      'group' => 0,
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'status' => array(
        'id' => 'status',
        'table' => 'comments',
        'field' => 'status',
        'operator' => '=',
        'value' => 0,
        'group' => 0,
        'exposed' => FALSE,
        'expose' => array(
          'operator' => FALSE,
          'label' => '',
        ),
        'relationship' => 'none',
      ),
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'perm',
    'perm' => 'access content',
  ));
  $handler->override_option('cache', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Posts by term');
  $handler->override_option('items_per_page', '25');
  $handler->override_option('use_pager', TRUE);
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => 'name_1',
    'override' => 1,
    'sticky' => 0,
    'order' => 'desc',
    'columns' => array(
      'type' => 'type',
      'name_1' => 'name_1',
      'title' => 'title',
      'name' => 'name',
      'comment_count' => 'comment_count',
      'last_comment_timestamp' => 'last_comment_timestamp',
      'timestamp' => 'title',
      'new_comments' => 'comment_count',
    ),
    'info' => array(
      'type' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'name_1' => array(
        'sortable' => 0,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'comment_count' => array(
        'sortable' => 1,
        'separator' => '<br />',
      ),
      'last_comment_timestamp' => array(
        'sortable' => 1,
        'separator' => '&nbsp;',
      ),
      'timestamp' => array(
        'separator' => '',
      ),
      'new_comments' => array(
        'separator' => '',
      ),
    ),
    'default' => 'last_comment_timestamp',
  ));
  $handler = $view->new_display('page', 'Page', 'page');
  $handler->override_option('path', 'node/%/by_subject');
  $handler->override_option('menu', array(
    'type' => 'tab',
    'title' => 'By term',
    'description' => '',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => NULL,
    'description' => '',
    'weight' => NULL,
    'name' => 'navigation',
  ));
  $views[$view->name] = $view;

  return $views;
}