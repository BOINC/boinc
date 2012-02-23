<?php
// $Id: flag_handler_relationships.inc,v 1.1.2.9 2009/04/26 21:35:12 quicksketch Exp $

/**
 * @file
 * Contains various relationship handlers.
 */

/**
 * Specialized relationship handler associating flags and content.
 *
 * @ingroup views
 */
class flag_handler_relationship_content extends views_handler_relationship {

  function option_definition() {
    $options = parent::option_definition();

    $content_type = $this->definition['flag type'];
    $options['flag'] = array('default' => flag_views_flag_default($content_type));
    $options['user_scope'] = array('default' => 'current');
    $options['required'] = array('default' => 1);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $content_type = $this->definition['flag type'];
    $form['label']['#description'] .= ' '. t('The name of the selected flag makes a good label.');
    $form['flag'] = flag_views_flag_config_form('radios', $content_type, $this->options['flag']);

    $form['user_scope'] = array(
      '#type' => 'radios',
      '#title' => t('By'),
      '#options' => array('current' => t('Current user'), 'any' => t('Any user')),
      '#default_value' => $this->options['user_scope'],
    );

    $form['required']['#title'] = t('Include only flagged content');
    $form['required']['#description'] = t('If checked, only content that has this flag will be included. Leave unchecked to include all content; or, in combination with the <em>Flagged</em> filter, <a href="@unflagged-url">to limit the results to specifically unflagged content</a>.', array('@unflagged-url' => 'http://drupal.org/node/299335'));

    if (!$form['flag']['#options']) {
      $form = array(
        'error' => array(
          '#value' => '<p class="error">' . t('No %type flags exist. You must first <a href="@create-url">create a %type flag</a> before being able to use this relationship type.', array('%type' => $content_type, '@create-url' => url('admin/build/flags'))) . '</p>',
        ),
      );
      $form_state['no flags exist'] = TRUE;
    }
  }

  function options_validate($form, &$form_state) {
    if (!empty($form_state['no flags exist'])) {
      form_error($form, t('You must first create a flag'));
    }
  }

  function admin_summary() {
    return $this->options['user_scope'] == 'current' ? t('by current user') : t('by any user');
  }

  function ui_name() {
    // We put the bookmark name in the UI string to save space.
    return t('!group: !title', array('!group' => $this->definition['group'], '!title' => $this->options['flag']));
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    $flag = flag_get_flag($this->options['flag']);
    $this->definition['extra'][] = array(
      'field' => 'fid',
      'value' => $flag->fid,
      'numeric' => TRUE,
    );
    if ($this->options['user_scope'] == 'current' && !$flag->global) {
      $this->definition['extra'][] = array(
        'field' => 'uid',
        'value' => '***CURRENT_USER***',
        'numeric' => TRUE,
      );
    }
    parent::query();
  }
}

/**
 * Specialized relationship handler associating flag counts and content.
 *
 * @ingroup views
 */
class flag_handler_relationship_counts extends views_handler_relationship {

  function option_definition() {
    $options = parent::option_definition();

    $content_type = $this->definition['flag type'];
    $options['flag'] = array('default' => flag_views_flag_default($content_type));
    $options['required'] = array('default' => 1);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $content_type = $this->definition['flag type'];
    $form['flag'] = flag_views_flag_config_form('radios', $content_type, $this->options['flag']);

    $form['required']['#title'] = t('Include only flagged content');
    $form['required']['#description'] = t('If checked, only content that is flagged will be included.');
  }

  function admin_summary() {
    // Nothing to show.
  }

  function ui_name() {
    // We put the bookmark name in the UI string to save space.
    return t('!group: !title counter', array('!group' => $this->definition['group'], '!title' => $this->options['flag']));
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    $flag = flag_get_flag($this->options['flag']);
    $this->definition['extra'][] = array(
      'field' => 'fid',
      'value' => $flag->fid,
      'numeric' => TRUE,
    );
    if (!empty($this->options['required'])) {
      // Unfortunately, we may have zeros in our table, so having
      // parent::query() do INNER JOIN doesn't suffice. We need to filter these
      // zeros out.
      // @todo Make sure zero records aren't written in the first place, and
      // remove this code.
      $this->definition['extra'][] = array(
        'field' => 'count',
        'operator' => '>',
        'value' => '0',
        'numeric' => TRUE,
      );
    }
    parent::query();
  }
}

/**
 * Specialized relationship handler associating flags and users.
 *
 * @ingroup views
 */
class flag_handler_relationship_user_content extends views_handler_relationship {

  function option_definition() {
    $options = parent::option_definition();
    $options['flag'] = array('default' => flag_views_flag_default(NULL));
    $options['required'] = array('default' => 1);
    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['label']['#description'] .= ' '. t('Including name of the selected flag helps identify this relationship.');

    $form['flag'] = flag_views_flag_config_form('radios', NULL, $this->options['flag']);
    $form['flag']['#title'] = t('Flagged');

    $form['required']['#title'] = t('Include only users who have flagged content.');
    $form['required']['#description'] = t('If checked, only users that have flagged any content with this flag will be included.');
  }

  function admin_summary() {
    return $this->options['flag'];
  }

  /**
   * Called to implement a relationship in a query.
   */
  function query() {
    $flag = flag_get_flag($this->options['flag']);
    $this->definition['extra'][] = array(
      'field' => 'fid',
      'value' => $flag->fid,
      'numeric' => TRUE,
    );
    parent::query();
  }
}

