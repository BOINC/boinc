<?php

/**
 * @file
 * Contains helper functions for the Panels page wizards.
 */
function panels_page_wizard_add_layout(&$form, &$form_state) {
  $form_state['allowed_layouts'] = 'panels_page';
  $form_state['display'] = $form_state['cache']->display;

  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;

  // Change the #id of the form so the CSS applies properly.
  $form['#id'] = 'panels-choose-layout';

  $form['layout_prefix'] = array(
    '#value' => '<fieldset><legend>' . t('Layout') . '</legend>',
  );

  ctools_include('common', 'panels');
  ctools_include('display-layout', 'panels');
  ctools_include('plugins', 'panels');

  $form = array_merge($form, panels_choose_layout($form_state));

  $form['layout_suffix'] = array(
    '#value' => '</fieldset>',
  );
}

function panels_page_wizard_add_content(&$form, &$form_state) {
  ctools_include('ajax');
  ctools_include('plugins', 'panels');
  ctools_include('common', 'panels');
  ctools_include('display-edit', 'panels');

  // Panels provides this caching mechanism to make it easy to use the
  // wizard to cache the display.

  $cache = panels_edit_cache_get('panels_page_wizard:node_override');

  $form_state['renderer'] = panels_get_renderer_handler('editor', $cache->display);
  $form_state['renderer']->cache = &$cache;

  $form_state['display'] = &$cache->display;
  $form_state['content_types'] = $cache->content_types;
  // Tell the Panels form not to display buttons.
  $form_state['no buttons'] = TRUE;
  $form_state['display_title'] = !empty($cache->display_title);

  $form = array_merge($form, panels_edit_display_form($form_state));
  // Make sure the theme will work since our form id is different.
  $form['#theme'] = 'panels_edit_display_form';
}

function panels_page_wizard_add_content_submit(&$form, &$form_state) {
  // Call the normal panels edit form submit to make sure values are stored
  // on the display
  panels_edit_display_form_submit($form, $form_state);
  $cache = &$form_state['cache'];

  // Copy the "being edited" cached display to the "actual" cached display.
  $cache->display = &$form_state['display'];
  unset($cache->display_cache);
}

