<?php

/**
 * @file
 * Class file to control the main Panels editor.
 */

class panels_renderer_editor extends panels_renderer_standard {

  /**
   * An array of AJAX commands to return. If populated it will automatically
   * be used by the AJAX router.
   */
  var $commands = array();
  var $admin = TRUE;
  // -------------------------------------------------------------------------
  // Display edit rendering.

  function edit() {
    ctools_include('form');
    $form_state = array(
      'display' => &$this->display,
      'renderer' => &$this,
      'content_types' => $this->cache->content_types,
      're_render' => FALSE,
      'no_redirect' => TRUE,
      'display_title' => !empty($this->cache->display_title),
      'cache key' => $this->display->cache_key,
    );

    $output = ctools_build_form('panels_edit_display_form', $form_state);
    if ($output) {
      return $output;
    }

    // no output == submit
    if (!$output) {
      if (!empty($form_state['clicked_button']['#save-display'])) {
        drupal_set_message(t('Panel content has been updated.'));
        panels_save_display($this->display);
      }
      else {
        drupal_set_message(t('Your changes have been discarded.'));
      }

      panels_cache_clear('display', $this->display->did);
      return $this->display;
    }
  }

  function add_meta() {
    parent::add_meta();
    if ($this->admin) {
      ctools_include('ajax');
      ctools_include('modal');
      ctools_modal_add_js();

      ctools_add_js('panels-base', 'panels');
      ctools_add_js('display_editor', 'panels');
      ctools_add_css('panels_dnd', 'panels');
      ctools_add_css('panels_admin', 'panels');
    }
  }

  function render() {
    // Pass through to normal rendering if not in admin mode.
    if (!$this->admin) {
      return parent::render();
    }

    $this->add_meta();

    $output = '<div class="panels-dnd" id="panels-dnd-main">';
    $output .= $this->render_layout();
    $output .= '</div>';

    return $output;
  }

  function render_region($region_id, $panes) {
    // Pass through to normal rendering if not in admin mode.
    if (!$this->admin) {
      return parent::render_region($region_id, $panes);
    }

    $content = implode('', $panes);

    $panel_buttons = $this->get_region_links($region_id);

    // @todo this should be panel-region not panels-display -- but CSS and .js has to be updated.
    $output = "<div class='panels-display' id='panel-pane-$region_id'>";
    $output .= $panel_buttons;
    $output .= "<h2 class='label'>" . check_plain($this->plugins['layout']['panels'][$region_id]) . "</h2>";
    $output .= $content;
    $output .= "</div>";

    return $output;
  }

  function render_pane(&$pane) {
    // Pass through to normal rendering if not in admin mode.
    if (!$this->admin) {
      return parent::render_pane($pane);
    }

    ctools_include('content');
    $content_type = ctools_get_content_type($pane->type);

    // This is just used for the title bar of the pane, not the content itself.
    // If we know the content type, use the appropriate title for that type,
    // otherwise, set the title using the content itself.
    $title = ctools_content_admin_title($content_type, $pane->subtype, $pane->configuration, $this->display->context);
    if (!$title) {
      $title = t('Deleted/missing content type @type', array('@type' => $pane->type));
    }

    $buttons = $this->get_pane_links($pane, $content_type);

    // Render administrative buttons for the pane.

    $block = new stdClass();
    if (empty($content_type)) {
      $block->title = '<em>' . t('Missing content type') . '</em>';
      $block->content = t('This pane\'s content type is either missing or has been deleted. This pane will not render.');
    }
    else {
      $block = ctools_content_admin_info($content_type, $pane->subtype, $pane->configuration, $this->display->context);
    }

    $output = '';
    $class = 'panel-pane';

    if (empty($pane->shown)) {
      $class .= ' hidden-pane';
    }

    if (isset($this->display->title_pane) && $this->display->title_pane == $pane->pid) {
      $class .= ' panel-pane-is-title';
    }

    $output = '<div class="' . $class . '" id="panel-pane-' . $pane->pid . '">';

    if (!$block->title) {
      $block->title = t('No title');
    }

    $output .= '<div class="grabber">';
    if ($buttons) {
      $output .= '<span class="buttons">' . $buttons . '</span>';
    }
    $output .= '<span class="text">' . $title . '</span>';
    $output .= '</div>'; // grabber

    $output .= '<div class="panel-pane-collapsible">';
    $output .= '<div class="pane-title">' . $block->title . '</div>';
    $output .= '<div class="pane-content">' . filter_xss_admin($block->content) . '</div>';
    $output .= '</div>'; // panel-pane-collapsible

    $output .= '</div>'; // panel-pane

    return $output;
  }

  /**
   * Get the style links.
   *
   * This is abstracted out since we have styles on both panes and regions.
   */
  function get_style_links($type, $id = NULL) {
    $info = $this->get_style($type, $id);
    $style = $info[0];
    $conf = $info[1];

    $style_title = isset($style['title']) ? $style['title'] : t('Default');

    $style_links[] = array(
      'title' => $style_title,
      'attributes' => array('class' => 'panels-text'),
    );

    $style_links[] = array(
      'title' => t('Change'),
      'href' => $this->get_url('style-type', $type, $id),
      'attributes' => array('class' => 'ctools-use-modal'),
    );

    $function = $type != 'pane' ? 'settings form' : 'pane settings form';
    if (panels_plugin_get_function('styles', $style, $function)) {
      $style_links[] = array(
        'title' => t('Settings'),
        'href' => $this->get_url('style-settings', $type, $id),
        'attributes' => array('class' => 'ctools-use-modal'),
      );
    }

    return $style_links;
  }

  /**
   * Get the links for a panel display.
   *
   * This is abstracted out for easy ajax replacement.
   */
  function get_display_links() {
    $links = array();

    if (user_access('administer panels styles')) {
      $style_links = $this->get_style_links('display');

      $links[] = array(
        'title' => '<span class="dropdown-header">' . t('Style') . '</span>' . theme_links($style_links),
        'html' => TRUE,
        'attributes' => array('class' => 'panels-sub-menu'),
      );
    }

    if (user_access('use panels caching features')) {
      $links[] = array(
        'title' => '<hr />',
        'html' => TRUE,
      );

      $method = isset($this->display->cache['method']) ? $this->display->cache['method'] : 0;
      $info = panels_get_cache($method);
      $cache_method = isset($info['title']) ? $info['title'] : t('No caching');

      $cache_links[] = array(
        'title' => $cache_method,
        'attributes' => array('class' => 'panels-text'),
      );
      $cache_links[] = array(
        'title' => t('Change'),
        'href' => $this->get_url('cache-method', 'display'),
        'attributes' => array('class' => 'ctools-use-modal'),
      );
      if (panels_plugin_get_function('cache', $info, 'settings form')) {
        $cache_links[] = array(
          'title' => t('Settings'),
          'href' => $this->get_url('cache-settings', 'display'),
          'attributes' => array('class' => 'ctools-use-modal'),
        );
      }

      $links[] = array(
        'title' => '<span class="dropdown-header">' . t('Caching') . '</span>' . theme_links($cache_links),
        'html' => TRUE,
        'attributes' => array('class' => 'panels-sub-menu'),
      );
    }

    return theme('ctools_dropdown', t('Display settings'), $links, FALSE, 'panels-display-links');
  }

