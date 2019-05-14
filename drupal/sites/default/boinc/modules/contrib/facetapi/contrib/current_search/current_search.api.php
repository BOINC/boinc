<?php

/**
 * @file
 * Hooks provided by the Current Search Blocks module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define all current search item provided by the module.
 *
 * Current search items are elements that are added to the current search block
 * such as a list of active facet items, custom text, etc.
 *
 * @return array
 *   An associative array keyed by unique name of the current search item. Each
 *   item item is an associative array keyed by "handler" containing:
 *   - label: The human readable name of the plugin displayed in the admin UI.
 *   - class: The name of the plugin class.
 *
 * @see CurrentSearchItem
 */
function hook_current_search_items() {
  return array(
    'text' => array(
      'handler' => array(
        'label' => t('Custom text'),
        'class' => 'CurrentSearchItemText',
      ),
    ),
  );
}

/**
 * @} End of "addtogroup hooks".
 */
