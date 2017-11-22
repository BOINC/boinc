<?php

/**
 * Renderer class for all In-Place Editor (IPE) behavior.
 */
class panels_renderer_ipe extends panels_renderer_editor {
  // The IPE operates in normal render mode, not admin mode.
  var $admin = FALSE;

  // Whether or not the user has access.
  var $access = NULL;

  function invoke_panels_ipe_access() {
    if (user_access('bypass access in place editing')) {
      return TRUE;
    }
    // Modules can return TRUE, FALSE or NULL, for allowed, disallowed,
    // or don't care - respectively. On the first FALSE, we deny access,
    // otherwise allow.
    foreach (module_invoke_all('panels_ipe_access', $this->display) as $result) {
      if ($result === FALSE) {
        return FALSE;
      }
    }
    return TRUE;
  }

  function access() {
    if (is_null($this->access)) {
      $this->access = $this->invoke_panels_ipe_access();
    }
    return $this->access;
  }

  function render() {
    $output = parent::render();
    if ($this->access()) {
      return "<div id='panels-ipe-display-{$this->clean_key}' class='panels-ipe-display-container'>$output</div>";
    }
    return $output;
  }

  function add_meta() {
    if (!$this->access()) {
      return parent::add_meta();
    }

    ctools_include('display-edit', 'panels');
    ctools_include('content');

    if (empty($this->display->cache_key)) {
      $this->cache = panels_edit_cache_get_default($this->display);
    }
    // @todo we may need an else to load the cache, but I am not sure we
    // actually need to load it if we already have our cache key, and doing
    // so is a waste of resources.

    ctools_include('cleanstring');
    $this->clean_key = ctools_cleanstring($this->display->cache_key);
    panels_ipe_get_cache_key($this->clean_key);

    ctools_include('ajax');
    ctools_include('modal');
    ctools_modal_add_js();

    ctools_add_css('panels_dnd', 'panels');
    ctools_add_css('panels_admin', 'panels');
    ctools_add_js('panels_ipe', 'panels_ipe');
    ctools_add_css('panels_ipe', 'panels_ipe');

    $settings = array(
      'formPath' => url($this->get_url('save-form')),
    );
    drupal_add_js(array('PanelsIPECacheKeys' => array($this->clean_key)), 'setting');
    drupal_add_js(array('PanelsIPESettings' => array($this->clean_key => $settings)), 'setting');

    jquery_ui_add(array('ui.draggable', 'ui.droppable', 'ui.sortable'));
    parent::add_meta();
  }

  /**
   * Override & call the parent, then pass output through to the dnd wrapper
   * theme function.
   *
   * @param $pane
   */
  function render_pane(&$pane) {
    $output = parent::render_pane($pane);
    if (empty($output)) {
      return;
    }
    if (!$this->access()) {
      return $output;
    }

    if (empty($pane->IPE_empty)) {
      // Add an inner layer wrapper to the pane content before placing it into
      // draggable portlet
      $output = "<div class=\"panels-ipe-portlet-content\">$output</div>";
    }
    else {
      $output = "<div class=\"panels-ipe-portlet-content panels-ipe-empty-pane\">$output</div>";
    }
    // Hand it off to the plugin/theme for placing draggers/buttons
    $output = theme('panels_ipe_pane_wrapper', $output, $pane, $this->display, $this);
    return "<div id=\"panels-ipe-paneid-{$pane->pid}\" class=\"panels-ipe-portlet-wrapper panels-ipe-portlet-marker\">" . $output . "</div>";
  }

  function render_pane_content(&$pane) {
    $content = parent::render_pane_content($pane);
    if (!$this->access()) {
      return $output;
    }
    if (!is_object($content)) {
      $content = new StdClass();
    }
    // Ensure that empty panes have some content.
    if (empty($content->content)) {
      // Get the administrative title.
      $content_type = ctools_get_content_type($pane->type);
      $title = ctools_content_admin_title($content_type, $pane->subtype, $pane->configuration, $this->display->context);

      $content->content = t('Placeholder for empty "@title"', array('@title' => $title));
      $pane->IPE_empty = TRUE;
    }

    return $content;
  }

  /**
   * Add an 'empty' pane placeholder above all the normal panes.
   *
   * @param $region_id
   * @param $panes
   */
  function render_region($region_id, $panes) {
    if (!$this->access()) {
      return parent::render_region($region_id, $panes);
    }

    // Generate this region's 'empty' placeholder pane from the IPE plugin.
    $empty_ph = theme('panels_ipe_placeholder_pane', $region_id, $this->plugins['layout']['panels'][$region_id]);

    // Wrap the placeholder in some guaranteed markup.
    $panes['empty_placeholder'] = '<div class="panels-ipe-placeholder panels-ipe-on panels-ipe-portlet-marker panels-ipe-portlet-static">' . $empty_ph . "</div>";

    // Generate this region's add new pane button. FIXME waaaaay too hardcoded
    $panes['add_button'] = theme('panels_ipe_add_pane_button', $region_id, $this->display, $this);

    $output = parent::render_region($region_id, $panes);
    $output = theme('panels_ipe_region_wrapper', $output, $region_id, $this->display);
    $classes = 'panels-ipe-region';

    return "<div id='panels-ipe-regionid-$region_id' class='panels-ipe-region'>$output</div>";
  }

