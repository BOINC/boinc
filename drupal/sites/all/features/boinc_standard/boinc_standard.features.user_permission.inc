<?php

/**
 * Implementation of hook_user_default_permissions().
 */
function boinc_standard_user_default_permissions() {
  $permissions = array();

  // Exported permission: Use PHP input for field settings (dangerous - grant with care)
  $permissions['Use PHP input for field settings (dangerous - grant with care)'] = array(
    'name' => 'Use PHP input for field settings (dangerous - grant with care)',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: access administration pages
  $permissions['access administration pages'] = array(
    'name' => 'access administration pages',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: access all views
  $permissions['access all views'] = array(
    'name' => 'access all views',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: access ckeditor
  $permissions['access ckeditor'] = array(
    'name' => 'access ckeditor',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
      '3' => 'community member',
      '4' => 'content editor',
      '5' => 'moderator',
      '6' => 'verified contributor',
    ),
  );

  // Exported permission: access content
  $permissions['access content'] = array(
    'name' => 'access content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'anonymous user',
      '2' => 'authenticated user',
    ),
  );

  // Exported permission: access site reports
  $permissions['access site reports'] = array(
    'name' => 'access site reports',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer actions
  $permissions['administer actions'] = array(
    'name' => 'administer actions',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer advanced pane settings
  $permissions['administer advanced pane settings'] = array(
    'name' => 'administer advanced pane settings',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer blocks
  $permissions['administer blocks'] = array(
    'name' => 'administer blocks',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer ckeditor
  $permissions['administer ckeditor'] = array(
    'name' => 'administer ckeditor',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer content types
  $permissions['administer content types'] = array(
    'name' => 'administer content types',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer elysia_cron
  $permissions['administer elysia_cron'] = array(
    'name' => 'administer elysia_cron',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer features
  $permissions['administer features'] = array(
    'name' => 'administer features',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer files
  $permissions['administer files'] = array(
    'name' => 'administer files',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer filters
  $permissions['administer filters'] = array(
    'name' => 'administer filters',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer flags
  $permissions['administer flags'] = array(
    'name' => 'administer flags',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer imce(execute PHP)
  $permissions['administer imce(execute PHP)'] = array(
    'name' => 'administer imce(execute PHP)',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer menu
  $permissions['administer menu'] = array(
    'name' => 'administer menu',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: administer mini panels
  $permissions['administer mini panels'] = array(
    'name' => 'administer mini panels',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer nodes
  $permissions['administer nodes'] = array(
    'name' => 'administer nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer page manager
  $permissions['administer page manager'] = array(
    'name' => 'administer page manager',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer pane access
  $permissions['administer pane access'] = array(
    'name' => 'administer pane access',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer pane visibility
  $permissions['administer pane visibility'] = array(
    'name' => 'administer pane visibility',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer panel-nodes
  $permissions['administer panel-nodes'] = array(
    'name' => 'administer panel-nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer panels layouts
  $permissions['administer panels layouts'] = array(
    'name' => 'administer panels layouts',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer pathauto
  $permissions['administer pathauto'] = array(
    'name' => 'administer pathauto',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer permissions
  $permissions['administer permissions'] = array(
    'name' => 'administer permissions',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer rules
  $permissions['administer rules'] = array(
    'name' => 'administer rules',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer site configuration
  $permissions['administer site configuration'] = array(
    'name' => 'administer site configuration',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer smtp module
  $permissions['administer smtp module'] = array(
    'name' => 'administer smtp module',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer tabtamer
  $permissions['administer tabtamer'] = array(
    'name' => 'administer tabtamer',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer taxonomy
  $permissions['administer taxonomy'] = array(
    'name' => 'administer taxonomy',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer url aliases
  $permissions['administer url aliases'] = array(
    'name' => 'administer url aliases',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer users
  $permissions['administer users'] = array(
    'name' => 'administer users',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: administer views
  $permissions['administer views'] = array(
    'name' => 'administer views',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: assign all roles
  $permissions['assign all roles'] = array(
    'name' => 'assign all roles',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: change own username
  $permissions['change own username'] = array(
    'name' => 'change own username',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create mini panels
  $permissions['create mini panels'] = array(
    'name' => 'create mini panels',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create page content
  $permissions['create page content'] = array(
    'name' => 'create page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: create panel-nodes
  $permissions['create panel-nodes'] = array(
    'name' => 'create panel-nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create story content
  $permissions['create story content'] = array(
    'name' => 'create story content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: create url aliases
  $permissions['create url aliases'] = array(
    'name' => 'create url aliases',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete any page content
  $permissions['delete any page content'] = array(
    'name' => 'delete any page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: delete any panel-nodes
  $permissions['delete any panel-nodes'] = array(
    'name' => 'delete any panel-nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete any story content
  $permissions['delete any story content'] = array(
    'name' => 'delete any story content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete own account
  $permissions['delete own account'] = array(
    'name' => 'delete own account',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'authenticated user',
    ),
  );

  // Exported permission: delete own page content
  $permissions['delete own page content'] = array(
    'name' => 'delete own page content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete own panel-nodes
  $permissions['delete own panel-nodes'] = array(
    'name' => 'delete own panel-nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete own story content
  $permissions['delete own story content'] = array(
    'name' => 'delete own story content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: delete revisions
  $permissions['delete revisions'] = array(
    'name' => 'delete revisions',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: edit any page content
  $permissions['edit any page content'] = array(
    'name' => 'edit any page content',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: edit any panel-nodes
  $permissions['edit any panel-nodes'] = array(
    'name' => 'edit any panel-nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit any story content
  $permissions['edit any story content'] = array(
    'name' => 'edit any story content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit own page content
  $permissions['edit own page content'] = array(
    'name' => 'edit own page content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit own panel-nodes
  $permissions['edit own panel-nodes'] = array(
    'name' => 'edit own panel-nodes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: edit own story content
  $permissions['edit own story content'] = array(
    'name' => 'edit own story content',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: ignore user
  $permissions['ignore user'] = array(
    'name' => 'ignore user',
    'roles' => array(
      '0' => 'authenticated user',
    ),
  );

  // Exported permission: import input formats
  $permissions['import input formats'] = array(
    'name' => 'import input formats',
    'roles' => array(),
  );

  // Exported permission: manage features
  $permissions['manage features'] = array(
    'name' => 'manage features',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: notify of path changes
  $permissions['notify of path changes'] = array(
    'name' => 'notify of path changes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: reset abuse flags
  $permissions['reset abuse flags'] = array(
    'name' => 'reset abuse flags',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'moderator',
    ),
  );

  // Exported permission: revert revisions
  $permissions['revert revisions'] = array(
    'name' => 'revert revisions',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: select different theme
  $permissions['select different theme'] = array(
    'name' => 'select different theme',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: upload files
  $permissions['upload files'] = array(
    'name' => 'upload files',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  // Exported permission: view uploaded files
  $permissions['view uploaded files'] = array(
    'name' => 'view uploaded files',
    'roles' => array(),
  );

  // Exported permission: use PHP for block visibility
  $permissions['use PHP for block visibility'] = array(
    'name' => 'use PHP for block visibility',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use PHP for me alias paths
  $permissions['use PHP for me alias paths'] = array(
    'name' => 'use PHP for me alias paths',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use flag import
  $permissions['use flag import'] = array(
    'name' => 'use flag import',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use page manager
  $permissions['use page manager'] = array(
    'name' => 'use page manager',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use panels caching features
  $permissions['use panels caching features'] = array(
    'name' => 'use panels caching features',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use panels dashboard
  $permissions['use panels dashboard'] = array(
    'name' => 'use panels dashboard',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: use panels in place editing
  $permissions['use panels in place editing'] = array(
    'name' => 'use panels in place editing',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view all panes
  $permissions['view all panes'] = array(
    'name' => 'view all panes',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view pane admin links
  $permissions['view pane admin links'] = array(
    'name' => 'view pane admin links',
    'roles' => array(
      '0' => 'administrator',
    ),
  );

  // Exported permission: view revisions
  $permissions['view revisions'] = array(
    'name' => 'view revisions',
    'roles' => array(
      '0' => 'administrator',
      '1' => 'content editor',
    ),
  );

  return $permissions;
}