  /**
   * Render the links to display when editing a region.
   */
  function get_region_links($region_id) {
    $links = array();
    $links[] = array(
      'title' => t('Add content'),
      'href' => $this->get_url('select-content', $region_id),
      'attributes' => array(
        'class' => 'ctools-use-modal',
      ),
    );

    if (user_access('administer panels styles')) {
      $links[] = array(
        'title' => '<hr />',
        'html' => TRUE,
      );

      $style_links = $this->get_style_links('region', $region_id);

      $links[] = array(
        'title' => '<span class="dropdown-header">' . t('Style') . '</span>' . theme_links($style_links),
        'html' => TRUE,
        'attributes' => array('class' => 'panels-sub-menu'),
      );
    }

    return theme('ctools_dropdown', theme('image', ctools_image_path('icon-addcontent.png', 'panels')), $links, TRUE, 'pane-add-link panels-region-links-' . $region_id);
  }

  /**
   * Render the links to display when editing a pane.
   */
  function get_pane_links($pane, $content_type) {
    $links = array();

    if (!empty($pane->shown)) {
      $links[] = array(
        'title' => t('Disable this pane'),
        'href' => $this->get_url('hide', $pane->pid),
        'attributes' => array('class' => 'ctools-use-ajax'),
      );
    }
    else {
      $links[] = array(
        'title' => t('Enable this pane'),
        'href' => $this->get_url('show', $pane->pid),
        'attributes' => array('class' => 'ctools-use-ajax'),
      );
    }

    if (isset($this->display->title_pane) && $this->display->title_pane == $pane->pid) {
      $links['panels-set-title'] = array(
        'title' => t('&#x2713;Panel title'),
        'html' => TRUE,
      );
    }
    else {
      $links['panels-set-title'] = array(
        'title' => t('Panel title'),
        'href' => $this->get_url('panel-title', $pane->pid),
        'attributes' => array('class' => 'ctools-use-ajax'),
      );
    }

    $subtype = ctools_content_get_subtype($content_type, $pane->subtype);

    if (ctools_content_editable($content_type, $subtype, $pane->configuration)) {
      $links[] = array(
        'title' => isset($content_type['edit text']) ? $content_type['edit text'] : t('Settings'),
        'href' => $this->get_url('edit-pane', $pane->pid),
        'attributes' => array('class' => 'ctools-use-modal'),
      );
    }

    if (user_access('administer advanced pane settings')) {
      $links[] = array(
        'title' => t('CSS properties'),
        'href' => $this->get_url('pane-css', $pane->pid),
        'attributes' => array('class' => 'ctools-use-modal'),
      );
    }

    if (user_access('administer panels styles')) {
      $links[] = array(
        'title' => '<hr />',
        'html' => TRUE,
      );

      $style_links = $this->get_style_links('pane', $pane->pid);

      $links[] = array(
        'title' => '<span class="dropdown-header">' . t('Style') . '</span>' . theme_links($style_links),
        'html' => TRUE,
        'attributes' => array('class' => 'panels-sub-menu'),
      );
    }

    if (user_access('administer pane access')) {
      $links[] = array(
        'title' => '<hr />',
        'html' => TRUE,
      );

      $contexts = $this->display->context;
      // Make sure we have the logged in user context
      if (!isset($contexts['logged-in-user'])) {
        $contexts['logged-in-user'] = ctools_access_get_loggedin_context();
      }

      $visibility_links = array();

      if (!empty($pane->access['plugins'])) {
        foreach ($pane->access['plugins'] as $id => $test) {
          $plugin = ctools_get_access_plugin($test['name']);
          $access_title  = isset($plugin['title']) ? $plugin['title'] : t('Broken/missing access plugin %plugin', array('%plugin' => $test['name']));
          $access_description = ctools_access_summary($plugin, $contexts, $test);

          $visibility_links[] = array(
            'title' => $access_description,
            'href' => $this->get_url('access-configure-test', $pane->pid, $id),
            'attributes' => array('class' => 'ctools-use-modal panels-italic'),
          );
        }
      }
      if (empty($visibility_links)) {
        $visibility_links[] = array(
          'title' => t('No rules'),
          'attributes' => array('class' => 'panels-text'),
        );
      }

      $visibility_links[] = array(
        'title' => t('Add new rule'),
        'href' => $this->get_url('access-add-test', $pane->pid),
        'attributes' => array('class' => 'ctools-use-modal'),
      );

      $visibility_links[] = array(
        'title' => t('Settings'),
        'href' => $this->get_url('access-settings', $pane->pid),
        'attributes' => array('class' => 'ctools-use-modal'),
      );

      $links[] = array(
        'title' => '<span class="dropdown-header">' . t('Visibility rules') . '</span>' . theme_links($visibility_links),
        'html' => TRUE,
        'attributes' => array('class' => 'panels-sub-menu'),
      );
    }

    if (panels_get_caches() && user_access('use panels caching features')) {
      $links[] = array(
        'title' => '<hr />',
        'html' => TRUE,
      );

      $method = isset($pane->cache['method']) ? $pane->cache['method'] : 0;
      $info = panels_get_cache($method);
      $cache_method = isset($info['title']) ? $info['title'] : t('No caching');
      $cache_links[] = array(
        'title' => $cache_method,
        'attributes' => array('class' => 'panels-text'),
      );
      $cache_links[] = array(
        'title' => t('Change'),
        'href' => $this->get_url('cache-method', $pane->pid),
        'attributes' => array('class' => 'ctools-use-modal'),
      );
      if (panels_plugin_get_function('cache', $info, 'settings form')) {
        $cache_links[] = array(
          'title' => t('Settings'),
          'href' => $this->get_url('cache-settings', $pane->pid),
          'attributes' => array('class' => 'ctools-use-modal'),
        );
      }

      $links[] = array(
        'title' => '<span class="dropdown-header">' . t('Caching') . '</span>' . theme_links($cache_links),
        'html' => TRUE,
        'attributes' => array('class' => 'panels-sub-menu'),
      );
    }

    $links[] = array(
      'title' => '<hr />',
      'html' => TRUE,
    );

    $links[] = array(
      'title' => t('Remove'),
      'href' => '#',
      'attributes' => array(
        'class' => 'pane-delete',
        'id' => "pane-delete-panel-pane-$pane->pid",
      ),
    );

    return theme('ctools_dropdown', theme('image', ctools_image_path('icon-configure.png', 'panels')), $links, TRUE);
  }

