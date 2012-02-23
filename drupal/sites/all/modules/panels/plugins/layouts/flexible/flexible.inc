<?php

/**
 * Implementation of hook_panels_layouts()
 */
// Plugin definition
$plugin = array(
  'title' => t('Flexible'),
  'category' => t('Builders'),
  'icon' => 'flexible.png',
  'theme' => 'panels_flexible',
  'admin theme' => 'panels_flexible_admin',
  'css' => 'flexible.css',
  'admin css' => 'flexible-admin.css',
  'settings form' => 'panels_flexible_settings_form',
  'settings submit' => 'panels_flexible_settings_submit',
  'settings validate' => 'panels_flexible_settings_validate',
  'panels function' => 'panels_flexible_panels',
  'hook menu' => 'panels_flexible_menu',

  // Reuisable layout Builder specific directives
  'builder' => TRUE,
  'builder tab title' => 'Add flexible layout', // menu so translated elsewhere

  'get child' => 'panels_flexible_get_sublayout',
  'get children' => 'panels_flexible_get_sublayouts',

  // Define ajax callbacks
  'ajax' => array(
    'settings' => 'panels_ajax_flexible_edit_settings',
    'add' => 'panels_ajax_flexible_edit_add',
    'remove' => 'panels_ajax_flexible_edit_remove',
    'resize' => 'panels_ajax_flexible_edit_resize',
    'reuse' => 'panels_ajax_flexible_edit_reuse',
  ),
);

/**
 * Merge the main flexible plugin with a layout to create a sub plugin.
 *
 * This is used for both panels_flexible_get_sublayout and
 * panels_flexible_get_sublayouts.
 */
function panels_flexible_merge_plugin($plugin, $layout) {
  $plugin['name'] = 'flexible:' . $layout->name;
  $plugin['category'] = !empty($layout->category) ? check_plain($layout->category) : t('Miscellaneous');
  $plugin['title'] = check_plain($layout->admin_title);
  $plugin['description'] = check_plain($layout->admin_description);
  $plugin['layout'] = $layout;
  $plugin['builder'] = FALSE;
  $plugin['builder tab title'] = NULL;
  return $plugin;
}

/**
 * Callback to provide a single stored flexible layout.
 */
function panels_flexible_get_sublayout($plugin, $layout_name, $sublayout_name) {
  // Do not worry about caching; Panels is handling that for us.
  ctools_include('export');
  $item = ctools_export_crud_load('panels_layout', $sublayout_name);
  if ($item) {
    return panels_flexible_merge_plugin($plugin, $item);
  }
}

/**
 * Callback to provide all stored flexible layouts.
 */
function panels_flexible_get_sublayouts($plugin, $layout_name) {
  $layouts[$layout_name] = $plugin;
  ctools_include('export');
  $items = ctools_export_load_object('panels_layout', 'conditions', array('plugin' => 'flexible'));
  foreach ($items as $name => $item) {
    $layouts['flexible:' . $name] = panels_flexible_merge_plugin($plugin, $item);
  }

  return $layouts;
}

/**
 * Convert settings from old style to new, or provide defaults for
 * empty settings.
 * @param <type> $settings
 */
function panels_flexible_convert_settings(&$settings, &$layout) {
  // This indicates that this is a layout that they used the checkbox
  // on. The layout is still 'flexible' but it's actually pointing
  // to another stored one and we have to load it.
  if (!empty($settings['layout'])) {
    $layout = panels_get_layout('flexible:' . $settings['layout']);
  }

  if (!empty($layout['layout'])) {
    $settings = $layout['layout']->settings;
    if ($settings) {
      return $settings;
    }
  }

  if (empty($settings)) {
    // set up a default
    $settings = array(
      'items' => array(
        // The 'canvas' is a special row that does not get rendered
        // normally, but is used to contain the columns.
        'canvas' => array(
          'type' => 'row',
          'contains' => 'column',
          'children' => array('main'),
          'parent' => NULL,
        ),
        'main' => array(
          'type' => 'column',
          'width' => 100,
          'width_type' => '%',
          'children' => array('main-row'),
          'parent' => 'canvas',
        ),
        'main-row' => array(
          'type' => 'row',
          'contains' => 'region',
          'children' => array('center'),
          'parent' => 'main',
        ),
        'center' => array(
          'type' => 'region',
          'title' => t('Center'),
          'width' => 100,
          'width_type' => '%',
          'parent' => 'main-row',
        ),
      ),
    );
  }
  else if (!isset($settings['items'])) {
    // Convert an old style flexible to a new style flexible.
    $old = $settings;
    $settings = array();
    $settings['items']['canvas'] = array(
      'type' => 'row',
      'contains' => 'column',
      'children' => array(),
      'parent' => NULL,
    );
    // add the left sidebar column, row and region if it exists.
    if (!empty($old['sidebars']['left'])) {
      $settings['items']['canvas']['children'][] = 'sidebar-left';
      $settings['items']['sidebar-left'] = array(
        'type' => 'column',
        'width' => $old['sidebars']['left_width'],
        'width_type' => $old['sidebars']['width_type'],
        'children' => array('sidebar-left-row'),
        'parent' => 'canvas',
      );
      $settings['items']['sidebar-left-row'] = array(
        'type' => 'row',
        'contains' => 'region',
        'children' => array('sidebar_left'),
        'parent' => 'sidebar-left',
      );
      $settings['items']['sidebar_left'] = array(
        'type' => 'region',
        'title' => t('Left sidebar'),
        'width' => 100,
        'width_type' => '%',
        'parent' => 'sidebar-left-row',
      );
    }

    $settings['items']['canvas']['children'][] = 'main';

    if (!empty($old['sidebars']['right'])) {
      $settings['items']['canvas']['children'][] = 'sidebar-right';
      $settings['items']['sidebar-right'] = array(
        'type' => 'column',
        'width' => $old['sidebars']['right_width'],
        'width_type' => $old['sidebars']['width_type'],
        'children' => array('sidebar-right-row'),
        'parent' => 'canvas',
      );
      $settings['items']['sidebar-right-row'] = array(
        'type' => 'row',
        'contains' => 'region',
        'children' => array('sidebar_right'),
        'parent' => 'sidebar-right',
      );
      $settings['items']['sidebar_right'] = array(
        'type' => 'region',
        'title' => t('Right sidebar'),
        'width' => 100,
        'width_type' => '%',
        'parent' => 'sidebar-right-row',
      );
    }

    // Add the main column.
    $settings['items']['main'] = array(
      'type' => 'column',
      'width' => 100,
      'width_type' => '%',
      'children' => array(),
      'parent' => 'canvas',
    );

    // Add rows and regions.
    for ($row = 1; $row <= intval($old['rows']); $row++) {
      // Create entry for the row
      $settings['items']["row_$row"] = array(
        'type' => 'row',
        'contains' => 'region',
        'children' => array(),
        'parent' => 'main',
      );
      // Add the row to the parent's children
      $settings['items']['main']['children'][] = "row_$row";

      for ($col = 1; $col <= intval($old["row_$row"]['columns']); $col++) {
        // Create entry for the region
        $settings['items']["row_${row}_$col"] = array(
          'type' => 'region',
          'width' => $old["row_$row"]["width_$col"],
          'width_type' => '%',
          'parent' => "row_$row",
        );
        // Add entry for the region to the row's children
        $settings['items']["row_$row"]['children'][] = "row_${row}_$col";

        // Apply the proper title to the region
        if (!empty($old["row_$row"]['names'][$col - 1])) {
          $settings['items']["row_${row}_$col"]['title'] = $old["row_$row"]['names'][$col - 1];
        }
        else {
          $settings['items']["row_${row}_$col"]['title'] = t("Row @row, Column @col", array('@row' => $row, '@col' => $col));
        }
      }
    }
  }
  else if (isset($settings['canvas'])) {
    // Convert the old 'canvas' to the new canvas row.
    $settings['items']['canvas'] = array(
      'type' => 'row',
      'contains' => 'column',
      'children' => $settings['canvas'],
      'parent' => NULL,
    );
    unset($settings['canvas']);
  }
}

