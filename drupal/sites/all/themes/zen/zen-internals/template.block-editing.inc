<?php
/**
 * @file
 * Contains functions only needed if the user has block editing permissions.
 */

/**
 * Add block editing variables into the block templates.
 *
 * @param $vars
 *   An array of variables to pass to the theme template.
 * @param $hook
 *   The name of the template being rendered ("block" in this case.)
 */
function zen_preprocess_block_editing(&$vars, $hook) {
  $block = $vars['block'];

  // Display 'edit block' for custom blocks.
  if ($block->module == 'block') {
    $vars['edit_links_array']['block-edit'] = l('<span>' . t('edit block') . '</span>', 'admin/build/block/configure/' . $block->module . '/' . $block->delta,
      array(
        'attributes' => array(
          'title' => t('edit the content of this block'),
          'class' => 'block-edit',
        ),
        'query' => drupal_get_destination(),
        'html' => TRUE,
      )
    );
  }
  // Display 'configure' for other blocks.
  else {
    $vars['edit_links_array']['block-config'] = l('<span>' . t('configure') . '</span>', 'admin/build/block/configure/' . $block->module . '/' . $block->delta,
      array(
        'attributes' => array(
          'title' => t('configure this block'),
          'class' => 'block-config',
        ),
        'query' => drupal_get_destination(),
        'html' => TRUE,
      )
    );
  }

  // Display 'edit view' for Views blocks.
  if ($block->module == 'views' && user_access('administer views')) {
    list($view_name, $view_block) = explode('-block', $block->delta);
    $vars['edit_links_array']['block-edit-view'] = l('<span>' . t('edit view') . '</span>', 'admin/build/views/edit/' . $view_name,
      array(
        'attributes' => array(
          'title' => t('edit the view that defines this block'),
          'class' => 'block-edit-view',
        ),
        'query' => drupal_get_destination(),
        'fragment' => 'views-tab-block' . $view_block,
        'html' => TRUE,
      )
    );
  }
  // Display 'edit menu' for Menu blocks.
  elseif (($block->module == 'menu' || ($block->module == 'user' && $block->delta == 1)) && user_access('administer menu')) {
    $menu_name = ($block->module == 'user') ? 'navigation' : $block->delta;
    $vars['edit_links_array']['block-edit-menu'] = l('<span>' . t('edit menu') . '</span>', 'admin/build/menu-customize/' . $menu_name,
      array(
        'attributes' => array(
          'title' => t('edit the menu that defines this block'),
          'class' => 'block-edit-menu',
        ),
        'query' => drupal_get_destination(),
        'html' => TRUE,
      )
    );
  }
  // Display 'edit menu' for Menu block blocks.
  elseif ($block->module == 'menu_block' && user_access('administer menu')) {
    list($menu_name, ) = split(':', variable_get("menu_block_{$block->delta}_parent", 'navigation:0'));
    $vars['edit_links_array']['block-edit-menu'] = l('<span>' . t('edit menu') . '</span>', 'admin/build/menu-customize/' . $menu_name,
      array(
        'attributes' => array(
          'title' => t('edit the menu that defines this block'),
          'class' => 'block-edit-menu',
        ),
        'query' => drupal_get_destination(),
        'html' => TRUE,
      )
    );
  }
}