  // -----------------------------------------------------------------------
  // Display edit AJAX callbacks and helpers.

  /**
   * Generate a URL path for the AJAX editor.
   */
  function get_url() {
    $args = func_get_args();
    $command = array_shift($args);
    $url = 'panels/ajax/' . $this->plugin['name'] . '/' . $command . '/' . $this->display->cache_key;
    if ($args) {
      $url .= '/' . implode('/', $args);
    }

    return $url;
  }

  /**
   * Get the Panels storage oparation for a given renderer AJAX method.
   *
   * @param string $method
   *   The method name.
   *
   * @return string
   *   The Panels storage op.
   */
  function get_panels_storage_op_for_ajax($method) {
    switch ($method) {
      case 'ajax_show':
      case 'ajax_hide':
      case 'ajax_select_content':
      case 'ajax_add_pane':
      case 'ajax_edit_pane':
      case 'ajax_panel_title':
      case 'ajax_cache_method':
      case 'ajax_cache_settings':
      case 'ajax_style_type':
      case 'ajax_style_settings':
      case 'ajax_pane_css':
      case 'ajax_lock':
      case 'ajax_access_settings':
      case 'ajax_access_add_test':
      case 'ajax_access_configure_test':
      case 'ajax_layout':
      case 'ajax_style':
        return 'update';
    }

    return parent::get_panels_storage_op($method);
  }

  /**
   * AJAX command to show a pane.
   */
  function ajax_show($pid = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_ajax_render_error(t('Invalid pane id.'));
    }

    $this->display->content[$pid]->shown = TRUE;
    panels_edit_cache_set($this->cache);

