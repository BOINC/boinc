<?php
//$Id: user.views.inc,v 1.59.2.3 2010/03/22 20:32:48 merlinofchaos Exp $
/**
 * @file
 * Provide views data and handlers for user.module
 */

/**
 * @defgroup views_user_module user.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function user_views_data() {
  // ----------------------------------------------------------------
  // users table

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['users']['table']['group']  = t('User');

  $data['users']['table']['base'] = array(
    'field' => 'uid',
    'title' => t('User'),
    'help' => t('Users who have created accounts on your site.'),
  );

  $data['users']['table']['join'] = array(
    'node' => array(
      'left_field' => 'uid',
      'field' => 'uid',
      'type' => 'INNER', // all nodes have an author.
    ),
    // This goes to the node so that we have consistent authorship.
    'node_revisions' => array(
      'left_table' => 'node',
      'left_field' => 'uid',
      'field' => 'uid',
      'type' => 'INNER', // all nodes have an author.
    ),
  );

  // uid
  $data['users']['uid'] = array(
    'title' => t('Uid'),
    'help' => t('The user ID'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
      'name field' => 'name', // display this field in the summary
    ),
    'filter' => array(
      'title' => t('Name'),
      'handler' => 'views_handler_filter_user_name',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // uid
  $data['users']['uid_current'] = array(
    'real field' => 'uid',
    'title' => t('Current'),
    'help' => t('Filter the view to the currently logged in user.'),
    'filter' => array(
      'handler' => 'views_handler_filter_user_current',
      'type' => 'yes-no',
    ),
  );

  // name
  $data['users']['name'] = array(
    'title' => t('Name'), // The item it appears as on the UI,
    'help' => t('The user or author name.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_user_name',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // mail
  // Note that this field implements field level access control.
  $data['users']['mail'] = array(
    'title' => t('E-mail'), // The item it appears as on the UI,
    'help' => t('Email address for a given user. This field is normally not shown to users, so be cautious when using it.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_user_mail',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // language
  $data['users']['language'] = array(
    'title' => t('Language'), // The item it appears as on the UI,
    'help' => t('Language of the user'),
    'field' => array(
      'handler' => 'views_handler_field_user_language',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_node_language',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_node_language',
    ),
  );

  // picture
  $data['users']['picture'] = array(
    'title' => t('Picture'),
    'help' => t("The user's picture, if allowed."), // The help that appears on the UI,
    // Information for displaying the uid
    'field' => array(
      'handler' => 'views_handler_field_user_picture',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator_string',
      'label' => t('Has Avatar'),
      'type' => 'yes-no',
    ),
  );

    // created field
  $data['users']['created'] = array(
    'title' => t('Created date'), // The item it appears as on the UI,
    'help' => t('The date the user was created.'), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  // access field
  $data['users']['access'] = array(
    'title' => t('Last access'), // The item it appears as on the UI,
    'help' => t("The user's last access date."), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  // login field
  $data['users']['login'] = array(
    'title' => t('Last login'), // The item it appears as on the UI,
    'help' => t("The user's last login date."), // The help that appears on the UI,
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  // published status
  $data['users']['status'] = array(
    'title' => t('Active'), // The item it appears as on the UI,
    'help' => t('Whether a user is active or blocked.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Active'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // log field
  $data['users']['signature'] = array(
    'title' => t('Signature'), // The item it appears as on the UI,
    'help' => t("The user's signature."), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_xss',
     ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
  );

  $data['users']['edit_node'] = array(
    'field' => array(
      'title' => t('Edit link'),
      'help' => t('Provide a simple link to edit the user.'),
      'handler' => 'views_handler_field_user_link_edit',
    ),
  );

  $data['users']['delete_node'] = array(
    'field' => array(
      'title' => t('Delete link'),
      'help' => t('Provide a simple link to delete the user.'),
      'handler' => 'views_handler_field_user_link_delete',
    ),
  );

  // ----------------------------------------------------------------------
  // users_roles table

  $data['users_roles']['table']['group']  = t('User');

  // Explain how this table joins to others.
  $data['users_roles']['table']['join'] = array(
     // Directly links to users table.
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node_revisions' => array(
      'left_table' => 'node',
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );

  $data['users_roles']['rid'] = array(
    'title' => t('Roles'),
    'help' => t('Roles that a user belongs to.'),
    'field' => array(
      'handler' => 'views_handler_field_user_roles',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_user_roles',
      'numeric' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_users_roles_rid',
      'name table' => 'role',
      'name field' => 'name',
      'empty field name' => t('No role'),
      'numeric' => TRUE,
    ),
  );

  // ----------------------------------------------------------------------
  // role table

  $data['role']['table']['join'] = array(
     // Directly links to users table.
    'users' => array(
      'left_table' => 'users_roles',
      'left_field' => 'rid',
      'field' => 'rid',
    ),
    // needed for many to one helper sometimes
    'users_roles' => array(
      'left_field' => 'rid',
      'field' => 'rid',
    ),
    'node' => array(
      'left_table' => 'users_roles',
      'left_field' => 'rid',
      'field' => 'rid',
    ),
    'node_revisions' => array(
      'left_table' => 'users_roles',
      'left_field' => 'rid',
      'field' => 'rid',
    ),
  );

  // ----------------------------------------------------------------------
  // authmap table
  
  $data['authmap']['table']['group']  = t('User');
  $data['authmap']['table']['join'] = array(
     // Directly links to users table.
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
    'node' => array(
      'left_table' => 'users',
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );
  $data['authmap']['aid'] = array(
    'title' => t('Authmap ID'),
    'help' => t('The Authmap ID.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
      'numeric' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
      'numeric' => TRUE,
    ),
  );
  $data['authmap']['authname'] = array(
    'title' => t('Authentication name'),
    'help' => t('The unique authentication name.'),
    'field' => array(
      'handler' => 'views_handler_field',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  $data['authmap']['module'] = array(
    'title' => t('Authentication module'),
    'help' => t('The name of the module managing the authentication entry.'),
    'field' => array(
      'handler' => 'views_handler_field',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );
  
  return $data;
}

/**
 * Implementation of hook_views_plugins
 */
