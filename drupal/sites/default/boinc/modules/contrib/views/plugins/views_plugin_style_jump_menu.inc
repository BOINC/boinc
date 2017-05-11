<?php
/**
 * @file
 * Contains the table style plugin.
 */

/**
 * Style plugin to render each item as a row in a table.
 *
 * @ingroup views_style_plugins
 */
class views_plugin_style_jump_menu extends views_plugin_style {
  function option_definition() {
    $options = parent::option_definition();

    $options['hide'] = array('default' => FALSE);
    $options['path'] = array('default' => '');
    $options['text'] = array('default' => 'Go', 'translatable' => TRUE);
    $options['choose'] = array('default' => '- Choose -', 'translatable' => TRUE);
    $options['default_value'] = array('default' => FALSE);

    return $options;
  }

  /**
   * Render the given style.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      $form['error_markup'] = array(
        '#value' => t('You need at least one field before you can configure your jump menu settings'),
        '#prefix' => '<div class="error form-item description">',
        '#suffix' => '</div>',
      );
      return;
    }

    $form['markup'] = array(
      '#value' => t('To properly configure a jump menu, you must select one field that will represent the path to utilize. You should then set that field to exclude. All other displayed fields will be part of the menu. Please note that all HTML will be stripped from this output as select boxes cannot show HTML.'),
      '#prefix' => '<div class="form-item description">',
      '#suffix' => '</div>',
    );

    foreach ($handlers as $id => $handler) {
      $options[$id] = $handler->ui_name();
    }

    $form['path'] = array(
      '#type' => 'select',
      '#title' => t('Path field'),
      '#options' => $options,
      '#default_value' => $this->options['path'],
    );

    $form['hide'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide the "Go" button.'),
      '#default_value' => !empty($this->options['hide']),
      '#description' => t('If hidden, this button will only be hidden for users with javascript and the page will automatically jump when the select is changed.'),
    );

    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Button text'),
      '#default_value' => $this->options['text'],
    );

    $form['choose'] = array(
      '#type' => 'textfield',
      '#title' => t('Choose text'),
      '#default_value' => $this->options['choose'],
      '#description' => t('The text that will appear as the selected option in the jump menu.'),
    );

    $form['default_value'] = array(
      '#type' => 'checkbox',
      '#title' => t('Select the current argument.'),
      '#default_value' => !empty($this->options['default_value']),
      '#description' => t('If checked, the current path will be displayed as the default option in the jump menu, if applicable.'),
    );
  }

  /**
   * Render the display in this style.
   *
   * This is overridden so that we can render our grouping specially.
   */
  function render() {
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);

    // Turn this all into an $options array for the jump menu.
    $this->view->row_index = 0;
    $options = array();
    $paths = array();

    foreach ($sets as $title => $records) {
      foreach ($records as $row_index => $row) {
        $this->view->row_index = $row_index;
        $path = html_entity_decode(strip_tags($this->get_field($this->view->row_index, $this->options['path'])), ENT_QUOTES);
        // Putting a '/' in front messes up url() so let's take that out
        // so users don't shoot themselves in the foot.
        if (strpos($path, '/') === 0) {
          $path = substr($path, 1);
        }

        // use parse_url() to preserve query and fragment in case the user
        // wants to do fun tricks.
        $url = parse_url($path);
        $url_options = array();
        if (isset($url['query'])) {
          $path = strtr($path, array('?' . $url['query'] => ''));
          $url_options['query'] = $url['query'];
        }
        if (isset($url['fragment'])) {
          $path = strtr($path, array('#' . $url['fragment'] => ''));
          $url_options['fragment'] = $url['fragment'];
        }

        $path = url($path, $url_options);
        $field = html_entity_decode(strip_tags($this->row_plugin->render($row)), ENT_QUOTES);
        if ($title) {
          $options[$title][$path] = $field;
        }
        else {
          $options[$path] = $field;
        }
        $paths[$path] = $path;
        $this->view->row_index++;
      }
    }
    unset($this->view->row_index);

    $default_value = '';
    if ($this->options['default_value'] && !empty($paths[url($_GET['q'])])) {
      $default_value = url($_GET['q']);
    }

    ctools_include('jump-menu');
    $settings = array(
      'hide' => $this->options['hide'],
      'button' => $this->options['text'],
      'choose' => $this->options['choose'],
      'default_value' => $default_value,
    );

    return drupal_get_form('ctools_jump_menu', $options, $settings);
  }

  function render_set($title, $records) {
    $options = array();
    $fields = $this->rendered_fields;
  }
}