/**
 * Define the actual list of columns and rows for this flexible panel.
 */
function panels_flexible_panels($display, $settings, $layout) {
  $items = array();
  panels_flexible_convert_settings($settings, $layout);
  foreach ($settings['items'] as $id => $item) {
    if ($item['type'] == 'region') {
      $items[$id] = $item['title'];
    }
  }

  return $items;
}

/**
 * Create a renderer object.
 *
 * The renderer object contains data that is passed around from function
 * to function allowing us to render our CSS and HTML easily.
 *
 * @todo Convert the functions to methods and make this properly OO.
 */
function panels_flexible_create_renderer($admin, $id, $content, $settings, &$display, $layout, $handler) {
  $renderer = new stdClass;
  $renderer->settings = $settings;
  $renderer->content = $content;
  $renderer->css_id = $id;
  $renderer->did = &$display->did;
  if ($admin) {
    // always scale in admin mode.
    $renderer->scale_base = 99.0;
  }
  else {
    $renderer->scale_base = !empty($settings['items']['canvas']['no_scale']) ? 100.0 : 99.0;
  }
  $renderer->id_str = $id ? 'id="' . $id . '"' : '';
  $renderer->admin = $admin;
  $renderer->handler = $handler;

  // Set up basic classes for all of our components.
  $renderer->name                 = !empty($layout['layout']) ? $layout['layout']->name : $display->did;
  $renderer->base_class           = $renderer->name;
  $renderer->item_class['column'] = 'panels-flexible-column';
  $renderer->item_class['row']    = 'panels-flexible-row';
  $renderer->item_class['region'] = 'panels-flexible-region';
  $renderer->base['canvas']       = 'panels-flexible-' . $renderer->base_class;

  // Override these if selected from the UI and not in admin mode.
  if (!$admin) {
    if (!empty($settings['items']['canvas']['class'])) {
      $renderer->base_class = $settings['items']['canvas']['class'];
      $renderer->base['canvas'] = $renderer->base_class;
    }
    if (!empty($settings['items']['canvas']['column_class'])) {
      $renderer->item_class['column'] = $settings['items']['canvas']['column_class'];
    }
    if (!empty($settings['items']['canvas']['row_class'])) {
      $renderer->item_class['row'] = $settings['items']['canvas']['row_class'];
    }
    if (!empty($settings['items']['canvas']['region_class'])) {
      $renderer->item_class['region'] = $settings['items']['canvas']['region_class'];
    }
  }

  // Get the separation values out of the canvas settings.
  $renderer->column_separation = !empty($settings['items']['canvas']['column_separation']) ? $settings['items']['canvas']['column_separation'] : '0.5em';

  $renderer->region_separation = !empty($settings['items']['canvas']['region_separation']) ? $settings['items']['canvas']['region_separation'] : '0.5em';

  $renderer->row_separation = !empty($settings['items']['canvas']['row_separation']) ? $settings['items']['canvas']['row_separation'] : '0.5em';

  // Make some appended classes so it's easier to reference them.

  $renderer->base['column'] = $renderer->item_class['column'] . '-' . $renderer->base_class;
  $renderer->base['row']    = $renderer->item_class['row'] . '-' . $renderer->base_class;
  $renderer->base['region'] = $renderer->item_class['region'] . '-' . $renderer->base_class;

  if ($renderer->name != 'new') {
    // Use v2 to guarantee all CSS gets regenerated to account for changes in
    // how some divs will be rendered.
    $renderer->css_cache_name = 'flexiblev2:' . $renderer->name;
    if ($admin) {
      ctools_include('css');
      ctools_css_clear($renderer->css_cache_name);
    }
  }
  return $renderer;
}

/**
 * Draw the flexible layout.
 */
function theme_panels_flexible($id, $content, $settings, $display, $layout, $handler) {
  panels_flexible_convert_settings($settings, $layout);

  $renderer = panels_flexible_create_renderer(FALSE, $id, $content, $settings, $display, $layout, $handler);

  // CSS must be generated because it reports back left/middle/right
  // positions.
  $css = panels_flexible_render_css($renderer);

  if (!empty($renderer->css_cache_name) && empty($display->editing_layout)) {
    ctools_include('css');
    // Generate an id based upon rows + columns:
    $filename = ctools_css_retrieve($renderer->css_cache_name);
    if (!$filename) {
      $filename = ctools_css_store($renderer->css_cache_name, $css, FALSE);
    }

    // Give the CSS to the renderer to put where it wants.
    if ($handler) {
      $handler->add_css($filename, 'module', 'all', FALSE);
    }
    else {
      ctools_css_add_css($filename, 'module', 'all', FALSE);
    }
  }
  else {
    // If the id is 'new' we can't reliably cache the CSS in the filesystem
    // because the display does not truly exist, so we'll stick it in the
    // head tag. We also do this if we've been told we're in the layout
    // editor so that it always gets fresh CSS.
    drupal_set_html_head("<style type=\"text/css\">\n$css</style>\n");
  }

  // Also store the CSS on the display in case the live preview or something
  // needs it
  $display->add_css = $css;

  $output = "<div class=\"panel-flexible " . $renderer->base['canvas'] . " clear-block\" $renderer->id_str>\n";
  $output .= "<div class=\"panel-flexible-inside " . $renderer->base['canvas'] . "-inside\">\n";

  $output .= panels_flexible_render_items($renderer, $settings['items']['canvas']['children'], $renderer->base['canvas']);

  // Wrap the whole thing up nice and snug
  $output .= "</div>\n</div>\n";

  return $output;
}

/**
 * Draw the flexible layout.
 */
