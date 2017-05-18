<?php

/*
 * @file
 * Core Panels API include file containing various display-editing functions.
 * This includes all the basic editing forms (content, layout, layout settings)
 * as well as the ajax modal forms associated with them.
 */


/**
 * Handle calling and processing of the form for editing display content.
 *
 * Helper function for panels_edit().
 *
 * @see panels_edit() for details on the various behaviors of this function.
 */
function _panels_edit($display, $destination, $content_types, $title = FALSE) {
  $did = $display->did;
  if (!$did) {
    $display->did = $did = 'new';
  }

  // Load the display being edited from cache, if possible.
  if (!empty($_POST) && is_object($cache = panels_edit_cache_get($did))) {
    $display = $cache->display;
  }
  else {
    $cache = panels_edit_cache_get_default($display, $content_types, $title);
  }

  // Get a renderer.
  $renderer = panels_get_renderer_handler('editor', $display);
  $renderer->cache = $cache;

  $output = $renderer->edit();
  if (is_object($output) && $destination) {
    return panels_goto($destination);
  }
  return $output;
}

/**
 * Form definition for the panels display editor
 *
 * No validation function is necessary, as all 'validation' is handled
 * either in the lead-up to form rendering (through the selection of
 * specified content types) or by the validation functions specific to
 * the ajax modals & content types.
 *
 * @ingroup forms
 * @see panels_edit_display_submit()
 */
function panels_edit_display_form(&$form_state) {
  $display = &$form_state['display'];
  $renderer = &$form_state['renderer'];

  // Make sure there is a valid cache key.
  $cache_key = isset($display->cache_key) ? $display->cache_key : $display->did;
  $display->cache_key = $cache_key;

  // Annoyingly, theme doesn't have access to form_state so we have to do this.
  $form['#display'] = $display;

  // The flexible layout maker wants to be able to edit a display without
  // actually editing a display, so we provide this 'setting' to allow
  // that to go away.
  if (empty($form_state['no display settings'])) {
    $links = $renderer->get_display_links();
  }
  else {
    $links = '';
  }
  $form['hide']['display-settings'] = array(
    '#value' => $links,
  );

  $form += panels_edit_display_settings_form($form_state);

  $form['panel'] = array('#tree' => TRUE);
  $form['panel']['pane'] = array('#tree' => TRUE);

  $form['display'] = array(
    '#value' => $renderer->render(),
  );

  foreach ($renderer->plugins['layout']['panels'] as $region_id => $title) {
    // Make sure we at least have an empty array for all possible locations.
    if (!isset($display->panels[$region_id])) {
      $display->panels[$region_id] = array();
    }

    $form['panel']['pane'][$region_id] = array(
      // Use 'hidden' instead of 'value' so the js can access it.
      '#type' => 'hidden',
      '#default_value' => implode(',', (array) $display->panels[$region_id]),
    );
  }

  if (empty($form_state['no buttons'])) {
    $form['buttons']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#id' => 'panels-dnd-save',
      '#submit' => array('panels_edit_display_form_submit'),
      '#save-display' => TRUE,
    );
    $form['buttons']['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
    );
  }

  // Build up the preview portion of the form, if necessary.
  if (empty($form_state['no preview'])) {
    $form['preview'] = array(
      '#tree' => TRUE,
      '#prefix' => '<h2>' . t('Live preview') . '</h2>' . '<div id="panels-live-preview">',
      '#suffix' => '</div>',
    );

    ctools_context_replace_form($form['preview'], $display->context);
    $form['preview']['button'] = array(
      '#type' => 'submit',
      '#value' => t('Preview'),
      '#attributes' => array('class' => 'ctools-use-ajax'),
      '#id' => 'panels-live-preview-button',
      '#submit' => array('panels_edit_display_form_submit', 'panels_edit_display_form_preview'),
    );
  }

  return $form;
}

/**
 * Handle form submission of the display content editor.
 *
 * This reads the location of the various panes from the form, which will
 * have been modified from the ajax, rearranges them and then saves
 * the display.
 */
function panels_edit_display_form_submit($form, &$form_state) {
  $display = &$form_state['display'];

  $old_content = $display->content;
  $display->content = array();

  if (!empty($form_state['values']['panel']['pane'])) {
    foreach ($form_state['values']['panel']['pane'] as $panel_id => $panes) {
      $display->panels[$panel_id] = array();
      if ($panes) {
        $pids = explode(',', $panes);
        // need to filter the array, b/c passing it in a hidden field can generate trash
        foreach (array_filter($pids) as $pid) {
          if ($old_content[$pid]) {
            $display->panels[$panel_id][] = $pid;
            $old_content[$pid]->panel = $panel_id;
            $display->content[$pid] = $old_content[$pid];
          }
        }
      }
    }
  }

  panels_edit_display_settings_form_submit($form, $form_state);
}

