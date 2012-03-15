<?php
// $Id: context_reaction_theme.inc,v 1.1.2.3 2010/08/09 15:13:03 yhahn Exp $

/**
 * Expose themes as context reactions.
 */
class context_reaction_theme extends context_reaction {
  /**
   * Editor form.
   */
  function editor_form($context) {
    $form = $this->options_form($context);

    // Hide descriptions which take up too much space.
    unset($form['title']['#description']);
    unset($form['subtitle']['#description']);
    unset($form['class']['#description']);
    return $form;
  }

  /**
   * Submit handler for editor form.
   */
  function editor_form_submit($context, $values) {
    return $values;
  }

  /**
   * Allow admins to provide a section title, section subtitle and section class.
   */
  function options_form($context) {
    $values = $this->fetch_from_context($context);
    $form = array(
      '#tree' => TRUE,
      '#title' => t('Theme variables'),
      'title' => array(
        '#title' => t('Section title'),
        '#description' => t('Provides this text as a <strong>$section_title</strong> variable for display in page.tpl.php when this context is active.'),
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#default_value' => isset($values['title']) ? $values['title'] : '',
      ),
      'subtitle' => array(
        '#title' => t('Section subtitle'),
        '#description' => t('Provides this text as a <strong>$section_subtitle</strong> variable for display in page.tpl.php when this context is active.'),
        '#type' => 'textfield',
        '#maxlength' => 255,
        '#default_value' => isset($values['subtitle']) ? $values['subtitle'] : '',
      ),
      'class' => array(
        '#title' => t('Section class'),
        '#description' => t('Provides this text as an additional body class (in <strong>$body_classes</strong> in page.tpl.php) when this section is active.'),
        '#type' => 'textfield',
        '#maxlength' => 64,
        '#default_value' => isset($values['class']) ? $values['class'] : '',
      ),
    );
    return $form;
  }

  /**
   * Set 'section_title', and 'section_subtitle' if not set and merge all
   * additional classes onto the 'body_classes'.
   */
  function execute(&$vars) {
    $classes = array();
    foreach ($this->get_contexts() as $k => $v) {
      if (!empty($v->reactions[$this->plugin]['title']) && !isset($vars['section_title'])) {
        $vars['section_title'] = t($v->reactions[$this->plugin]['title']);
      }
      if (!empty($v->reactions[$this->plugin]['subtitle']) && !isset($vars['section_subtitle'])) {
        $vars['section_subtitle'] = t($v->reactions[$this->plugin]['subtitle']);
      }
      if (!empty($v->reactions[$this->plugin]['class'])) {
        $classes[$v->reactions[$this->plugin]['class']] = $v->reactions[$this->plugin]['class'];
      }
    }
    $vars['body_classes'] .= !empty($classes) ? ' '. implode(' ', $classes) : '';
  }
}