function theme_panels_flexible_admin($id, $content, $settings, $display, $layout, $handler) {
  // We never draw stored flexible layouts in admin mode; they must be edited
  // from the stored layout UI at that point.
  if (!empty($layout['layout'])) {
    return theme_panels_flexible($id, $content, $settings, $display, $layout, $handler);
  }

  panels_flexible_convert_settings($settings, $layout);
  $renderer = panels_flexible_create_renderer(TRUE, $id, $content, $settings, $display, $layout, $handler);

  $css = panels_flexible_render_css($renderer);

  // For the administrative view, add CSS directly to head.
  drupal_set_html_head("<style type=\"text/css\">\n$css</style>\n");

  if (empty($display->editing_layout)) {
    $output = '<input type="submit" id="panels-flexible-toggle-layout" value ="' .
      t('Show layout designer') . '">';
    if (user_access('administer panels layouts')) {
      $output .= '<input type="hidden" class="panels-flexible-reuse-layout-url" value="' . url($handler->get_url('layout', 'reuse'), array('absolute' => TRUE)) . '">';
      $output .= '<input type="submit" id="panels-flexible-reuse-layout" class="ctools-use-modal" value ="' .
        t('Reuse layout') . '">';
    }
    $output .= "<div class=\"panel-flexible " . $renderer->base['canvas'] . " clear-block panel-flexible-admin panel-flexible-no-edit-layout\" $renderer->id_str>\n";
  }
  else {
    $output = "<div class=\"panel-flexible " . $renderer->base['canvas'] . " clear-block panel-flexible-admin panel-flexible-edit-layout\" $renderer->id_str>\n";
  }
  $output .= "<div class=\"panel-flexible-inside " . $renderer->base['canvas'] . "-inside \">\n";

  $content = panels_flexible_render_items($renderer, $settings['items']['canvas']['children'], $renderer->base['row'] . '-canvas');
  $output .= panels_flexible_render_item($renderer, $settings['items']['canvas'], $content, 'canvas', 0, 0, TRUE);

  // Wrap the whole thing up nice and snug
  $output .= "</div>\n</div>\n";

  drupal_add_js($layout['path'] . '/flexible-admin.js');
  drupal_add_js(array('flexible' => array('resize' => url($handler->get_url('layout', 'resize'), array('absolute' => TRUE)))), 'setting');
  return $output;
}

/**
 * Render a piece of a flexible layout.
 */
function panels_flexible_render_items($renderer, $list, $owner_id) {
  $output = '';
  $groups = array('left' => '', 'middle' => '', 'right' => '');
  $max = count($list) - 1;
  $prev = NULL;

  foreach ($list as $position => $id) {
    $item = $renderer->settings['items'][$id];
    $location = isset($renderer->positions[$id]) ? $renderer->positions[$id] : 'middle';

    if ($renderer->admin && $item['type'] != 'row' && $prev ) {
      $groups[$location] .= panels_flexible_render_splitter($renderer, $prev, $id);
    }

    switch ($item['type']) {
      case 'column':
        $content = panels_flexible_render_items($renderer, $item['children'], $renderer->base['column'] . '-' . $id);
        $groups[$location] .= panels_flexible_render_item($renderer, $item, $content, $id, $position, $max);
        break;
      case 'row':
        $content = panels_flexible_render_items($renderer, $item['children'], $renderer->base['row'] . '-' . $id);
        $groups[$location] .= panels_flexible_render_item($renderer, $item, $content, $id, $position, $max, TRUE);
        break;
      case 'region':
        $content = isset($renderer->content[$id]) ? $renderer->content[$id] : "&nbsp;";
        $groups[$location] .= panels_flexible_render_item($renderer, $item, $content, $id, $position, $max);
        break;
    }

    // If all items are fixed then we have a special splitter on the right to
    // control the overall width.
    if (!empty($renderer->admin) && $max == $position && $location == 'left') {
      $groups[$location] .= panels_flexible_render_splitter($renderer, $id, NULL);
    }
    $prev = $id;
  }

  $group_count = count(array_filter($groups));

  // Render each group. We only render the group div if we're in admin mode
  // or if there are multiple groups.
  foreach ($groups as $position => $content) {
    if (!empty($content) || $renderer->admin) {
      if ($group_count > 1 || $renderer->admin) {
        $output .= '<div class="' . $owner_id . '-' . $position . '">' . $content . '</div>';
      }
      else {
        $output .= $content;
      }
    }
  }

  return $output;
}

/**
 * Render a column in the flexible layout.
 */
function panels_flexible_render_item($renderer, $item, $content, $id, $position, $max, $clear = FALSE) {

  // If we are rendering a row and there is just one row, we don't need to
  // render the row unless there is fixed_width content inside it.
  if (empty($renderer->admin) && $item['type'] == 'row' && $max == 0) {
    $fixed = FALSE;
    foreach ($item['children'] as $id) {
      if ($renderer->settings['items'][$id]['width_type'] != '%') {
        $fixed = TRUE;
        break;
      }
    }

    if (!$fixed) {
      return $content;
    }
  }

  // If we are rendering a column and there is just one column, we don't
  // need to render the column unless it has a fixed_width.
  if (empty($renderer->admin) && $item['type'] == 'column' && $max == 0 && $item['width_type'] == '%') {
    return $content;
  }

  $base = $renderer->item_class[$item['type']];
  $output = '<div class="' . $base . ' ' . $renderer->base[$item['type']] . '-' . $id;
  if ($position == 0) {
    $output .= ' ' . $base . '-first';
  }
  if ($position == $max) {
    $output .= ' ' . $base . '-last';
  }
  if ($clear) {
    $output .= ' clear-block';
  }

  if (isset($item['class'])) {
    $output .= ' ' . check_plain($item['class']);
  }

  $output .= '">' . "\n";

  if (!empty($renderer->admin)) {
    $output .= panels_flexible_render_item_links($renderer, $id, $item);
  }

  $output .= '  <div class="inside ' . $base . '-inside ' . $base . '-' . $renderer->base_class . '-' . $id . '-inside';
  if ($position == 0) {
    $output .= ' ' . $base . '-inside-first';
  }
  if ($position == $max) {
    $output .= ' ' . $base . '-inside-last';
  }
  if ($clear) {
    $output .= ' clear-block';
  }

  $output .= "\">\n";
  $output .= $content;
  $output .= '  </div>' . "\n";
  $output .= '</div>' . "\n";

  return $output;
}
/**
 * Render a splitter div to place between the $left and $right items.
 *
 * If the right ID is NULL that means there isn't actually a box to the
 * right, but we need a splitter anyway. We'll mostly use info about the
 * left, but pretend it's 'fluid' so that the javascript won't actually
 * modify the right item.
 */
function panels_flexible_render_splitter($renderer, $left_id, $right_id) {
  $left = $renderer->settings['items'][$left_id];

  $left_class = $renderer->base[$left['type']] . '-' . $left_id;
  if ($right_id) {
    $right = $renderer->settings['items'][$right_id];
    $right_class = $renderer->base[$left['type']] . '-' . $right_id;
  }
  else {
    $right = $left;
    $right_class = $left_class;
  }

  $output = '<div class="panels-flexible-splitter flexible-splitter-for-' . $left_class . '">';

  // Name the left object
  $output .= '<span class="panels-flexible-splitter-left">';
  $output .= '.' . $left_class;
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-left-id">';
  $output .= $left_id;
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-left-width ' . $left_class . '-width">';
  $output .= $left['width'];
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-left-scale">';
  $output .= isset($renderer->scale[$left_id]) ? $renderer->scale[$left_id] : 1;
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-left-width-type">';
  $output .= $left['width_type'];
  $output .= '</span>';

  // Name the right object
  $output .= '<span class="panels-flexible-splitter-right">';
  $output .= '.' . $right_class;
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-right-id">';
  $output .= $right_id;
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-right-width ' . $right_class . '-width">';
  $output .= $right['width'];
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-right-scale">';
  $output .= isset($renderer->scale[$right_id]) ? $renderer->scale[$right_id] : 1;
  $output .= '</span>';

  $output .= '<span class="panels-flexible-splitter-right-width-type">';
  // If there is no right, make it fluid.
  $output .= $right_id ? $right['width_type'] : '%';
  $output .= '</span>';

  $output .= '</div>';
  return $output;
}

/**
 * Render the dropdown links for an item.
 */
