<?php

class panels_layouts_ui extends ctools_export_ui {
  var $lipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam egestas congue nibh, vel dictum ante posuere vitae. Cras gravida massa tempor metus eleifend sed elementum tortor scelerisque. Vivamus egestas, tortor quis luctus tristique, sem velit adipiscing risus, et tempus enim felis in massa. Morbi viverra, nisl quis rhoncus imperdiet, turpis massa vestibulum turpis, egestas faucibus nibh metus vel nunc. In hac habitasse platea dictumst. Nunc sit amet nisi quis ipsum tincidunt semper. Donec ac urna enim, et placerat arcu. Morbi eu laoreet justo. Nullam nec velit eu neque mattis pulvinar sed non libero. Sed sed vulputate erat. Fusce sit amet dui nibh.";

  function hook_menu(&$items) {
    // During updates, this can run before our schema is set up, so our
    // plugin can be empty.
    if (empty($this->plugin['menu']['items']['add'])) {
      return;
    }

    // Change the item to a tab on the Panels page.
    $this->plugin['menu']['items']['list callback']['type'] = MENU_LOCAL_TASK;

    // Establish a base for adding plugins
    $base = $this->plugin['menu']['items']['add'];
    // Remove the default 'add' menu item.
    unset($this->plugin['menu']['items']['add']);

    ctools_include('plugins', 'panels');
    $this->builders = panels_get_layout_builders();
    asort($this->builders);
    foreach ($this->builders as $name => $builder) {
      // Create a new menu item for the builder
      $item = $base;
      $item['title'] = !empty($builder['builder tab title']) ? $builder['builder tab title'] : 'Add ' . $builder['title'];
      $item['page arguments'][] = $name;
      $item['path'] = 'add-' . $name;
      $this->plugin['menu']['items']['add ' . $name] = $item;
    }

    parent::hook_menu($items);
  }

  function edit_form(&$form, &$form_state) {
    ctools_include('plugins', 'panels');
    // If the plugin is not set, then it should be provided as an argument:
    if (!isset($form_state['item']->plugin)) {
      $form_state['item']->plugin = $form_state['function args'][2];
    }

    parent::edit_form($form, $form_state);

    $form['category'] = array(
      '#type' => 'textfield',
      '#title' => t('Category'),
      '#description' => t('What category this layout should appear in. If left blank the category will be "Miscellaneous".'),
      '#default_value' => $form_state['item']->category,
    );

    ctools_include('context');
    ctools_include('display-edit', 'panels');
    ctools_include('content');

    // Provide actual layout admin UI here.
    // Create a display for editing:
    $cache_key = 'builder-' . $form_state['item']->name;

    // Load the display being edited from cache, if possible.
    if (!empty($_POST) && is_object($cache = panels_edit_cache_get($cache_key))) {
      $display = &$cache->display;
    }
    else {
      $content_types = ctools_content_get_available_types();

      panels_cache_clear('display', $cache_key);
      $cache = new stdClass();

      $display = panels_new_display();
      $display->did = $form_state['item']->name;
      $display->layout = $form_state['item']->plugin;
      $display->layout_settings = $form_state['item']->settings;
      $display->cache_key = $cache_key;
      $display->editing_layout = TRUE;

      $cache->display = $display;
      $cache->content_types = $content_types;
      $cache->display_title = FALSE;
      panels_edit_cache_set($cache);
    }

    // Set up lipsum content in all of the existing panel regions:
    $display->content = array();
    $display->panels = array();
    $custom = ctools_get_content_type('custom');
    $layout = panels_get_layout($display->layout);

    $regions = panels_get_regions($layout, $display);
    foreach ($regions as $id => $title) {
      $pane = panels_new_pane('custom', 'custom');
      $pane->pid = $id;
      $pane->panel = $id;
      $pane->configuration = ctools_content_get_defaults($custom, 'custom');
      $pane->configuration['title'] = 'Lorem Ipsum';
      $pane->configuration['body'] = $this->lipsum;
      $display->content[$id] = $pane;
      $display->panels[$id] = array($id);
    }

    $form_state['display'] = &$display;
    // Tell the Panels form not to display buttons.
    $form_state['no buttons'] = TRUE;
    $form_state['no display settings'] = TRUE;

    $form_state['cache_key'] = $cache_key;
    $form_state['content_types'] = $cache->content_types;
    $form_state['display_title'] = FALSE;

    $form_state['renderer'] = panels_get_renderer_handler('editor', $cache->display);
    $form_state['renderer']->cache = &$cache;

    $form = array_merge($form, panels_edit_display_form($form_state));
    // Make sure the theme will work since our form id is different.
    $form['#theme'] = 'panels_edit_display_form';

    // If we leave the standard submit handler, it'll try to reconcile
    // content from the input, but we've not exposed that to the user. This
    // makes previews work with the content we forced in.
    $form['preview']['button']['#submit'] = array('panels_edit_display_form_preview');
  }

