<?php

function i18nviews_views_data() {
  $data['node']['content_negotiation'] = array(
    'group' => t('Node translation'),
    'title' => t('Content negotiation'),
    'help' => t('Removes the nodes that are not valid according to the content selection mode.'),
    'filter' => array(
      'handler' => 'content_negotiation_filter_handler',
    ),
  );
  return $data;
}

/**
 * Implementation of hook_views_handlers
 */
function i18nviews_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'i18nviews') . '/includes',
    ),
    'handlers' => array(
      'content_negotiation_filter_handler' => array(
        'parent' => 'views_handler_filter',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_plugins().
 */
function i18nviews_views_plugins() {
  $path = drupal_get_path('module', 'i18nviews') . '/includes';
  return array(
    'module' => 'i18nviews',
    'localization' => array(
      'i18nstrings' => array(
        'title' => t('Views translation module'),
        'help' => t("Use the locale system as implemented by the Views translation module."),
        'handler' => 'views_plugin_localization_i18nstrings',
        'path' => $path,
      ),
    ),
  );
}