function user_views_plugins() {
  return array(
    'module' => 'views', // This just tells our themes are elsewhere.
    'argument default' => array(
      'user' => array(
        'title' => t('User ID from URL'),
        'handler' => 'views_plugin_argument_default_user',
        'path' => drupal_get_path('module', 'views') . '/modules/user', // not necessary for most modules
        'parent' => 'fixed', // so that the parent class is included
      ),
      'current_user' => array(
        'title' => t('User ID from logged in user'),
        'handler' => 'views_plugin_argument_default_current_user',
        'path' => drupal_get_path('module', 'views') . '/modules/user', // not necessary for most modules
        'parent' => 'fixed', // so that the parent class is included
      ),
    ),
    'argument validator' => array(
      'user' => array(
        'title' => t('User'),
        'handler' => 'views_plugin_argument_validate_user',
        'path' => drupal_get_path('module', 'views') . '/modules/user', // not necessary for most modules
      ),
    ),
  );
}

/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function user_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/user',
    ),
    'handlers' => array(
      'views_handler_argument_user_uid' => array(
        'parent' => 'views_handler_argument_numeric',
      ),
      'views_handler_field_user' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_user_name' => array(
        'parent' => 'views_handler_field_user',
      ),
      'views_handler_field_user_link' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_user_link_edit' => array(
        'parent' => 'views_handler_field_user_link',
      ),
      'views_handler_field_user_link_delete' => array(
        'parent' => 'views_handler_field_user_link',
      ),
      'views_handler_field_user_mail' => array(
        'parent' => 'views_handler_field_user',
      ),
      'views_handler_field_user_language' => array(
        'parent' => 'views_handler_field_user',
      ),
      'views_handler_field_user_picture' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_user_roles' => array(
        'parent' => 'views_handler_field_prerender_list',
      ),
      'views_handler_filter_user_name' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_filter_user_current' => array(
        'parent' => 'views_handler_filter_boolean_operator',
      ),
      'views_handler_filter_user_roles' => array(
        'parent' => 'views_handler_filter_many_to_one',
      ),
      'views_handler_argument_users_roles_rid' => array(
        'parent' => 'views_handler_argument_many_to_one',
      ),
    ),
  );
}


/**
 * Allow replacement of current userid so we can cache these queries
 */
function user_views_query_substitutions($view) {
  global $user;
  return array('***CURRENT_USER***' => intval($user->uid));
}

/**
 * @}
 */
