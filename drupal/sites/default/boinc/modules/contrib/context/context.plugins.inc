<?php

/**
 * Context registry.
 */
function _context_context_registry() {
  $registry = array();
  $registry['conditions'] = array(
    'context' => array(
      'title' => t('Context'),
      'description' => t('Set this context on the basis of other active contexts. Put each context on a separate line. You can use the <code>*</code> character as a wildcard and <code>~</code> to exclude one or more contexts.'),
      'plugin' => 'context_condition_context',
    ),
    'node' => array(
      'title' => t('Node type'),
      'description' => t('Set this context when viewing a node page or using the add/edit form of one of these content types.'),
      'plugin' => 'context_condition_node',
    ),
    'sitewide' => array(
      'title' => t('Sitewide context'),
      'description' => t('Should this context always be set? If <strong>true</strong>, this context will be active across your entire site.'),
      'plugin' => 'context_condition_sitewide',
    ),
    'path' => array(
      'title' => t('Path'),
      'description' => t('Set this context when any of the paths above match the page path. Put each path on a separate line. You can use the "*" character as a wildcard and <code>~</code> to exclude one or more paths. Use &lt;front&gt; for the site front page.'),
      'plugin' => 'context_condition_path',
    ),
    'user' => array(
      'title' => t('User role'),
      'description' => t('Set this context when the current user has one of the selected role(s).'),
      'plugin' => 'context_condition_user',
    ),
    'user_page' => array(
      'title' => t('User page'),
      'description' => t('Set this context when viewing a user page.'),
      'plugin' => 'context_condition_user_page',
    ),
  );
  if (module_exists('menu')) {
    $registry['conditions']['menu'] = array(
      'title' => t('Menu'),
      'description' => t('Set this context when any of the selected menu items belong to the current active menu trail.'),
      'plugin' => 'context_condition_menu',
    );
  }
  if (module_exists('views')) {
    $registry['conditions']['views'] = array(
      'title' => t('Views'),
      'description' => t('Set this context when displaying the page of one of these views.'),
      'plugin' => 'context_condition_views',
    );
  }
  if (module_exists('book')) {
    $registry['conditions']['book'] = array(
      'title' => t('Book'),
      'description' => t('Set this context when a node in the selected book is viewed.'),
      'plugin' => 'context_condition_book',
    );
    $registry['conditions']['bookroot'] = array(
      'title' => t('Book root'),
      'description' => t('Set this context when viewing a node whose root book is of the selected type.'),
      'plugin' => 'context_condition_bookroot',
    );
  }
  if (module_exists('locale')) {
    $registry['conditions']['language'] = array(
      'title' => t('Language'),
      'description' => t('Set this context when viewing the site in the selected language.'),
      'plugin' => 'context_condition_language',
    );
  }
  if (module_exists('taxonomy')) {
    $registry['conditions']['node_taxonomy'] = array(
      'title' => t('Taxonomy'),
      'description' => t('Set this context when viewing a node with the selected taxonomy terms.'),
      'plugin' => 'context_condition_node_taxonomy',
    );
  }
  $registry['reactions'] = array(
    'block' => array(
      'title' => t('Blocks'),
      'description' => t('Control block visibility using context.'),
      'plugin' => 'context_reaction_block',
    ),
    'breadcrumb' => array(
      'title' => t('Breadcrumb'),
      'description' => t('Set the breadcrumb trail to the selected menu item.'),
      'plugin' => 'context_reaction_breadcrumb',
    ),
    'theme' => array(
      'title' => t('Theme'),
      'description' => t('Control theme variables using context.'),
      'plugin' => 'context_reaction_theme',
    ),
    'debug' => array(
      'title' => t('Debug'),
      'description' => t('Debug output reaction for SimpleTest.'),
      'plugin' => 'context_reaction_debug',
    ),
  );
  if (module_exists('menu')) {
    $registry['reactions']['menu'] = array(
      'title' => t('Menu'),
      'description' => t('Control menu active class using context.'),
      'plugin' => 'context_reaction_menu',
    );
  }
  if (module_exists('css_injector')) {
    $registry['reactions']['css_injector'] = array(
      'title' => t('CSS Injector'),
      'description' => t('Inject the selected css when this context is set.'),
      'plugin' => 'context_reaction_css_injector',
    );
  }
  return $registry;
}

/**
 * Context plugins.
 */