  function edit_form_submit(&$form, &$form_state) {
    parent::edit_form_submit($form, $form_state);
    $form_state['item']->settings = $form_state['display']->layout_settings;
  }

  function list_form(&$form, &$form_state) {
    ctools_include('plugins', 'panels');
    $this->builders = panels_get_layout_builders();
    parent::list_form($form, $form_state);

    $categories = $plugins = array('all' => t('- All -'));
    foreach ($this->items as $item) {
      $categories[$item->category] = $item->category ? $item->category : t('Miscellaneous');
    }

    $form['top row']['category'] = array(
      '#type' => 'select',
      '#title' => t('Category'),
      '#options' => $categories,
      '#default_value' => 'all',
      '#weight' => -10,
    );

    foreach ($this->builders as $name => $plugin) {
      $plugins[$name] = $plugin['title'];
    }

    $form['top row']['plugin'] = array(
      '#type' => 'select',
      '#title' => t('Type'),
      '#options' => $plugins,
      '#default_value' => 'all',
      '#weight' => -9,
    );
  }

  function list_filter($form_state, $item) {
    if ($form_state['values']['category'] != 'all' && $form_state['values']['category'] != $item->category) {
      return TRUE;
    }

    if ($form_state['values']['plugin'] != 'all' && $form_state['values']['plugin'] != $item->plugin) {
      return TRUE;
    }

    return parent::list_filter($form_state, $item);
  }

  function list_sort_options() {
    return array(
      'disabled' => t('Enabled, title'),
      'title' => t('Title'),
      'name' => t('Name'),
      'category' => t('Category'),
      'storage' => t('Storage'),
      'plugin' => t('Type'),
    );
  }

  function list_build_row($item, &$form_state, $operations) {
    // Set up sorting
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$item->name] = empty($item->disabled) . $item->admin_title;
        break;
      case 'title':
        $this->sorts[$item->name] = $item->admin_title;
        break;
      case 'name':
        $this->sorts[$item->name] = $item->name;
        break;
      case 'category':
        $this->sorts[$item->name] = ($item->category ? $item->category : t('Miscellaneous')) . $item->admin_title;
        break;
      case 'plugin':
        $this->sorts[$item->name] = $item->plugin;
        break;
      case 'storage':
        $this->sorts[$item->name] = $item->type . $item->admin_title;
        break;
    }

    $type = !empty($this->builders[$item->plugin]) ? $this->builders[$item->plugin]['title'] : t('Broken/missing plugin');
    $category = $item->category ? check_plain($item->category) : t('Miscellaneous');
    $this->rows[$item->name] = array(
      'data' => array(
        array('data' => check_plain($type), 'class' => 'ctools-export-ui-type'),
        array('data' => check_plain($item->name), 'class' => 'ctools-export-ui-name'),
        array('data' => check_plain($item->admin_title), 'class' => 'ctools-export-ui-title'),
        array('data' => $category, 'class' => 'ctools-export-ui-category'),
        array('data' => theme('links', $operations), 'class' => 'ctools-export-ui-operations'),
      ),
      'title' => check_plain($item->admin_description),
      'class' => !empty($item->disabled) ? 'ctools-export-ui-disabled' : 'ctools-export-ui-enabled',
    );
  }

  function list_table_header() {
    return array(
      array('data' => t('Type'), 'class' => 'ctools-export-ui-type'),
      array('data' => t('Name'), 'class' => 'ctools-export-ui-name'),
      array('data' => t('Title'), 'class' => 'ctools-export-ui-title'),
      array('data' => t('Category'), 'class' => 'ctools-export-ui-category'),
      array('data' => t('Operations'), 'class' => 'ctools-export-ui-operations'),
    );
  }
}
