<?php
// $Id: context_export_ui.class.php,v 1.1.2.5 2010/08/11 15:57:28 yhahn Exp $

/**
 * CTools export UI extending class. Slightly customized for Context.
 */
class context_export_ui extends ctools_export_ui {
  function list_form(&$form, &$form_state) {
    parent::list_form($form, $form_state);
    $form['top row']['submit'] = $form['bottom row']['submit'];
    $form['top row']['reset'] = $form['bottom row']['reset'];
    $form['bottom row']['#access'] = FALSE;
    // Invalidate the context cache.
    context_invalidate_cache();
    return;
  }

  function list_css() {
    ctools_add_css('export-ui-list');
    drupal_add_css(drupal_get_path("module", "context_ui") ."/context_ui.css");
  }

  function list_render(&$form_state) {
    return theme('table', $this->list_table_header(), $this->rows, array('class' => 'context-admin', 'id' => 'ctools-export-ui-list-items'));
  }

  function list_build_row($item, &$form_state, $operations) {
    $name = $item->name;

    // Add a row for tags.
    $tag = !empty($item->tag) ? $item->tag : t('< Untagged >');
    if (!isset($this->rows[$tag])) {
      $this->rows[$tag]['data'] = array();
      $this->rows[$tag]['data'][] = array('data' => check_plain($tag), 'colspan' => 3, 'class' => 'tag');
      $this->sorts["{$tag}"] = $tag;
    }

    // Build row for each context item.
    $this->rows["{$tag}:{$name}"]['data'] = array();
    $this->rows["{$tag}:{$name}"]['class'] = !empty($item->disabled) ? 'ctools-export-ui-disabled' : 'ctools-export-ui-enabled';
    $this->rows["{$tag}:{$name}"]['data'][] = array(
      'data' => check_plain($name) . "<div class='description'>" . check_plain($item->description) . "</div>",
      'class' => 'ctools-export-ui-name'
    );
    $this->rows["{$tag}:{$name}"]['data'][] = array(
      'data' => check_plain($item->type),
      'class' => 'ctools-export-ui-storage'
    );
    $this->rows["{$tag}:{$name}"]['data'][] = array(
      'data' => theme('links', $operations, array('class' => 'links inline')),
      'class' => 'ctools-export-ui-operations'
    );

    // Sort by tag, name.
    $this->sorts["{$tag}:{$name}"] = $tag . $name;
  }

  /**
   * Override of edit_form_submit().
   * Don't copy values from $form_state['values'].
   */
  function edit_form_submit(&$form, &$form_state) {
    if (!empty($this->plugin['form']['submit'])) {
      $this->plugin['form']['submit']($form, $form_state);
    }
  }
}


/**
 * Generates the omnibus context definition editing form.
 *
 * @param $op
 *   The type of form to build. Either "add", "view" or "edit"
 * @param $cid
 *   The db context identifier - required when $op == "edit"
 *
 * @return
 *   A Drupal form array.
 */