    $this->command_update_pane($pid);
  }

  /**
   * AJAX command to show a pane.
   */
  function ajax_hide($pid = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_ajax_render_error(t('Invalid pane id.'));
    }

    $this->display->content[$pid]->shown = FALSE;
    panels_edit_cache_set($this->cache);

    $this->command_update_pane($pid);
  }

  /**
   * AJAX command to present a dialog with a list of available content.
   */
  function ajax_select_content($region = NULL, $category = NULL) {
    if (!array_key_exists($region, $this->plugins['layout']['panels'])) {
      ctools_modal_render(t('Error'), t('Invalid input'));
    }

    $title = t('Add content to !s', array('!s' => $this->plugins['layout']['panels'][$region]));

    $categories = $this->get_categories($this->cache->content_types);

    if (empty($categories)) {
      $output = t('There are no content types you may add to this display.');
    }
    else {
      $output = '<div class="panels-add-content-modal">';
      $selector = $this->render_category_selector($categories, $category, $region);

      $content = !empty($categories[$category]['content']) ? $categories[$category]['content'] : array();
      $center = $this->render_category($content, $category, $region);

      $output .= '<div class="panels-section-column panels-section-column-categories">'
        . '<div class="inside">' . $selector . '</div></div>';
      $output .= $center;
      $output .= '</div>'; // panels-add-content-modal
    }

    $this->commands[] = ctools_modal_command_display($title, $output);
  }

  /**
   * Return the category name and the category key of a given content
   * type.
   *
   * @todo -- this should be in CTools.
   */
  function get_category($content_type) {
    if (isset($content_type['top level'])) {
      $category = 'root';
    }
    else if (isset($content_type['category'])) {
      if (is_array($content_type['category'])) {
        list($category, $weight) = $content_type['category'];
      }
      else {
        $category = $content_type['category'];
      }
    }
    else {
      $category = t('Uncategorized');
    }

    return array(preg_replace('/[^a-z0-9]/', '-', strtolower($category)), $category);
  }


  /**
   * Create a list of categories from all of the content type.
   *
   * @return array
   *   An array of categories. Each entry in the array will also be an array
   *   with 'title' as the printable title of the category, and 'content'
   *   being an array of all content in the category. Each item in the 'content'
   *   array contain the array plugin definition so that it can be later
   *   found in the content array. They will be keyed by the title so that they
   *   can be sorted.
   */
  function get_categories($content_types) {
    $categories = array();
    $category_names = array();

    foreach ($content_types as $type_name => $subtypes) {
      foreach ($subtypes as $subtype_name => $content_type) {
        list($category_key, $category) = $this->get_category($content_type);

        if (empty($categories[$category_key])) {
          $categories[$category_key] = array(
            'title' => $category,
            'content' => array(),
          );
          $category_names[$category_key] = $category;
        }

        $content_title = filter_xss_admin($content_type['title']);

        // Ensure content with the same title doesn't overwrite each other.
        while (isset($categories[$category_key]['content'][$content_title])) {
          $content_title .= '-';
        }

        $categories[$category_key]['content'][$content_title] = $content_type;
        $categories[$category_key]['content'][$content_title]['type_name'] = $type_name;
        $categories[$category_key]['content'][$content_title]['subtype_name'] = $subtype_name;
      }
    }

    // Now sort
    natcasesort($category_names);
    foreach ($category_names as $category => $name) {
      $output[$category] = $categories[$category];
    }

    return $output;
  }

  /**
   * Render a single link to add a content type.
   */
  function render_add_content_link($region, $content_type) {
    $title = filter_xss_admin($content_type['title']);
    $description = isset($content_type['description']) ? $content_type['description'] : $title;
    $icon = ctools_content_admin_icon($content_type);
    $url = $this->get_url('add-pane', $region, $content_type['type_name'], $content_type['subtype_name']);

    $output = '<div class="content-type-button clear-block">';
    $output .= ctools_ajax_image_button($icon, $url, $description, 'panels-modal-add-config');
    $output .= '<div>' . ctools_ajax_text_button($title, $url, $description, 'panels-modal-add-config') . '</div>';
    $output .= '</div>';

    return $output;
  }

  /**
   * Render the selector widget in the add content modal to select categories.
   */
  function render_category_selector($categories, $category, $region) {
    $output = '<div class="panels-categories-box">';

    // Render our list of categories in column 0.
    foreach ($categories as $key => $category_info) {
      if ($key == 'root') {
        continue;
      }

      $class = 'panels-modal-add-category';
      if ($key == $category) {
        $class .= ' active';
      }

      $url = $this->get_url('select-content', $region, $key);
      $output .= ctools_ajax_text_button($category_info['title'], $url, '', $class);
    }

    $output .= '</div>'; // panels-categories-box

    if (!empty($categories['root'])) {
      foreach ($categories['root']['content'] as $content_type) {
        $output .= $this->render_add_content_link($region, $content_type);
      }
    }

    return $output;
  }

  /**
   * Render all of the content links in a category.
   */
  function render_category($content, $category, $region) {
    if (empty($category) || empty($content) || $category == 'root') {
      $output = '<div class="panels-categories-description">';
      $output .= t('Content options are divided by category. Please select a category from the left to proceed.');
      $output .= '</div>';
    }
    else {
      $titles = array_keys($content);
      natcasesort($titles);

      // Fill out the info for our current category.
      $columns = 2;
      $col[1] = '';
      $col[2] = '';

      $col_size = count($titles) / $columns;
      $count = 0;
      foreach ($titles as $title) {
        $which = floor($count++ / $col_size) + 1; // we leave 0 for the categories.
        $col[$which] .= $this->render_add_content_link($region, $content[$title]);
      }

      $output = '<div class="panels-section-columns">';
      foreach ($col as $id => $column) {
        $output .= '<div class="panels-section-column panels-section-column-' . $id . '">'
        . '<div class="inside">' . $column . '</div></div>';
      }
      $output .= '</div>'; // columns
    }

    if ($messages = theme('status_messages')) {
      $output = '<div class="messages">' . $messages . '</div>' . $output;
    }
    return $output;
  }

  /**
   * AJAX entry point to add a new pane.
   */
  function ajax_add_pane($region = NULL, $type_name = NULL, $subtype_name = NULL, $step = NULL) {
    $content_type = ctools_get_content_type($type_name);
    $subtype = ctools_content_get_subtype($content_type, $subtype_name);

    if (!isset($step) || !isset($this->cache->new_pane)) {
      $pane = panels_new_pane($type_name, $subtype_name, TRUE);
      $this->cache->new_pane = &$pane;
    }
    else {
      $pane = &$this->cache->new_pane;
    }

    $form_state = array(
      'display' => &$this->cache->display,
      'contexts' => $this->cache->display->context,
      'pane' => &$pane,
      'cache_key' => $this->display->cache_key,
      'cache' => &$this->cache,
      'ajax' => TRUE,
      'modal' => TRUE,
      // This will force the system to not automatically render.
      'modal return' => TRUE,
      'commands' => array(),
    );

    $form_info = array(
      'path' => $this->get_url('add-pane', $region, $type_name, $subtype_name, '%step'),
      'show cancel' => TRUE,
      'next callback' => 'panels_ajax_edit_pane_next',
      'finish callback' => 'panels_ajax_edit_pane_finish',
      'cancel callback' => 'panels_ajax_edit_pane_cancel',
    );

    $output = ctools_content_form('add', $form_info, $form_state, $content_type, $pane->subtype, $subtype, $pane->configuration, $step);

    // If $rc is FALSE, there was no actual form.
    if ($output === FALSE || !empty($form_state['complete'])) {
      $pane = $this->cache->new_pane;
      unset($this->cache->new_pane);

      // Add the pane to the display
      $this->display->add_pane($pane, $region);
      panels_edit_cache_set($this->cache);

      // Tell the client to draw the pane
      $this->command_add_pane($pane);

      // Dismiss the modal.
      $this->commands[] = ctools_modal_command_dismiss();
    }
    else if (!empty($form_state['cancel'])) {
      // If cancelling, return to the activity.
      list($category_key, $category) = $this->get_category($subtype);
      $this->ajax_select_content($region, $category_key);
    }
    else {
      // This overwrites any previous commands.
      $this->commands = ctools_modal_form_render($form_state, $output);
    }
  }

  /**
   * AJAX entry point to edit a pane.
   */
  function ajax_edit_pane($pid = NULL, $step = NULL) {
    if (empty($this->cache->display->content[$pid])) {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    $pane = &$this->cache->display->content[$pid];

    $content_type = ctools_get_content_type($pane->type);
    $subtype = ctools_content_get_subtype($content_type, $pane->subtype);

    $form_state = array(
      'display' => &$this->cache->display,
      'contexts' => $this->cache->display->context,
      'pane' => &$pane,
      'cache' => &$this->cache,
      'ajax' => TRUE,
      'modal' => TRUE,
      'modal return' => TRUE,
      'commands' => array(),
    );

    $form_info = array(
      'path' => $this->get_url('edit-pane', $pid, '%step'),
      'show cancel' => TRUE,
      'next callback' => 'panels_ajax_edit_pane_next',
      'finish callback' => 'panels_ajax_edit_pane_finish',
      'cancel callback' => 'panels_ajax_edit_pane_cancel',
    );

    $output = ctools_content_form('edit', $form_info, $form_state, $content_type, $pane->subtype,  $subtype, $pane->configuration, $step);

    // If $rc is FALSE, there was no actual form.
    if ($output === FALSE || !empty($form_state['cancel'])) {
      // Dismiss the modal.
      $this->commands[] = ctools_modal_command_dismiss();
    }
    else if (!empty($form_state['complete'])) {
      panels_edit_cache_set($this->cache);
      $this->command_update_pane($pid);
      $this->commands[] = ctools_modal_command_dismiss();
    }
    else {
      // This overwrites any previous commands.
      $this->commands = ctools_modal_form_render($form_state, $output);
    }
  }

  /**
   * AJAX entry point to select which pane is currently the title.
   *
   * @param string $pid
   *   The pane id for the pane object whose title state we're setting.
   */
  function ajax_panel_title($pid = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_ajax_render_error(t('Invalid pane id.'));
    }

    $pane = &$this->display->content[$pid];

    $old_title = !empty($this->display->title_pane) ? $this->display->title_pane : NULL;
    $this->display->title_pane = $pid;

    panels_edit_cache_set($this->cache);

    $this->command_update_pane($pane);

    if ($old_title && !empty($this->cache->display->content[$old_title])) {
      $this->command_update_pane($this->cache->display->content[$old_title]);
    }
  }

  /**
   * AJAX entry point to configure the cache method for a pane or the display.
   *
   * @param string $pid
   *   Either a pane id for a pane in the display, or 'display' to edit the
   *   display cache settings.
   */
  function ajax_cache_method($pid = NULL) {
    ctools_include('content');
    // This lets us choose whether we're doing the display's cache or
    // a pane's.
    if ($pid == 'display') {
      $conf = &$this->display->cache;
      $title = t('Cache method for this display');
    }
    else if (!empty($this->display->content[$pid])) {
      $pane = &$this->display->content[$pid];
      $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);
      $conf = &$pane->cache;
      $title = t('Cache method for !subtype_title', array('!subtype_title' => $subtype['title']));
    }
    else {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    $form_state = array(
      'display' => &$this->display,
      'conf' => &$conf,
      'title' => $title,
      'ajax' => TRUE,
    );

    $output = ctools_modal_form_wrapper('panels_edit_cache_method_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    // Preserve this; this way we don't actually change the method until they
    // have saved the form.
    $info = panels_get_cache($form_state['method']);
    $function = panels_plugin_get_function('cache', $info, 'settings form');
    if (!$function) {
      $conf['method'] = $form_state['method'];
      $conf['settings'] = array();
      panels_edit_cache_set($this->cache);

      $this->commands[] = ctools_modal_command_dismiss();

      if ($pid != 'display') {
        $this->command_update_pane($pane);
      }
      else {
        $this->command_update_display_links();
      }
    }
    else {
      $this->cache->method = $form_state['method'];
      panels_edit_cache_set($this->cache);
      // send them to next form.
      return $this->ajax_cache_settings($pid);
    }
  }

  /**
   * AJAX entry point to configure the cache settings for a pane or the display.
   *
   * @param string $pid
   *   Either a pane id for a pane in the display, or 'display' to edit the
   *   display cache settings.
   */
  function ajax_cache_settings($pid = 0) {
    ctools_include('content');

    // This lets us choose whether we're doing the display's cache or
    // a pane's.
    if ($pid == 'display') {
      $conf = &$this->display->cache;
      $title = t('Cache settings for this display');
    }
    else if (!empty($this->display->content[$pid])) {
      $pane = &$this->display->content[$pid];
      $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);

      $conf = &$pane->cache;
      $title = t('Cache settings for !subtype_title', array('!subtype_title' => $subtype['title']));
    }
    else {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    if (isset($this->cache->method) && (empty($conf['method']) || $conf['method'] != $this->cache->method)) {
      $conf['method'] = $this->cache->method;
      $info = panels_get_cache($conf['method']);
      $conf['settings'] = isset($info['defaults']) ? $info['defaults'] : array();
    }

    $form_state = array(
      'display' => &$this->display,
      'pid' => $pid,
      'conf' => &$conf,
      'ajax' => TRUE,
      'title' => $title,
      'url' => url($this->get_url('cache-settings', $pid), array('absolute' => TRUE)),
    );

    $output = ctools_modal_form_wrapper('panels_edit_cache_settings_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    panels_edit_cache_set($this->cache);

    $this->commands[] = ctools_modal_command_dismiss();

    if ($pid != 'display') {
      $this->command_update_pane($pane);
    }
    else {
      $this->command_update_display_links();
    }
  }

  /**
   * AJAX entry point to select the style for a display, region or pane.
   *
   * @param string $type
   *   Either display, region or pane
   * @param $pid
   *   The pane id, if a pane. The region id, if a region.
   */
  function ajax_style_type($type, $pid = NULL) {
    // This lets us choose whether we're doing the display's cache or
    // a pane's.
    switch ($type) {
      case 'display':
        $style = isset($this->display->panel_settings['style']) ? $this->display->panel_settings['style'] : 'default';
        $title = t('Default style for this display');
        break;

      case 'region':
        $style = isset($this->display->panel_settings[$pid]['style']) ? $this->display->panel_settings[$pid]['style'] : '-1'; // -1 signifies to use the default setting.
        $title = t('Panel style for region "!region"', array('!region' => $this->plugins['layout']['panels'][$pid]));
        break;

      case 'pane':
        ctools_include('content');
        $pane = &$this->display->content[$pid];
        $style = isset($pane->style['style']) ? $pane->style['style'] : 'default';
        $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);
        $title = t('Pane style for "!pane"', array('!pane' => $subtype['title']));
        break;

      default:
        ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }
    $info = $this->get_style($type, $pid);
    $style_plugin = $info[0];
    $style_settings = $info[1];

    // Backward compatibility: Translate old-style stylizer to new style
    // stylizer.
    if ($style == 'stylizer' && !empty($style_settings['style']) && $style_settings['style'] != '$') {
      $style = 'stylizer:' . $style_settings['style'];
    }

    $form_state = array(
      'display' => &$this->display,
      'style' => $style,
      'title' => $title,
      'ajax' => TRUE,
      'type' => $type,
    );

    $output = ctools_modal_form_wrapper('panels_edit_style_type_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    // Preserve this; this way we don't actually change the method until they
    // have saved the form.
    $style = panels_get_style($form_state['style']);
    $function = panels_plugin_get_function('styles', $style, ($type == 'pane') ? 'pane settings form' : 'settings form');
    if (!$function) {
      if (isset($this->cache->style)) {
        unset($this->cache->style);
      }

      // If there's no settings form, just change the style and exit.
      switch($type) {
        case 'display':
          $this->display->panel_settings['style'] = $form_state['style'];
          if (isset($this->display->panel_settings['style_settings']['default'])) {
            unset($this->display->panel_settings['style_settings']['default']);
          }
          break;

        case 'region':
          $this->display->panel_settings[$pid]['style'] = $form_state['style'];
          if (isset($this->display->panel_settings['style_settings'][$pid])) {
            unset($this->display->panel_settings['style_settings'][$pid]);
          }
          break;

        case 'pane':
          $pane->style['style'] = $form_state['style'];
          if (isset($pane->style['settings'])) {
            unset($pane->style['settings']);
          }

          break;
      }
      panels_edit_cache_set($this->cache);

      $this->commands[] = ctools_modal_command_dismiss();

      if ($type == 'pane') {
        $this->command_update_pane($pane);
      }
      else if ($type == 'region') {
        $this->command_update_region_links($pid);
      }
      else {
        $this->command_update_display_links();
      }
    }
    else {
      if ($form_state['style'] != $form_state['old_style']) {
        $this->cache->style = $form_state['style'];
        panels_edit_cache_set($this->cache);
      }

      // send them to next form.
      return $this->ajax_style_settings($type, $pid);
    }
  }

  /**
   * Get the appropriate style from the panel in the cache.
   *
   * Since we have styles for regions, panes and the display itself, and
   * they are stored differently, we use this method to simplify getting
   * style information into a way that's easy to cope with.
   */
  function get_style($type, $pid = '') {
    if (isset($this->cache->style)) {
      $style = panels_get_style($this->cache->style);
      $defaults = isset($style['defaults']) ? $style['defaults'] : array();
      // Get the &$conf variable based upon whose style we're editing.
      switch ($type) {
        case 'display':
          $this->display->panel_settings['style'] = $this->cache->style;
          $this->display->panel_settings['style_settings']['default'] = $defaults;
          break;

        case 'region':
          $this->display->panel_settings[$pid]['style'] = $this->cache->style;
          $this->display->panel_settings['style_settings'][$pid] = $defaults;
          break;

        case 'pane':
          $pane = &$this->display->content[$pid];
          $pane->style['style'] = $this->cache->style;
          $pane->style['settings'] = $defaults;
          $conf = &$pane->style['settings'];
          break;
      }
    }
    else {
      switch ($type) {
        case 'display':
          $style = panels_get_style((!empty($this->display->panel_settings['style'])) ? $this->display->panel_settings['style'] : 'default');
          break;

        case 'region':
          $style = panels_get_style((!empty($this->display->panel_settings[$pid]['style'])) ? $this->display->panel_settings[$pid]['style'] : '-1');
          break;

        case 'pane':
          $pane = &$this->display->content[$pid];
          $style = panels_get_style(!empty($pane->style['style']) ? $pane->style['style'] : 'default');
          break;
      }
    }

    // Set up our $conf reference.
    switch ($type) {
      case 'display':
        $conf = &$this->display->panel_settings['style_settings']['default'];
        break;

      case 'region':
        $conf = &$this->display->panel_settings['style_settings'][$pid];
        break;

      case 'pane':
        ctools_include('content');
        $pane = &$this->display->content[$pid];
        $conf = &$pane->style['settings'];
        break;
    }

    // Backward compatibility: Translate old-style stylizer to new style
    // stylizer.
    if ($style['name'] == 'stylizer' && !empty($conf['style']) && $conf['style'] != '$') {
      $style = panels_get_style('stylizer:' . $conf['style']);
    }

    return array($style, &$conf);
  }

  /**
   * AJAX entry point to configure the style for a display, region or pane.
   *
   * @param string $type
   *   Either display, region or pane
   * @param $pid
   *   The pane id, if a pane. The region id, if a region.
   */
  function ajax_style_settings($type, $pid = '') {
    $info = $this->get_style($type, $pid);
    $style = $info[0];
    $conf = &$info[1];

    switch ($type) {
      case 'display':
        $title = t('Style settings for @style (display)', array('@style' => $style['title']));
        break;

      case 'region':
        $title = t('Style settings for style @style (Region "!region")', array('@style' => $style['title'], '!region' => $this->plugins['layout']['panels'][$pid]));
        break;

      case 'pane':
        ctools_include('content');
        $pane = &$this->display->content[$pid];
        $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);
        $title = t('Style settings for style @style (Pane "!pane")', array('@style' => $style['title'], '!pane' => $subtype['title']));
        break;
    }

    $form_state = array(
      'display' => &$this->display,
      'type' => $type,
      'pid' => $pid,
      'conf' => &$conf,
      'style' => $style,
      'ajax' => TRUE,
      'title' => $title,
      'url' => url($this->get_url('style-settings', $type, $pid), array('absolute' => TRUE)),
      'renderer' => &$this,
    );

    $output = ctools_modal_form_wrapper('panels_edit_style_settings_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    if (isset($this->cache->style)) {
      unset($this->cache->style);
    }

    // $conf was a reference so it should just modify.
    panels_edit_cache_set($this->cache);

    $this->commands[] = ctools_modal_command_dismiss();

    if ($type == 'pane') {
      $this->command_update_pane($pane);
    }
    else if ($type == 'region') {
      $this->command_update_region_links($pid);
    }
    else {
      $this->command_update_display_links();
    }
  }

  /**
   * AJAX entry point to configure CSS for a pane.
   *
   * @param $pid
   *   The pane id to edit.
   */
  function ajax_pane_css($pid = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    $pane = &$this->display->content[$pid];
    $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);

    $form_state = array(
      'display' => &$this->display,
      'pane' => &$pane,
      'ajax' => TRUE,
      'title' => t('Configure CSS on !subtype_title', array('!subtype_title' => $subtype['title'])),
    );

    $output = ctools_modal_form_wrapper('panels_edit_configure_pane_css_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    panels_edit_cache_set($this->cache);
    $this->command_update_pane($pid);
    $this->commands[] = ctools_modal_command_dismiss();
  }

  /**
   * AJAX entry point to configure access settings for a pane.
   *
   * @param $pid
   *   The pane id to edit.
   */
  function ajax_access_settings($pid = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    $pane = &$this->display->content[$pid];
    $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);

    $form_state = array(
      'display' => &$this->display,
      'pane' => &$pane,
      'ajax' => TRUE,
      'title' => t('Access settings on !subtype_title', array('!subtype_title' => $subtype['title'])),
    );

    $output = ctools_modal_form_wrapper('panels_edit_configure_access_settings_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    panels_edit_cache_set($this->cache);
    $this->command_update_pane($pid);
    $this->commands[] = ctools_modal_command_dismiss();
  }

  /**
   * AJAX entry point for to add a visibility rule.
   */
  function ajax_access_add_test($pid = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    $pane = &$this->display->content[$pid];
    $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);

    $form_state = array(
      'display' => &$this->display,
      'pane' => &$pane,
      'ajax' => TRUE,
      'title' => t('Add visibility rule for !subtype_title', array('!subtype_title' => $subtype['title'])),
    );

    $output = ctools_modal_form_wrapper('panels_edit_add_access_test_form', $form_state);
    if (empty($output)) {
      // Set up the plugin in cache
      $plugin = ctools_get_access_plugin($form_state['values']['type']);
      $this->cache->new_plugin = ctools_access_new_test($plugin);
      panels_edit_cache_set($this->cache);

      // go to the next step.
      return $this->ajax_access_configure_test($pid, 'add');
    }

    ctools_ajax_render($output);
  }

  /**
   * AJAX entry point for to configure vsibility rule.
   */
  function ajax_access_configure_test($pid = NULL, $id = NULL) {
    if (empty($this->display->content[$pid])) {
      ctools_modal_render(t('Error'), t('Invalid pane id.'));
    }

    $pane = &$this->display->content[$pid];
    $subtype = ctools_content_get_subtype($pane->type, $pane->subtype);

    // Set this up here because $id gets changed later.
    $url = $this->get_url('access-configure-test', $pid, $id);

    // If we're adding a new one, get the stored data from cache and
    // add it. It's stored as a cache so that if this is closed
    // we don't accidentally add an unconfigured plugin.
    if ($id == 'add') {
      $pane->access['plugins'][] = $this->cache->new_plugin;
      $id = max(array_keys($pane->access['plugins']));
    }
    else if (empty($pane->access['plugins'][$id])) {
      ctools_modal_render(t('Error'), t('Invalid test id.'));
    }

    $form_state = array(
      'display' => &$this->display,
      'pane' => &$pane,
      'ajax' => TRUE,
      'title' => t('Configure visibility rule for !subtype_title', array('!subtype_title' => $subtype['title'])),
      'test' => &$pane->access['plugins'][$id],
      'plugin' => ctools_get_access_plugin($pane->access['plugins'][$id]['name']),
      'url' => url($url, array('absolute' => TRUE)),
    );

    $output = ctools_modal_form_wrapper('panels_edit_configure_access_test_form', $form_state);
    if (!empty($output)) {
      $this->commands = $output;
      return;
    }

    // Unset the new plugin
    if (isset($this->cache->new_plugin)) {
      unset($this->cache->new_plugin);
    }

    if (!empty($form_state['remove'])) {
      unset($pane->access['plugins'][$id]);
    }

    panels_edit_cache_set($this->cache);
    $this->command_update_pane($pid);
    $this->commands[] = ctools_modal_command_dismiss();
  }

  /**
   * AJAX Router function for layout owned AJAX calls.
   *
   * Layouts like the flexible layout builder need callbacks of their own.
   * This allows those layouts to simply declare their callbacks and use
   * them with $this->get_url('layout', $command).
   */
  function ajax_layout() {
    $args = func_get_args();
    if (empty($args)) {
      return MENU_NOT_FOUND;
    }

    $command = array_shift($args);
    if (empty($this->plugins['layout']['ajax'][$command]) || !function_exists($this->plugins['layout']['ajax'][$command])) {
      return MENU_NOT_FOUND;
    }

    // Make sure the this is always available to the called functions.
    array_unshift($args, $this);
    return call_user_func_array($this->plugins['layout']['ajax'][$command], $args);
  }

  /**
   * AJAX Router function for style owned AJAX calls.
   *
   * Styles like the stylizer need AJAX callbacks of their own. This
   * allows the system to figure out which style is being referenced,
   * load it, and execute the callback.
   *
   * This allows those layouts to simply declare their callbacks and use
   * them using $this->get_url('style', $command, $type, $pid).
   */
  function ajax_style() {
    $args = func_get_args();
    if (count($args) < 3) {
      return MENU_NOT_FOUND;
    }

    $command = array_shift($args);
    $type = array_shift($args);
    $pid = array_shift($args);

    $info = $this->get_style($type, $pid);

    $style = $info[0];
    $conf = &$info[1];

    if (empty($style['ajax'][$command]) || !function_exists($style['ajax'][$command])) {
      return MENU_NOT_FOUND;
    }

    // Make sure the this is always available to the called functions.
    $args = array_merge(array(&$this, $style, &$conf, $type, $pid), $args);
    return call_user_func_array($style['ajax'][$command], $args);
  }

  // ------------------------------------------------------------------------
  // AJAX command generators
  //
  // These are used to make sure that child implementations can control their
  // own AJAX commands as needed.

  /**
   * Create a command array to redraw a pane.
   */
  function command_update_pane($pid) {
    if (is_object($pid)) {
      $pane = $pid;
    }
    else {
      $pane = $this->display->content[$pid];
    }

    $this->commands[] = ctools_ajax_command_replace("#panel-pane-$pane->pid", $this->render_pane($pane));
    $this->commands[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
  }

  /**
   * Create a command array to add a new pane.
   */
  function command_add_pane($pid) {
    if (is_object($pid)) {
      $pane = $pid;
    }
    else {
      $pane = $this->display->content[$pid];
    }

    $this->commands[] = ctools_ajax_command_append("#panel-pane-$pane->panel", $this->render_pane($pane));
    $this->commands[] = ctools_ajax_command_changed("#panel-pane-$pane->pid", "div.grabber span.text");
  }

  /**
   * Create a command to update the links on a display after a change was made.
   */
  function command_update_display_links() {
    $this->commands[] = ctools_ajax_command_replace('.panels-display-links', $this->get_display_links());
  }

  /**
   * Create a command to update the links on a region after a change was made.
   */
  function command_update_region_links($id) {
    $this->commands[] = ctools_ajax_command_replace('.panels-region-links-' . $id, $this->get_region_links($id));
  }
}