function _context_context_plugins() {
  $plugins = array();

  /**
   * Conditions.
   */
  $plugins['context_condition'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition.inc',
      'class' => 'context_condition',
    ),
  );
  $plugins['context_condition_context'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_context.inc',
      'class' => 'context_condition_context',
      'parent' => 'context_condition_path',
    ),
  );
  $plugins['context_condition_node'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_node.inc',
      'class' => 'context_condition_node',
      'parent' => 'context_condition',
    ),
  );
  $plugins['context_condition_sitewide'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_sitewide.inc',
      'class' => 'context_condition_sitewide',
      'parent' => 'context_condition',
    ),
  );
  $plugins['context_condition_path'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_path.inc',
      'class' => 'context_condition_path',
      'parent' => 'context_condition',
    ),
  );
  $plugins['context_condition_user'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_user.inc',
      'class' => 'context_condition_user',
      'parent' => 'context_condition',
    ),
  );
  $plugins['context_condition_user_page'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_user_page.inc',
      'class' => 'context_condition_user_page',
      'parent' => 'context_condition',
    ),
  );
  $plugins['context_condition_menu'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_condition_menu.inc',
      'class' => 'context_condition_menu',
      'parent' => 'context_condition',
    ),
  );
  if (module_exists('taxonomy')) {
    $plugins['context_condition_node_taxonomy'] = array(
      'handler' => array(
        'path' => drupal_get_path('module', 'context') .'/plugins',
        'file' => 'context_condition_node_taxonomy.inc',
        'class' => 'context_condition_node_taxonomy',
        'parent' => 'context_condition_node',
      ),
    );
  }
  if (module_exists('locale')) {
    $plugins['context_condition_language'] = array(
      'handler' => array(
        'path' => drupal_get_path('module', 'context') .'/plugins',
        'file' => 'context_condition_language.inc',
        'class' => 'context_condition_language',
        'parent' => 'context_condition',
      ),
    );
  }
  if (module_exists('book')) {
    $plugins['context_condition_book'] = array(
      'handler' => array(
        'path' => drupal_get_path('module', 'context') .'/plugins',
        'file' => 'context_condition_book.inc',
        'class' => 'context_condition_book',
        'parent' => 'context_condition',
      ),
    );
    $plugins['context_condition_bookroot'] = array(
      'handler' => array(
        'path' => drupal_get_path('module', 'context') .'/plugins',
        'file' => 'context_condition_bookroot.inc',
        'class' => 'context_condition_bookroot',
        'parent' => 'context_condition_node',
      ),
    );
  }
  if (module_exists('views')) {
    $plugins['context_condition_views'] = array(
      'handler' => array(
        'path' => drupal_get_path('module', 'context') .'/plugins',
        'file' => 'context_condition_views.inc',
        'class' => 'context_condition_views',
        'parent' => 'context_condition',
      ),
    );
  }

  /**
   * Reactions.
   */
  $plugins['context_reaction'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_reaction.inc',
      'class' => 'context_reaction',
    ),
  );
  $plugins['context_reaction_block'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_reaction_block.inc',
      'class' => 'context_reaction_block',
      'parent' => 'context_reaction',
    ),
  );
  $plugins['context_reaction_breadcrumb'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_reaction_breadcrumb.inc',
      'class' => 'context_reaction_breadcrumb',
      'parent' => 'context_reaction_menu',
    ),
  );
  $plugins['context_reaction_menu'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_reaction_menu.inc',
      'class' => 'context_reaction_menu',
      'parent' => 'context_reaction',
    ),
  );
  $plugins['context_reaction_theme'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_reaction_theme.inc',
      'class' => 'context_reaction_theme',
      'parent' => 'context_reaction',
    ),
  );
  $plugins['context_reaction_debug'] = array(
    'handler' => array(
      'path' => drupal_get_path('module', 'context') .'/plugins',
      'file' => 'context_reaction_debug.inc',
      'class' => 'context_reaction_debug',
      'parent' => 'context_reaction',
    ),
  );
  if (module_exists('css_injector')) {
    $plugins['context_reaction_css_injector'] = array(
      'handler' => array(
        'path' => drupal_get_path('module', 'context') .'/plugins',
        'file' => 'context_reaction_css_injector.inc',
        'class' => 'context_reaction_css_injector',
        'parent' => 'context_reaction',
      ),
    );
  }
  return $plugins;
}