function context_ui_form(&$form, &$form_state) {
  $context = $form_state['item'];
  $form['#base'] = 'context_ui_form';
  $form['#theme'] = 'context_ui_form';

  // Core context definition
  $form['info']['#type'] = 'fieldset';
  $form['info']['#tree'] = FALSE;

  // Swap out name validator. Allow dashes.
  if (isset($form['info']['name']['#element_validate'])) {
    $form['info']['name']['#element_validate'] = array('context_ui_edit_name_validate');
  }

  $form['info']['tag'] = array(
    '#title' => t('Tag'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#maxlength' => 255,
    '#default_value' => isset($context->tag) ? $context->tag : '',
    '#description' => t('Example: <code>theme</code>') .'<br/>'. t('A tag to group this context with others.'),
  );

  $form['info']['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#required' => FALSE,
    '#maxlength' => 255,
    '#default_value' => isset($context->description) ? $context->description: '',
    '#description' => t('The description of this context definition.'),
  );

  // Condition mode
  $form['condition_mode'] = array(
    '#type' => 'checkbox',
    '#default_value' => isset($context->condition_mode) ? $context->condition_mode : FALSE,
    '#title' => t('Require all conditions'),
    '#description' => t('If checked, all conditions must be met for this context to be active. Otherwise, the first condition that is met will activate this context.')
  );

  // Condition plugin forms
  $form['conditions'] = array(
    '#theme' => 'context_ui_plugins',
    '#title' => t('Conditions'),
    '#description' => t('Trigger the activation of this context'),
    '#tree' => TRUE,
    'selector' => array(
      '#type' => 'select',
      '#options' => array(0 => '<'. t('Add a condition') .'>'),
      '#default_value' => 0,
    ),
    'state' => array(
      '#attributes' => array('class' => 'context-plugins-state'),
      '#type' => 'hidden',
    ),
    'plugins' => array('#tree' => TRUE),
  );
  $conditions = array_keys(context_conditions());
  sort($conditions);
  foreach ($conditions as $condition) {
    if ($plugin = context_get_plugin('condition', $condition)) {
      $form['conditions']['plugins'][$condition] = array(
        '#tree' => TRUE,
        '#plugin' => $plugin,
        '#context_enabled' => isset($context->conditions[$condition]), // This flag is used at the theme layer.
        'values' => $plugin->condition_form($context),
        'options' => $plugin->options_form($context),
      );
      $form['conditions']['selector']['#options'][$condition] = $plugin->title;
    }
  }

  // Reaction plugin forms
  $form['reactions'] = array(
    '#theme' => 'context_ui_plugins',
    '#title' => t('Reactions'),
    '#description' => t('Actions to take when this context is active'),
    '#tree' => TRUE,
    'selector' => array(
      '#type' => 'select',
      '#options' => array(0 => '<'. t('Add a reaction') .'>'),
      '#default_value' => 0,
    ),
    'state' => array(
      '#attributes' => array('class' => 'context-plugins-state'),
      '#type' => 'hidden',
    ),
    'plugins' => array('#tree' => TRUE),
  );
  $reactions = array_keys(context_reactions());
  sort($reactions);
  foreach ($reactions as $reaction) {
    if ($plugin = context_get_plugin('reaction', $reaction)) {
      $form['reactions']['plugins'][$reaction] = $plugin->options_form($context) + array(
        '#plugin' => $plugin,
        '#context_enabled' => isset($context->reactions[$reaction]), // This flag is used at the theme layer.
      );
      $form['reactions']['selector']['#options'][$reaction] = $plugin->title;
    }
  }
  return $form;
}

/**
 * Modifies a context object from submitted form values.
 *
 * @param $context
 *   The context object to modify.
 * @param $form
 *   A form array with submitted values
 *
 * @return
 *   A context object
 */
function context_ui_form_process($context, $form) {
  $context->name = isset($form['name']) ? $form['name'] : NULL;
  $context->description = isset($form['description']) ? $form['description'] : NULL;
  $context->tag = isset($form['tag']) ? $form['tag'] : NULL;
  $context->condition_mode = isset($form['condition_mode']) ? $form['condition_mode'] : NULL;
  $context->conditions = array();
  $context->reactions = array();
  if (!empty($form['conditions'])) {
    $enabled = explode(',', $form['conditions']['state']);
    foreach ($form['conditions']['plugins'] as $condition => $values) {
      if (in_array($condition, $enabled, TRUE) && ($plugin = context_get_plugin('condition', $condition))) {
        if (isset($values['values'])) {
          $context->conditions[$condition]['values'] = $plugin->condition_form_submit($values['values']);
        }
        if (isset($values['options'])) {
          $context->conditions[$condition]['options'] = $plugin->options_form_submit($values['options']);
        }
        if (context_empty($context->conditions[$condition]['values'])) {
          unset($context->conditions[$condition]);
        }
      }
    }
  }
  if (!empty($form['reactions'])) {
    $enabled = explode(',', $form['reactions']['state']);
    foreach ($form['reactions']['plugins'] as $reaction => $values) {
      if (in_array($reaction, $enabled, TRUE) && ($plugin = context_get_plugin('reaction', $reaction))) {
        if (isset($values)) {
          $context->reactions[$reaction] = $plugin->options_form_submit($values);
        }
        if (context_empty($context->reactions[$reaction])) {
          unset($context->reactions[$reaction]);
        }
      }
    }
  }
  return $context;
}

/**
 * Submit handler for main context_ui form.
 */
function context_ui_form_submit($form, &$form_state) {
  $form_state['item'] = context_ui_form_process($form_state['item'], $form_state['values']);
}

/**
 * Replacement for ctools_export_ui_edit_name_validate(). Allow dashes.
 */
function context_ui_edit_name_validate($element, &$form_state) {
  $plugin = $form_state['plugin'];
  // Check for string identifier sanity
  if (!preg_match('!^[a-z0-9_-]+$!', $element['#value'])) {
    form_error($element, t('The export id can only consist of lowercase letters, underscores, dashes, and numbers.'));
    return;
  }

  // Check for name collision
  if (empty($form_state['item']->export_ui_allow_overwrite) && $exists = ctools_export_crud_load($plugin['schema'], $element['#value'])) {
    form_error($element, t('A @plugin with this name already exists. Please choose another name or delete the existing item before creating a new one.', array('@plugin' => $plugin['title singular'])));
  }
}