/**
 * Handle the 'next' click on the add/edit pane form wizard.
 *
 * All we need to do is store the updated pane in the cache.
 */
function panels_ajax_edit_pane_next(&$form_state) {
  $form_state['cache']->new_pane = $form_state['pane'];
  panels_edit_cache_set($form_state['cache']);
}

/**
 * Handle the 'finish' click on teh add/edit pane form wizard.
 *
 * All we need to do is set a flag so the return can handle adding
 * the pane.
 */
function panels_ajax_edit_pane_finish(&$form_state) {
  $form_state['complete'] = TRUE;
  return;
}

/**
 * Handle the 'cancel' click on the add/edit pane form wizard.
 */
function panels_ajax_edit_pane_cancel(&$form_state) {
  $form_state['cancel'] = TRUE;
  return;
}

// --------------------------------------------------------------------------
// Forms for the editor object

/**
 * Choose cache method form
 */
function panels_edit_cache_method_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];

  // Set to 0 to ensure we get a selected radio.
  if (!isset($conf['method'])) {
    $conf['method'] = 0;
  }

  $caches = panels_get_caches();
  if (empty($caches)) {
    $form['markup'] = array('#value' => t('No caching options are available at this time. Please enable a panels caching module in order to use caching options.'));
    return $form;
  }

  $options[0] = t('No caching');
  foreach ($caches as $cache => $info) {
    $options[$cache] = check_plain($info['title']);
  }

  $form['method'] = array(
    '#prefix' => '<div class="no-float">',
    '#suffix' => '</div>',
    '#type' => 'radios',
    '#title' => t('Method'),
    '#options' => $options,
    '#default_value' => $conf['method'],
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  return $form;
}