  function get_panels_storage_op_for_ajax($method) {
    switch ($method) {
      case 'ajax_unlock_ipe':
      case 'ajax_save_form':
        return 'update';
      case 'ajax_change_layout':
      case 'ajax_set_layout':
        return 'change layout';
    }

    return parent::get_panels_storage_op_for_ajax($method);
  }
 
  /**
   * AJAX entry point to create the controller form for an IPE.
   */
  function ajax_save_form($break = NULL) {
    ctools_include('form');
    if (!empty($this->cache->locked)) {
      if ($break != 'break') {
        $account  = user_load($this->cache->locked->uid);
        $name     = theme('username', $account);
        $lock_age = format_interval(time() - $this->cache->locked->updated);

        $message = t("This panel is being edited by user !user, and is therefore locked from editing by others. This lock is !age old.\n\nClick OK to break this lock and discard any changes made by !user.", array('!user' => $name, '!age' => $lock_age));

        $this->commands[] = array(
          'command' => 'unlockIPE',
          'message' => $message,
          'break_path' => url($this->get_url('save-form', 'break'))
        );
        return;
      }

      // Break the lock.
      panels_edit_cache_break_lock($this->cache);
    }

    $form_state = array(
      'display' => &$this->display,
      'content_types' => $this->cache->content_types,
      'rerender' => FALSE,
      'no_redirect' => TRUE,
      // Panels needs this to make sure that the layout gets callbacks
      'layout' => $this->plugins['layout'],
    );

    $output = ctools_build_form('panels_ipe_edit_control_form', $form_state);
    if ($output) {
      // At this point, we want to save the cache to ensure that we have a lock.
      panels_edit_cache_set($this->cache);
      $this->commands[] = array(
        'command' => 'initIPE',
        'key' => $this->clean_key,
        'data' => $output,
      );
      return;
    }

    // no output == submit
    if (!empty($form_state['clicked_button']['#save-display'])) {
      // Saved. Save the cache.
      panels_edit_cache_save($this->cache);
    }
    else {
      // Cancelled. Clear the cache.
      panels_edit_cache_clear($this->cache);
    }

    $this->commands[] = array(
      'command' => 'endIPE',
      'key' => $this->clean_key,
      'data' => $output,
    );
  }

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

    $this->commands[] = ctools_ajax_command_replace("#panels-ipe-paneid-$pane->pid", $this->render_pane($pane));
    $this->commands[] = ctools_ajax_command_changed("#panels-ipe-display-{$this->clean_key}");
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

    $this->commands[] = ctools_ajax_command_append("#panels-ipe-regionid-{$pane->panel} div.panels-ipe-sort-container", $this->render_pane($pane));
    $this->commands[] = ctools_ajax_command_changed("#panels-ipe-display-{$this->clean_key}");
  }
}

/**
 * FAPI callback to create the Save/Cancel form for the IPE.
 */
function panels_ipe_edit_control_form(&$form_state) {
  $display = &$form_state['display'];
  // @todo -- this should be unnecessary as we ensure cache_key is set in add_meta()
//  $display->cache_key = isset($display->cache_key) ? $display->cache_key : $display->did;

  // Annoyingly, theme doesn't have access to form_state so we have to do this.
  $form['#display'] = $display;

  $layout = panels_get_layout($display->layout);
  $layout_panels = panels_get_regions($layout, $display);

  $form['panel'] = array('#tree' => TRUE);
  $form['panel']['pane'] = array('#tree' => TRUE);

  foreach ($layout_panels as $panel_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($display->panels[$panel_id])) {
      $display->panels[$panel_id] = array();
    }

    $form['panel']['pane'][$panel_id] = array(
      // Use 'hidden' instead of 'value' so the js can access it.
      '#type' => 'hidden',
      '#default_value' => implode(',', (array) $display->panels[$panel_id]),
    );
  }

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#id' => 'panels-ipe-save',
    '#submit' => array('panels_edit_display_form_submit'),
    '#save-display' => TRUE,
  );
  $form['buttons']['cancel'] = array(
    '#type' => 'submit',
    '#value' => t('Cancel'),
    '#id' => 'panels-ipe-cancel',
  );
  return $form;
}