/**
 * Submission of the preview button. Render the preview and put it into
 * the preview widget area.
 */
function panels_edit_display_form_preview(&$form, &$form_state) {
  $display = &$form_state['display'];
  ctools_include('ajax');

  $display->context = ctools_context_replace_placeholders($display->context, $form_state['values']['preview']);
  $display->skip_cache = TRUE;
  $output = panels_render_display($display);

  // Add any extra CSS that some layouts may have added specifically for this.
  if (!empty($display->add_css)) {
    $output = "<style type=\"text/css\">\n$display->add_css</style>\n" . $output;
  }

  $commands = array();
  $commands[] = array(
    'command' => 'panel_preview',
    'output' => $output,
  );

  ctools_ajax_render($commands);
}


/**
 * Form for display settings.
 */
function panels_edit_display_settings_form(&$form_state) {
  $form = array();
  $display = &$form_state['display'];

  $layout = panels_get_layout($display->layout);
  $form_state['layout'] = $layout;

  ctools_include('dependent');

  if ($form_state['display_title']) {
    $form['display_title'] = array (
      '#tree' => TRUE,
    );

    $form['display_title']['hide_title'] = array(
      '#type' => 'select',
      '#title' => t('Title type'),
      '#default_value' => (int) $display->hide_title,
      '#options' => array(
        PANELS_TITLE_NONE => t('No title'),
        PANELS_TITLE_FIXED => t('Manually set'),
        PANELS_TITLE_PANE => t('From pane'),
      ),
    );

    $form['display_title']['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $display->title,
      '#title' => t('Title'),
      '#description' => t('The title of this panel. If left blank, a default title may be used. If you want the title actually to be blank, change the "Title type" dropdown from "Manually Set" to "No Title".'),
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array('edit-display-title-hide-title' => array(PANELS_TITLE_FIXED)),
    );

    if (!empty($display->context)) {
      $form['display_title']['title']['#description'] .= ' ' . t('You may use substitutions in this title.');

      // We have to create a manual fieldset because fieldsets do not support IDs.
      // Use 'hidden' instead of 'markup' so that the process will run.
      // Add js for collapsible fieldsets manually
      drupal_add_js('misc/collapse.js');
      $form['display_title']['contexts_prefix'] = array(
        '#type' => 'hidden',
        '#id' => 'edit-display-substitutions',
        '#prefix' => '<div><fieldset id="edit-display-substitutions" class="collapsed collapsible"><legend>' . t('Substitutions') . '</legend>',
        '#process' => array('ctools_dependent_process'),
        '#dependency' => array('edit-display-title-hide-title' => array(PANELS_TITLE_FIXED)),
      );

      $rows = array();
      foreach ($display->context as $context) {
        foreach (ctools_context_get_converters('%' . check_plain($context->keyword) . ':', $context) as $keyword => $title) {
          $rows[] = array(
            check_plain($keyword),
            t('@identifier: @title', array('@title' => $title, '@identifier' => $context->identifier)),
          );
        }
      }

      $header = array(t('Keyword'), t('Value'));
      $form['display_title']['contexts'] = array(
        '#value' => theme('table', $header, $rows),
      );
      $form['display_title']['contexts_suffix'] = array(
        '#value' => '</fieldset></div>',
      );
    }
  }

  // TODO doc the ability to do this as part of the API
  if (!empty($layout['settings form']) && function_exists($layout['settings form'])) {
    $form['layout_settings'] = $layout['settings form']($display, $layout, $display->layout_settings);
  }
  $form['layout_settings']['#tree'] = TRUE;

  return $form;
}

/**
 * Validate the layout settings form.
 */
function panels_edit_display_settings_form_validate($form, &$form_state) {
  if ($function = panels_plugin_get_function('layout', $form_state['layout'], 'settings validate')) {
    $function($form_state['values']['layout_settings'], $form['layout_settings'], $form_state['display'], $form_state['layout'], $form_state['display']->layout_settings);
  }
}

/**
 * Store changes from the layout settings form.
 */
function panels_edit_display_settings_form_submit($form, &$form_state) {
  $display = &$form_state['display'];
  if ($function = panels_plugin_get_function('layout', $form_state['layout'], 'settings submit')) {
    $function($form_state['values']['layout_settings'], $display, $form_state['layout'], $display->layout_settings);
  }

  // Since not all layouts have layout settings, check here in case of notices.
  if (isset($form_state['values']['layout_settings'])) {
    $display->layout_settings = $form_state['values']['layout_settings'];
  }

  if (isset($form_state['values']['display_title']['title'])) {
    $display->title = $form_state['values']['display_title']['title'];
    $display->hide_title = $form_state['values']['display_title']['hide_title'];
  }
}