/**
 * Submit callback for panels_edit_cache_method_form.
 *
 * All this needs to do is return the method.
 */
function panels_edit_cache_method_form_submit($form, &$form_state) {
  $form_state['method'] = $form_state['values']['method'];
}

/**
 * Cache settings form
 */
function panels_edit_cache_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];
  $pid = $form_state['pid'];
  $info = panels_get_cache($conf['method']);

  $form['#action'] = $form_state['url'];

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($info['description']),
  );

  $function = panels_plugin_get_function('cache', $conf['method'], 'settings form');

  $form['settings'] = $function($conf['settings'], $display, $pid);
  $form['settings']['#tree'] = TRUE;

  $form['display'] = array(
    '#type' => 'value',
    '#value' => $display,
  );

  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate cache settings.
 */
function panels_edit_cache_settings_form_validate($form, &$form_state) {
  if ($function = panels_plugin_get_function('cache', $form_state['conf']['method'], 'settings form validate')) {
    $function($form, $form_state['values']['settings']);
  }
}

/**
 * Allows panel styles to validate their style settings.
 */
function panels_edit_cache_settings_form_submit($form, &$form_state) {
  if ($function = panels_plugin_get_function('cache', $form_state['conf']['method'], 'settings form submit')) {
    $function($form_state['values']['settings']);
  }

  $form_state['conf']['settings'] = $form_state['values']['settings'];
}