function panels_flexible_render_item_links($renderer, $id, $item) {
  $links = array();
  $remove = '';
  $add = '';
  if ($item['type'] == 'column') {
    $title = t('Column');
    $settings = t('Column settings');
    if (empty($item['children'])) {
      $remove = t('Remove column');
      $add = t('Add row');
    }
    else {
      $add = t('Add row to top');
      $add2 = t('Add row to bottom');
    }
  }
  else if ($item['type'] == 'row') {
    if ($id == 'canvas') {
      $title = t('Canvas');
      $settings = t('Canvas settings');
    }
    else {
      $title = t('Row');
      $settings = t('Row settings');
    }
    if (empty($item['children'])) {
      if ($id != 'canvas') {
        $remove = t('Remove row');
      }
      $add = $item['contains'] == 'region' ? t('Add region') : t('Add column');
    }
    else {
      $add = $item['contains'] == 'region' ? t('Add region to left') : t('Add column to left');
      $add2 = $item['contains'] == 'region' ? t('Add region to right') : t('Add column to right');
    }
  }
  else if ($item['type'] == 'region') {
    $title = t('Region');
    $settings = t('Region settings');
    $remove = t('Remove region');
  }

  if (!empty($settings)) {
    $links[] = array(
      'title' => $settings,
      'href' => $renderer->handler->get_url('layout', 'settings', $id),
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }
  if ($add) {
    $links[] = array(
      'title' => $add,
      'href' => $renderer->handler->get_url('layout', 'add', $id),
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }
  if (isset($add2)) {
    $links[] = array(
      'title' => $add2,
      'href' => $renderer->handler->get_url('layout', 'add', $id, 'right'),
      'attributes' => array('class' => 'ctools-use-modal'),
    );
  }
  if ($remove) {
    $links[] = array(
      'title' => $remove,
      'href' => $renderer->handler->get_url('layout', 'remove', $id),
      'attributes' => array('class' => 'ctools-use-ajax'),
    );
  }

  return theme('ctools_dropdown', $title, $links, FALSE,
      'flexible-layout-only flexible-links flexible-title flexible-links-' . $id);
}
/**
 * Provide CSS for a flexible layout.
 */
function panels_flexible_render_css($renderer) {
  if ($renderer->admin) {
    $parent_class = '.' . $renderer->base['row'] . '-canvas';
  }
  else {
    $parent_class = '.' . $renderer->base['canvas'];
  }
  return panels_flexible_render_css_group($renderer, $renderer->settings['items']['canvas']['children'], $parent_class, 'column', 'canvas');
}

/**
 * Render the CSS for a group of items to be displayed together.
 *
 * Columns and regions, when displayed as a group, need to cooperate in
 * order to share margins and make sure that percent widths add up
 * to the right total.
 */
function panels_flexible_render_css_group($renderer, $list, $owner_id, $type, $id) {
  $css = array();

  // Start off with some generic CSS to properly pad regions
  $css['.' . $renderer->item_class['region']] = array(
    'padding' => '0',
  );

  $css['.' . $renderer->item_class['region'] . '-inside'] = array(
    'padding-right' => $renderer->region_separation,
    'padding-left' => $renderer->region_separation,
  );

  $css['.' . $renderer->item_class['region'] . '-inside-first'] = array(
    'padding-left' => '0',
  );

  $css['.' . $renderer->item_class['region'] . '-inside-last'] = array(
    'padding-right' => '0',
  );

  $css['.' . $renderer->item_class['column']] = array(
    'padding' => '0',
  );

  $css['.' . $renderer->item_class['column'] . '-inside'] = array(
    'padding-right' => $renderer->column_separation,
    'padding-left' => $renderer->column_separation,
  );

  $css['.' . $renderer->item_class['column'] . '-inside-first'] = array(
    'padding-left' => '0',
  );

  $css['.' . $renderer->item_class['column'] . '-inside-last'] = array(
    'padding-right' => '0',
  );

  // And properly pad rows too
  $css['.' . $renderer->item_class['row']] = array(
    'padding' => '0 0 ' . $renderer->row_separation . ' 0',
    'margin' => '0',
  );

  $css['.' . $renderer->item_class['row'] . '-last'] = array(
    'padding-bottom' => '0',
  );

  panels_flexible_get_css_group($css, $renderer, $list, $owner_id, $type, $id);

  ctools_include('css');
  return ctools_css_assemble($css);
}

/**
 * Construct an array with all of the CSS properties for a group.
 *
 * This will parse down into children and produce all of the CSS needed if you
 * start from the top.
 */
function panels_flexible_get_css_group(&$css, $renderer, $list, $owner_id, $type, $item_id) {
  if ($type != 'row') {
    // Go through our items and break up into right/center/right groups so we
    // can figure out our offsets.

    // right == any items on the right that are 'fixed'.
    // middle == all fluid items.
    // right == any items on the right that are 'fixed'.
    $left = $middle = $right = array();
    $left_total = $right_total = $middle_total = 0;
    $current = 'left';
    foreach ($list as $id) {
      if ($renderer->settings['items'][$id]['width_type'] == 'px') {
        // fixed
        if ($current == 'left') {
          $left[] = $id;
          $renderer->positions[$id] = 'left';
          $left_total += $renderer->settings['items'][$id]['width'];
        }
        else {
          $current = 'right';
          $right[] = $id;
          $renderer->positions[$id] = 'right';
          $right_total += $renderer->settings['items'][$id]['width'];
        }
      }
      else {
        // fluid
        if ($current != 'right') {
          $current = 'middle';
          $middle[] = $id;
          $renderer->positions[$id] = 'middle';
          $middle_total += $renderer->settings['items'][$id]['width'];
        }
        // fall through: if current is 'right' and we ran into a 'fluid' then
        // it gets *dropped* because that is invalid.
      }
    }

    // Go through our right sides and create CSS.
    foreach ($left as $id) {
      $class = "." . $renderer->base[$type] . "-$id";
      $css[$class] = array(
        'position' => 'relative',
        'float' => 'left',
        'background-color' => 'transparent',
        'width' => $renderer->settings['items'][$id]['width'] . "px",
      );
    }

    // Do the same for right.
    $right_pixels = 0;

    foreach ($right as $id) {
      $class = "." . $renderer->base[$type] . "-$id";
      $css[$class] = array(
        'float' => 'left',
        'width' => $renderer->settings['items'][$id]['width'] . "px",
      );
    }

    $max = count($middle) - 1;

    if ($middle_total) {
      // Because we love IE so much, auto scale everything to 99%. This
      // means adding up the actual widths and then providing a multiplier
      // to each so that the total is 99%.
      $scale = $renderer->scale_base / $middle_total;
      foreach ($middle as $position => $id) {
        $class = "." . $renderer->base[$type] . "-$id";
        $css[$class] = array(
          'float' => 'left',
          'width' => number_format($renderer->settings['items'][$id]['width'] * $scale, 4, '.', '') . "%",
        );

        // Store this so we can use it later.
        // @todo: Store the scale, not the new width, so .js can adjust
        // bi-directionally.
        $renderer->scale[$id] = $scale;
      }
    }

    // If there is any total remaining, we need to offset the splitter
    // by this much too.
    if ($left_total) {
      // Add this even if it's 0 so we can handle removals.
      $css["$owner_id-inside"]['padding-left'] = '0px';
      if ($renderer->admin || count($middle)) {
        $css["$owner_id-middle"]['margin-left'] = $left_total . 'px';
        // IE hack
        $css["* html $owner_id-left"]['left'] = $left_total . "px";
        // Make this one very specific to the admin CSS so that preview
        // does not stomp it.
        $css[".panel-flexible-admin $owner_id-inside"]['padding-left'] = '0px';
      }
      else {
        $css["$owner_id-inside"]['margin-left'] = '-' . $left_total . 'px';
        $css["$owner_id-inside"]['padding-left'] = $left_total . 'px';
        // IE hack
        $css["* html $owner_id-inside"]['left'] = $left_total . "px";
      }
    }
    if ($right_total) {
      $css["$owner_id-middle"]['margin-right'] = $right_total . 'px';
    }
    $css["$owner_id-inside"]['padding-right'] = '0px';
  }

  // If the canvas has a fixed width set, and this is the canvas, fix the
  // width.
  if ($item_id == 'canvas') {
    $item = $renderer->settings['items'][$item_id];

    if (!empty($item['fixed_width']) && intval($item['fixed_width'])) {
      $css['.' . $renderer->base['canvas']]['width'] = intval($item['fixed_width']) . 'px';
    }
    else {
      $css['.' . $renderer->base['canvas']]['width'] = 'auto';
    }
  }

  // Go through each item and process children.
  foreach ($list as $id) {
    $item = $renderer->settings['items'][$id];
    if (empty($item['children'])) {
      continue;
    }

    if ($type == 'column') {
      // Columns can only contain rows.
      $child_type = 'row';
    }
    else {
      $child_type = isset($item['contains']) ? $item['contains'] : 'region';
    }

    $class = "." . $renderer->base[$type] . "-$id";
    panels_flexible_get_css_group($css, $renderer, $item['children'], $class, $child_type, $id);
  }
}

/**
 * AJAX responder to edit flexible settings for an item.
 *
 * $handler object
 *   The display renderer handler object.
 */
function panels_ajax_flexible_edit_settings($handler, $id) {
  $settings = &$handler->display->layout_settings;
  panels_flexible_convert_settings($settings, $handler->plugins['layout']);

  if (empty($settings['items'][$id])) {
    ctools_modal_render(t('Error'), t('Invalid item id.'));
  }

  $item = &$settings['items'][$id];
  $siblings = array();

  if ($id != 'canvas') {
    $siblings = $settings['items'][$item['parent']]['children'];
  }


  switch ($item['type']) {
    case 'column':
      $title = t('Configure column');
      break;
    case 'row':
      if ($id == 'canvas') {
        $title = t('Configure canvas');
      }
      else {
        $title = t('Configure row');
      }
      break;
    case 'region':
      $title = t('Configure region');
      break;
  }

  $form_state = array(
    'display' => &$handler->display,
    'item' => &$item,
    'id' => $id,
    'siblings' => $siblings,
    'settings' => &$settings,
    'ajax' => TRUE,
    'title' => $title,
    'op' => 'edit',
  );

  $output = ctools_modal_form_wrapper('panels_flexible_config_item_form', $form_state);
  if (empty($output)) {
    // If the width type changed then other nearby items will have
    // to have their widths adjusted.
    panels_edit_cache_set($handler->cache);

    $css_id = isset($handler->display->css_id) ? $handler->display->css_id : '';
    $renderer = panels_flexible_create_renderer(TRUE, $css_id, array(), $settings, $handler->display, $handler->plugins['layout'], $handler);

    $output = array();
    // If the item is a region, replace the title.
    $class = $renderer->base[$item['type']] . '-' . $id;
    if ($item['type'] == 'region') {
      $output[] = ctools_ajax_command_replace(".$class h2.label",
        '<h2 class="label">' . check_plain($item['title']) . '</h2>');
    }

    // Rerender our links in case something changed.
    $output[] = ctools_ajax_command_replace('.flexible-links-' . $id,
      panels_flexible_render_item_links($renderer, $id, $item));

    // If editing the canvas, reset the CSS width
    if ($id == 'canvas') {
      // update canvas CSS.
      $css = array(
        '.' . $renderer->item_class['column'] . '-inside' => array(
          'padding-left' => $renderer->column_separation,
          'padding-right' => $renderer->column_separation,
        ),
        '.' . $renderer->item_class['region'] . '-inside' => array(
          'padding-left' => $renderer->region_separation,
          'padding-right' => $renderer->region_separation,
        ),
        '.' . $renderer->item_class['row'] => array(
          'padding-bottom' => $renderer->row_separation,
        ),
      );
      if (!empty($item['fixed_width']) && intval($item['fixed_width'])) {
        $css['.' . $renderer->base['canvas']] = array('width' => intval($item['fixed_width']) . 'px');
      }
      else {
        $css['.' . $renderer->base['canvas']] = array('width' => 'auto');
      }
      foreach ($css as $selector => $data) {
        $output[] = ctools_ajax_command_css($selector, $data);
      }
    }

    $output[] = ctools_modal_command_dismiss();
  }

  $handler->commands = $output;
}

/**
 * Configure a row, column or region on the flexible page.
 *
 * @param <type> $form_state
 * @return <type>
 */
function panels_flexible_config_item_form(&$form_state) {
  $display = &$form_state['display'];
  $item = &$form_state['item'];
  $siblings = &$form_state['siblings'];
  $settings = &$form_state['settings'];
  $id = &$form_state['id'];

  if ($item['type'] == 'region') {
    $form['title'] = array(
      '#title' => t('Region title'),
      '#type' => 'textfield',
      '#default_value' => $item['title'],
      '#required' => TRUE,
    );
  }

  if ($id == 'canvas') {
    $form['class'] = array(
      '#title' => t('Canvas class'),
      '#type' => 'textfield',
      '#default_value' => isset($item['class']) ? $item['class'] : '',
      '#description' => t('This class will the primary class for this layout. It will also be appended to all column, row and region_classes to ensure that layouts within layouts will not inherit CSS from each other. If left blank, the name of the layout or ID of the display will be used.'),
    );

    $form['column_class'] = array(
      '#title' => t('Column class'),
      '#type' => 'textfield',
      '#default_value' => isset($item['column_class']) ? $item['column_class'] : '',
      '#description' => t('This class will be applied to all columns of the layout. If left blank this will be panels-flexible-column.'),
    );

    $form['row_class'] = array(
      '#title' => t('Row class'),
      '#type' => 'textfield',
      '#default_value' => isset($item['row_class']) ? $item['row_class'] : '',
      '#description' => t('This class will be applied to all rows of the layout. If left blank this will be panels-flexible-row.'),
    );

    $form['region_class'] = array(
      '#title' => t('Region class'),
      '#type' => 'textfield',
      '#default_value' => isset($item['region_class']) ? $item['region_class'] : '',
      '#description' => t('This class will be applied to all regions of the layout. If left blank this will be panels-flexible-region.'),
    );

    $form['no_scale'] = array(
      '#type' => 'checkbox',
      '#title' => t('Scale fluid widths for IE6'),
      '#description' => t('IE6 does not do well with 100% widths. If checked, width will be scaled to 99% to compensate.'),
      '#default_value' => empty($item['no_scale']),
    );

    $form['fixed_width'] = array(
      '#type' => 'textfield',
      '#title' => t('Fixed width'),
      '#description' => t('If a value is entered, the layout canvas will be fixed to the given pixel width.'),
      '#default_value' => isset($item['fixed_width']) ? $item['fixed_width'] : '',
    );

    $form['column_separation'] = array(
      '#type' => 'textfield',
      '#title' => t('Column separation'),
      '#description' => t('How much padding to put on columns that are that are next to other columns. Note that this is put on both columns so the real amount is doubled.'),
      '#default_value' => isset($item['column_separation']) ? $item['column_separation'] : '0.5em',
    );

    $form['region_separation'] = array(
      '#type' => 'textfield',
      '#title' => t('Region separation'),
      '#description' => t('How much padding to put on regions that are that are next to other regions. Note that this is put on both regions so the real amount is doubled.'),
      '#default_value' => isset($item['region_separation']) ? $item['region_separation'] : '0.5em',
    );

    $form['row_separation'] = array(
      '#type' => 'textfield',
      '#title' => t('Row separation'),
      '#description' => t('How much padding to put on beneath rows to separate them from each other. Because this is placed only on the bottom, not hte top, this is NOT doubled like column/region separation.'),
      '#default_value' => isset($item['row_separation']) ? $item['row_separation'] : '0.5em',
    );
  }
  else {
    $form['class'] = array(
      '#title' => t('CSS class'),
      '#type' => 'textfield',
      '#default_value' => isset($item['class']) ? $item['class'] : '',
      '#description' => t('Enter a CSS class that will be used. This can be used to apply automatic styling from your theme, for example.'),
    );

    if ($item['type'] != 'row') {
      // Test to see if there are fluid items to the left or the right. If there
      // are fluid items on both sides, this item cannot be set to fixed.
      $left = $right = FALSE;
      $current = 'left';
      foreach ($siblings as $sibling) {
        if ($sibling == $id) {
          $current = 'right';
        }
        else if ($settings['items'][$sibling]['width_type'] == '%') {
          $$current = TRUE; // Indirection.
        }
      }

      $form['width_type'] = array(
        '#type' => 'select',
        '#title' => t('Width'),
        '#default_value' => $item['width_type'],
        '#options' => array(
          '%' => t('Fluid'),
          'px' => t('Fixed'),
        ),
        '#disabled' => TRUE,
      );
    }
    else {
      $form['contains'] = array(
        '#type' => 'select',
        '#title' => t('Contains'),
        '#default_value' => $item['contains'],
        '#options' => array(
          'region' => t('Regions'),
          'column' => t('Columns'),
        ),
      );

      if (!empty($item['children'])) {
        $form['contains']['#disabled'] = TRUE;
        $form['contains']['#value'] = $item['contains'];
        $form['contains']['#description'] = t('You must remove contained items to change the row container type.');
      }
    }
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Submit handler for editing a flexible item.
 */
function panels_flexible_config_item_form_submit(&$form, &$form_state) {
  $item = &$form_state['item'];
  if ($item['type'] == 'region') {
    $item['title'] = $form_state['values']['title'];
  }

  $item['class'] = $form_state['values']['class'];

  if ($form_state['id'] == 'canvas') {
    $item['column_class'] = $form_state['values']['column_class'];
    $item['row_class'] = $form_state['values']['row_class'];
    $item['region_class'] = $form_state['values']['region_class'];
    // Reverse this as the checkbox is backward from how we actually store
    // it to make it simpler to default to scaling.
    $item['no_scale'] = !$form_state['values']['no_scale'];
    $item['fixed_width'] = $form_state['values']['fixed_width'];
    $item['column_separation'] = $form_state['values']['column_separation'];
    $item['region_separation'] = $form_state['values']['region_separation'];
    $item['row_separation'] = $form_state['values']['row_separation'];
  }
  else if ($item['type'] != 'row') {
    $item['width_type'] = $form_state['values']['width_type'];
  }
  else {
    $item['contains'] = $form_state['values']['contains'];
  }

}

/**
 * AJAX responder to add a new row, column or region to a flexible layout.
 */
function panels_ajax_flexible_edit_add($handler, $id, $location = 'left') {
  ctools_include('modal');
  ctools_include('ajax');
  $settings = &$handler->display->layout_settings;
  panels_flexible_convert_settings($settings, $handler->plugins['layout']);

  if (empty($settings['items'][$id])) {
    ctools_modal_render(t('Error'), t('Invalid item id.'));
  }

  $parent = &$settings['items'][$id];

  switch ($parent['type']) {
    case 'column':
      $title = t('Add row');
      // Create the new item with defaults.
      $item = array(
        'type' => 'row',
        'contains' => 'region',
        'children' => array(),
        'parent' => $id,
      );
      break;
    case 'row':
      switch ($parent['contains']) {
        case 'region':
          $title = $location == 'left' ? t('Add region to left') : t('Add region to right');
          $item = array(
            'type' => 'region',
            'title' => '',
            'width' => 100,
            'width_type' => '%',
            'parent' => $id,
          );
          break;
        case 'column':
          $title = $location == 'left' ? t('Add column to left') : t('Add column to right');
          $item = array(
            'type' => 'column',
            'width' => 100,
            'width_type' => '%',
            'parent' => $id,
            'children' => array(),
          );
          break;
      }
      // Create the new item with defaults.
      break;
    case 'region':
      // Cannot add items to regions.
      break;
  }

  $form_state = array(
    'display' => &$handler->display,
    'parent' => &$parent,
    'item' => &$item,
    'id' => $id,
    'settings' => &$settings,
    'ajax' => TRUE,
    'title' => $title,
    'location' => $location,
  );

  $output = ctools_modal_form_wrapper('panels_flexible_add_item_form', $form_state);
  if (empty($output)) {
    // If the width type changed then other nearby items will have
    // to have their widths adjusted.
    panels_edit_cache_set($handler->cache);
    $output = array();

    $css_id = isset($handler->display->css_id) ? $handler->display->css_id : '';
    // Create a renderer object so we can render our new stuff.
    $renderer = panels_flexible_create_renderer(TRUE, $css_id, array(), $settings, $handler->display, $handler->plugins['layout'], $handler);

    $content = '';
    if ($item['type'] == 'region') {
      $handler->plugins['layout']['panels'][$form_state['key']] = $item['title'];

      $content = $handler->render_region($form_state['key'], array());

      // Manually add the hidden field that our region uses to store pane info.
      $content .= '<input type="hidden" name="panel[pane][' .
        $form_state['key'] . ']" id="edit-panel-pane-' . $form_state['key'] . '" value="" />';

    }
    else {
      // We need to make sure the left/middle/right divs exist inside this
      // so that more stuff can be added inside it as needed.
      foreach (array('left', 'middle', 'right') as $position) {
        if (!empty($content) || $renderer->admin) {
          $content .= '<div class="' . $renderer->base[$item['type']] . '-' . $form_state['key'] . '-' . $position . '"></div>';
        }
      }

    }

    // render the item
    $parent_class =  $renderer->base[$parent['type']] . '-' . $id;
    $item_output = panels_flexible_render_item($renderer, $item, $content, $form_state['key'], 0, 0, $item['type'] == 'row');

    // Get all the CSS necessary for the entire row (as width adjustments may
    // have cascaded).
    $css = array();
    panels_flexible_get_css_group($css, $renderer, $parent['children'], '.' . $parent_class, $item['type'], $id);

    $position = isset($renderer->positions[$form_state['key']]) ? $renderer->positions[$form_state['key']] : 'middle';
    // If there's a nearby item, add the splitter and rewrite the width
    // of the nearby item as it probably got adjusted.
    // The blocks of code in this else look very similar but are not actually
    // duplicated because the order changes based on left or right.
    switch ($position) {
      case 'left':
        if ($location == 'left') {
          $item_output .= panels_flexible_render_splitter($renderer, $form_state['key'], $form_state['sibling']);
          $output[] = ctools_ajax_command_prepend('#panels-dnd-main .' . $parent_class . '-left', $item_output);
        }
        else if ($location == 'right') {
          // If we are adding to the right side of the left box, there is
          // a splitter that we have to remove; then we add our box normally,
          // and then add a new splitter for just our guy.
          $output[] = ctools_ajax_command_remove('panels-flexible-splitter-for-' . $renderer->base[$item['type']] . '-' . $form_state['key']);
          $item_output = panels_flexible_render_splitter($renderer, $form_state['sibling'], $form_state['key']) .  $item_output;
          $item_output .= panels_flexible_render_splitter($renderer, $form_state['key'], NULL);
          $output[] = ctools_ajax_command_append('#panels-dnd-main .' . $parent_class . '-left', $item_output);
        }
        break;
      case 'right':
        if (!empty($form_state['sibling'])) {
          $item_output = panels_flexible_render_splitter($renderer, $form_state['sibling'], $form_state['key']) .  $item_output;
        }
        $output[] = ctools_ajax_command_append('#panels-dnd-main .' . $parent_class . '-right', $item_output);
        break;
      case 'middle':
        if ($location == 'left') {
          if (!empty($form_state['sibling'])) {
            $item_output .= panels_flexible_render_splitter($renderer, $form_state['key'], $form_state['sibling']);
          }
          $output[] = ctools_ajax_command_prepend('#panels-dnd-main .' . $parent_class . '-middle', $item_output);
        }
        else {
          if (!empty($form_state['sibling'])) {
            $item_output = panels_flexible_render_splitter($renderer, $form_state['sibling'], $form_state['key']) .  $item_output;
          }
          $output[] = ctools_ajax_command_append('#panels-dnd-main .' . $parent_class . '-middle', $item_output);
        }
        break;

    }

    // Send our fix height command.
    $output[] = array('command' => 'flexible_fix_height');

    if (!empty($form_state['sibling'])) {
      $sibling_width = '#panels-dnd-main .' . $renderer->base[$item['type']] . '-' . $form_state['sibling'] . '-width';
      $output[] = ctools_ajax_command_html($sibling_width, $settings['items'][$form_state['sibling']]['width']);
    }
    foreach ($css as $selector => $data) {
      $output[] = ctools_ajax_command_css($selector, $data);
    }

    // Rerender our parent item links:
    $output[] = ctools_ajax_command_replace('.flexible-links-' . $id,
      panels_flexible_render_item_links($renderer, $id, $parent));

    $output[] = array(
      'command' => 'flexible_fix_firstlast',
      'selector' => '.' . $parent_class . '-inside',
      'base' => 'panels-flexible-' . $item['type'],
    );

    $output[] = ctools_modal_command_dismiss();
  }

  $handler->commands = $output;
}
/**
 * Form to add a row, column or region to a flexible layout.
 * @param <type> $form_state
 * @return <type>
 */
function panels_flexible_add_item_form(&$form_state) {
  $display = &$form_state['display'];
  $item = &$form_state['item'];
  $parent = &$form_state['parent'];
  $settings = &$form_state['settings'];
  $location = &$form_state['location'];
  $id = &$form_state['id'];

  if ($item['type'] == 'region') {
    $form['title'] = array(
      '#title' => t('Region title'),
      '#type' => 'textfield',
      '#default_value' => $item['title'],
      '#required' => TRUE,
    );
  }

  $form['class'] = array(
    '#title' => t('CSS Class'),
    '#type' => 'textfield',
    '#default_value' => isset($item['class']) ? $item['class'] : '',
    '#description' => t('Enter a CSS class that will be used. This can be used to apply automatic styling from your theme, for example.'),
  );

  if ($item['type'] != 'row') {
    // If there is a 'fixed' type on the side we're adding to, then this
    // must also be fixed. Otherwise it can be either and should default to
    // fluid.
    $restrict = FALSE;

    if (!empty($parent['children'])) {
      if ($location == 'left') {
        $sibling = reset($parent['children']);
      }
      else {
        $sibling = end($parent['children']);
      }
      if ($settings['items'][$sibling]['width_type'] == 'px') {
        $restrict = TRUE;
        $item['width_type'] = 'px';
      }
    }

    $form['width_type'] = array(
      '#type' => 'select',
      '#title' => t('Width'),
      '#default_value' => $item['width_type'],
      '#options' => array(
        '%' => t('Fluid'),
        'px' => t('Fixed'),
      ),
      '#disabled' => $restrict,
    );
    if ($restrict) {
      // This forces the value because disabled items don't always send
      // their data back.
      $form['width_type']['#value'] = $item['width_type'];
      $form['width_type']['#description'] = t('Items cannot be set to fluid if there are fixed items already on that side.');
    }
  }
  else {
    $form['contains'] = array(
      '#type' => 'select',
      '#title' => t('Contains'),
      '#default_value' => $item['contains'],
      '#options' => array(
        'region' => t('Regions'),
        'column' => t('Columns'),
      ),
    );
  }

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Submit handler for editing a flexible item.
 */
function panels_flexible_add_item_form_submit(&$form, &$form_state) {
  $item = &$form_state['item'];
  $parent = &$form_state['parent'];
  $location = &$form_state['location'];
  $settings = &$form_state['settings'];

  $item['class'] = $form_state['values']['class'];

  if ($item['type'] == 'region') {
    $item['title'] = $form_state['values']['title'];
  }

  if ($item['type'] != 'row') {
    $item['width_type'] = $form_state['values']['width_type'];
  }
  else {
    $item['contains'] = $form_state['values']['contains'];
  }

  if ($item['type'] == 'region') {
    // derive the region key from the title
    $key = preg_replace("/[^a-z0-9]/", '_', drupal_strtolower($item['title']));
    while (isset($settings['items'][$key])) {
      $key .= '_';
    }
    $form_state['key'] = $key;
  }
  else {
    $form_state['key'] = $key = max(array_keys($settings['items'])) + 1;
  }

  $form_state['sibling'] = NULL;
  if ($item['type'] != 'row' && !empty($parent['children'])) {
    // Figure out what the width should be and adjust our sibling if
    // necessary.
    if ($location == 'left') {
      $form_state['sibling'] = reset($parent['children']);
    }
    else {
      $form_state['sibling'] = end($parent['children']);

    }

    // If there is no sibling, or the sibling is of a different type,
    // the default 100 will work for either fixed or fluid.
    if ($form_state['sibling'] && $settings['items'][$form_state['sibling']]['width_type'] == $item['width_type']) {
      // steal half of the sibling's space.
      $width = $settings['items'][$form_state['sibling']]['width'] / 2;
      $settings['items'][$form_state['sibling']]['width'] = $width;
      $item['width'] = $width;
    }
  }

  // Place the item.
  $settings['items'][$key] = $item;
  if ($location == 'left') {
    array_unshift($parent['children'], $key);
  }
  else {
    $parent['children'][] = $key;
  }
}

/**
 * AJAX responder to remove an existing row, column or region from a flexible
 * layout.
 */
function panels_ajax_flexible_edit_remove($handler, $id) {
  $settings = &$handler->display->layout_settings;
  panels_flexible_convert_settings($settings, $handler->plugins['layout']);

  if (empty($settings['items'][$id])) {
    ctools_ajax_render_error(t('Invalid item id.'));
  }

  $item = &$settings['items'][$id];
  $css_id = isset($handler->display->css_id) ? $handler->display->css_id : '';
  // Create a renderer object so we can render our new stuff.
  $renderer = panels_flexible_create_renderer(TRUE, $css_id, array(), $settings, $handler->display, $handler->plugins['layout'], $handler);


  $siblings = &$settings['items'][$item['parent']]['children'];
  $parent_class = '.'  . $renderer->base[$settings['items'][$item['parent']]['type']] . '-' . $item['parent'];

  // Find the offset of our array. This will also be the key because
  // this is a simple array.
  $offset = array_search($id, $siblings);

  // Only bother with this stuff if our item is fluid, since fixed is
  // as fixed does.
  if ($item['type'] != 'row') {
    if (isset($siblings[$offset + 1])) {
      $next = $siblings[$offset + 1];
    }
    if (isset($siblings[$offset - 1])) {
      $prev = $siblings[$offset - 1];
    }

    if ($item['width_type'] == '%') {
      // First, try next.
      if (isset($next) && $settings['items'][$next]['width_type'] == '%') {
        $settings['items'][$next]['width'] += $item['width'];
      }
      // If that failed, try the previous one.
      else if (isset($prev) && $settings['items'][$prev]['width_type'] == '%') {
        $settings['items'][$prev]['width'] += $item['width'];
      }
    }
    // Not sure what happens if they both failed. Maybe nothing.
  }

  // Remove the item.
  array_splice($siblings, $offset, 1);

  unset($settings['items'][$id]);

  // Save our new state.
  panels_edit_cache_set($handler->cache);
  $class = $renderer->base[$item['type']] . '-' . $id;
  $output = array();

  $output[] = ctools_ajax_command_remove('#panels-dnd-main .' . $class);

  // Regenerate the CSS for siblings.
  if (!empty($siblings)) {
    // Get all the CSS necessary for the entire row (as width adjustments may
    // have cascaded).
    $css = array();
    panels_flexible_get_css_group($css, $renderer, $siblings, $parent_class, $item['type'], $item['parent']);
    foreach ($css as $selector => $data) {
      $output[] = ctools_ajax_command_css($selector, $data);
    }
  }

  // There are potentially two splitters linked to this item to be removed.
  if (!empty($prev)) {
    $output[] = ctools_ajax_command_remove('.flexible-splitter-for-' . $renderer->base[$item['type']] . '-' . $prev);
  }

  // Try to remove the 'next' one even if there isn't a $next.
  $output[] = ctools_ajax_command_remove('.flexible-splitter-for-' . $renderer->base[$item['type']] . '-' . $id);

  if (!empty($prev) && !empty($next)) {
    // Add a new splitter that links $prev and $next:
    $splitter = panels_flexible_render_splitter($renderer, $prev, $next);
    $prev_class = '#panels-dnd-main .' . $renderer->base[$item['type']] . '-' . $prev;
    $output[] = ctools_ajax_command_after($prev_class, $splitter);
    // Send our fix height command.
    $output[] = array('command' => 'flexible_fix_height');
  }
  // Rerender our parent item links:
  $output[] = ctools_ajax_command_replace('.flexible-links-' . $item['parent'],
    panels_flexible_render_item_links($renderer, $item['parent'], $settings['items'][$item['parent']]));

  $output[] = array(
    'command' => 'flexible_fix_firstlast',
    'selector' => $parent_class . '-inside',
    'base' => 'panels-flexible-' . $item['type'],
  );

  $handler->commands = $output;
}

/**
 * AJAX responder to store resize information when the user adjusts the
 * splitter.
 */
function panels_ajax_flexible_edit_resize($handler) {
  ctools_include('ajax');
  $settings = &$handler->display->layout_settings;
  panels_flexible_convert_settings($settings, $handler->plugins['layout']);

  $settings['items'][$_POST['left']]['width'] = $_POST['left_width'];
  if (!empty($_POST['right']) && $_POST['right'] != $_POST['left']) {
    $settings['items'][$_POST['right']]['width'] = $_POST['right_width'];
  }

  // Save our new state.
  panels_edit_cache_set($handler->cache);

  $handler->commands = array('ok');
}

/**
 * AJAX form to bring up the "reuse" modal.
 */
function panels_ajax_flexible_edit_reuse($handler) {
  $settings = &$handler->display->layout_settings;
  panels_flexible_convert_settings($settings, $handler->plugins['layout']);

  $form_state = array(
    'display' => &$handler->display,
    'settings' => &$settings,
    'ajax' => TRUE,
    'title' => t('Save this layout for reuse'),
  );

  $output = ctools_modal_form_wrapper('panels_flexible_reuse_form', $form_state);
  if (empty($output)) {
    // Create the new layout.
    ctools_include('export');
    $layout = ctools_export_crud_new('panels_layout');
    $layout->plugin = 'flexible';
    $layout->name = $form_state['values']['name'];
    $layout->admin_title = $form_state['values']['admin_title'];
    $layout->admin_description = $form_state['values']['admin_description'];
    $layout->category = $form_state['values']['category'];
    $layout->settings = $handler->display->layout_settings;

    // Save it.
    ctools_export_crud_save('panels_layout', $layout);

    if (empty($form_state['values']['keep'])) {
      // Set the actual layout_settings to now use the newly minted layout:
      $handler->display->layout = 'flexible:' . $layout->name;
      $handler->display->layout_settings = array();

      // Save our new state.
      panels_edit_cache_set($handler->cache);
    }

    // Dismiss the modal.
    $output[] = ctools_modal_command_dismiss();
  }

  $handler->commands = $output;
}

function panels_flexible_reuse_form(&$form_state) {
  $form['markup'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => t('If you save this layout for reuse it will appear in the list of reusable layouts at admin/build/panels/layouts, and you will need to go there to edit it. This layout will then become an option for all future panels you make.'),
  );

  $form['admin_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Administrative title'),
    '#description' => t('This will appear in the administrative interface to easily identify it.'),
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#description' => t('The machine readable name of this layout. It must be unique, and it must contain only alphanumeric characters and underscores. Once created, you will not be able to change this value!'),
  );

  $form['category'] = array(
    '#type' => 'textfield',
    '#title' => t('Category'),
    '#description' => t('What category this layout should appear in. If left blank the category will be "Miscellaneous".'),
  );

  $form['admin_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Administrative description'),
    '#description' => t('A description of what this layout is, does or is for, for administrative use.'),
  );

  $form['keep'] = array(
    '#type' => 'checkbox',
    '#title' => t('Keep current panel layout flexible'),
    '#description' => t('If checked, this panel will continue to use a generic flexible layout and will not use the saved layout. Use this option if you wish to clone this layout.'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function panels_flexible_reuse_form_validate(&$form, &$form_state) {
  if (empty($form_state['values']['name'])) {
    form_error($form['name'], t('You must choose a machine name.'));
  }

  ctools_include('export');
  $test = ctools_export_crud_load('panels_layout', $form_state['values']['name']);
  if ($test) {
    form_error($form['name'], t('That name is used by another layout: @layout', array('@layout' => $test->admin_title)));
  }

  // Ensure name fits the rules:
  if (preg_match('/[^a-zA-Z0-9_]/', $form_state['values']['name'])) {
    form_error($form['name'], t('Name must be alphanumeric or underscores only.'));
  }
}