/**
 * Choose style form
 */
function panels_edit_style_type_form(&$form_state) {
  $display = &$form_state['display'];
  $style = $form_state['style'];
  $type = $form_state['type'];

  $styles = panels_get_styles();

  $function = ($type == 'pane' ? 'render pane' : (variable_get('panels_legacy_rendering_mode', TRUE) ? 'render panel' : 'render region'));
  $options = array();
  if ($type == 'region') {
    $options[-1] = t('Use display default style');
  }

  uasort($styles, 'ctools_plugin_sort');

  foreach ($styles as $id => $info) {
    if (empty($info['hidden']) && (!empty($info[$function]) || $id == 'default')) {
      $options[$id] = check_plain($info['title']);
    }
  }

  $form['style'] = array(
    '#prefix' => '<div class="no-float">',
    '#suffix' => '</div>',
    '#type' => 'radios',
    '#title' => t('Style'),
    '#options' => $options,
    '#default_value' => $style,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  return $form;
}

/**
 * Submit callback for panels_edit_style_type_form.
 *
 * All this needs to do is return the method.
 */
function panels_edit_style_type_form_submit($form, &$form_state) {
  $form_state['old_style'] = $form_state['style'];
  $form_state['style'] = $form_state['values']['style'];
}

/**
 * Style settings form
 */
function panels_edit_style_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $conf = &$form_state['conf'];
  $pid = $form_state['pid'];
  $style = $form_state['style'];
  $type = $form_state['type'];

  $form['#action'] = $form_state['url'];

  $form['description'] = array(
    '#prefix' => '<div class="description">',
    '#suffix' => '</div>',
    '#value' => check_plain($style['description']),
  );

  $function = panels_plugin_get_function('styles', $style, ($type == 'pane') ? 'pane settings form' : 'settings form');

  $form['settings'] = $function($conf, $display, $pid, $type, $form_state);
  $form['settings']['#tree'] = TRUE;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Validate style settings.
 */
function panels_edit_style_settings_form_validate($form, &$form_state) {
  $name = $form_state['type'] == 'pane' ? 'pane settings form validate' : 'settings form validate';
  if ($function = panels_plugin_get_function('styles', $form_state['style'], $name)) {
    $function($form, $form_state['values']['settings'], $form_state);
  }
}

/**
 * Allows panel styles to validate their style settings.
 */
function panels_edit_style_settings_form_submit($form, &$form_state) {
  $name = $form_state['type'] == 'pane' ? 'pane settings form submit' : 'settings form submit';
  if ($function = panels_plugin_get_function('styles', $form_state['style'], $name)) {
    $function($form, $form_state['values']['settings'], $form_state);
  }

  $form_state['conf'] = $form_state['values']['settings'];
}


/**
 * Configure CSS on a pane form.
 */
function panels_edit_configure_pane_css_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];

  $form['css_id'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($pane->css['css_id']) ? $pane->css['css_id'] : '',
    '#title' => t('CSS ID'),
    '#description' => t('CSS ID to apply to this pane. This may be blank.'),
  );
  $form['css_class'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($pane->css['css_class']) ? $pane->css['css_class'] : '',
    '#title' => t('CSS class'),
    '#description' => t('CSS class to apply to this pane. This may be blank.'),
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * FAPI submission function for the CSS configure form.
 *
 * All this does is set up $pane properly. The caller is responsible for
 * actually storing this somewhere.
 */
function panels_edit_configure_pane_css_form_submit($form, &$form_state) {
  $pane = &$form_state['pane'];
  $display = $form_state['display'];

  $pane->css['css_id'] = $form_state['values']['css_id'];
  $pane->css['css_class'] = $form_state['values']['css_class'];
}

/**
 * Form to control basic visibility settings.
 */
function panels_edit_configure_access_settings_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];

  $form['logic'] = array(
    '#type' => 'radios',
    '#options' => array(
      'and' => t('All criteria must pass.'),
      'or' => t('Only one criterion must pass.'),
    ),
    '#default_value' => isset($pane->access['logic']) ? $pane->access['logic'] : 'and',
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * FAPI submission function for the edit access settings form.
 *
 * All this does is set up $pane properly. The caller is responsible for
 * actually storing this somewhere.
 */
function panels_edit_configure_access_settings_form_submit($form, &$form_state) {
  $pane = &$form_state['pane'];
  $display = $form_state['display'];

  $pane->access['logic'] = $form_state['values']['logic'];
}

/**
 * Form to add a visibility rule.
 */
function panels_edit_add_access_test_form(&$form_state) {
  $display = &$form_state['display'];
  $pane = &$form_state['pane'];

  $plugins = ctools_get_relevant_access_plugins($display->context);
  $options = array();
  foreach ($plugins as $id => $plugin) {
    $options[$id] = $plugin['title'];
  }

  asort($options);

  $form['type'] = array(
    // This ensures that the form item is added to the URL.
    '#type' => 'radios',
    '#options' => $options,
  );

  $form['next'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );

  return $form;
}

/**
 * Form to configure a visibility rule.
 */
function panels_edit_configure_access_test_form(&$form_state) {
  $display = &$form_state['display'];
  $test = &$form_state['test'];
  $plugin = &$form_state['plugin'];

  $form['#action'] = $form_state['url'];

  $contexts = $display->context;
  if (!isset($contexts['logged-in-user'])) {
    $contexts['logged-in-user'] = ctools_access_get_loggedin_context();
  }

  if (isset($plugin['required context'])) {
    $form['context'] = ctools_context_selector($contexts, $plugin['required context'], $test['context']);
  }

  $form['settings'] = array('#tree' => TRUE);
  if ($function = ctools_plugin_get_function($plugin, 'settings form')) {
    $function($form, $form_state, $test['settings']);
  }

  $form['not'] = array(
    '#type' => 'checkbox',
    '#title' => t('Reverse (NOT)'),
    '#default_value' => !empty($test['not']),
  );

  $form['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  $form['remove'] = array(
    '#type' => 'submit',
    '#value' => t('Remove'),
    '#remove' => TRUE,
  );

  return $form;
}

/**
 * Validate handler for visibility rule settings
 */
function panels_edit_configure_access_test_form_validate(&$form, &$form_state) {
  if (!empty($form_state['clicked_button']['#remove'])) {
    return;
  }

  if ($function = ctools_plugin_get_function($form_state['plugin'], 'settings form validate')) {
    $function($form, $form_state);
  }
}

/**
 * Submit handler for visibility rule settings
 */
function panels_edit_configure_access_test_form_submit(&$form, &$form_state) {
  if (!empty($form_state['clicked_button']['#remove'])) {
    $form_state['remove'] = TRUE;
    return;
  }

  if ($function = ctools_plugin_get_function($form_state['plugin'], 'settings form submit')) {
    $function($form, $form_state);
  }

  $form_state['test']['settings'] = $form_state['values']['settings'];
  if (isset($form_state['values']['context'])) {
    $form_state['test']['context'] = $form_state['values']['context'];
  }
  $form_state['test']['not'] = !empty($form_state['values']['not']);
}

