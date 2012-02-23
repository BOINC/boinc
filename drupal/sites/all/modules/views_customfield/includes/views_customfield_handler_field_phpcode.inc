<?php
// $Id: views_customfield_handler_field_phpcode.inc,v 1.1.2.3 2009/03/14 23:14:03 casey Exp $

/**
 * @file
 * Contains the 'customfield' phpcode field handler.
 */

/**
 * Field handler to display row-specific custom text using PHP.
 */
class views_customfield_handler_field_phpcode extends views_handler_field {
  var $static;
  
  function query() {
    $this->field_alias = 'customfield_phpcode_'. $this->position;
  }
  
  function option_definition() {
    $options = parent::option_definition();
    $options['value'] = array('default' => '');
    return $options;
  }

  /**
   * Provide link to node option
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $form['value'] = array(
      '#type' => 'textarea',
      '#title' => t('Value'),
      '#default_value' => $this->options['value'],
      '#rows' => 5,
      '#description' => 
        t('The text that should be displayed.')
        .' '.t('Include &lt;?php ?&gt; delimiters when using PHP code.')
        .' '.t('Available variables:').'<br/>'
        .t('$data: contains the retrieved record from the database (e.g. $data->nid).').'<br/>'
        .t('$static: can be used to store reusable data per row.')
      ,
    );
  }

  /**
   * Enter description here...
   *
   * @param stdClass $values
   * @return string
   */
  function render($values) {
    $d = $this->view->display[$this->view->current_display];
    $field_handlers = $d->handler->handlers['field'];
    foreach ($field_handlers as $h) {
      // Handle grouped fields
      if (is_a($h, 'content_handler_field_multiple') && $h->defer_query) {
        // content_handler_field_multiple stores its query results in $obj->field_values
        $tmp = array();
        if(!empty($h->field_values[$values->{$h->field_alias}])) {
          foreach ($h->field_values[$values->{$h->field_alias}] as $k => $v) {
            $tmp[$k] = $v;
            // clean up a bit
            unset($tmp[$k]['#delta']);
            unset($tmp[$k]['_nid']);
          }
        }
        $values->{$h->table.'_'.$h->field} = $tmp;
      }
    }
    
    return $this->render_phpcode($this->options['value'], $this->static, $values);
  }

  /**
   * Prepare and evaluate of a string of PHP code.
   * 
   * This method and eval_phpcode are mostly a copy of drupal_eval().
   *
   * @param string $code
   * @param mixed $static
   * @param array $data
   * @return string
   */
  function render_phpcode($code, &$static, $data) {
    global $theme_path, $theme_info, $conf;
  
    // Store current theme path.
    $old_theme_path = $theme_path;
  
    // Restore theme_path to the theme, as long as drupal_eval() executes,
    // so code evaluted will not see the caller module as the current theme.
    // If theme info is not initialized get the path from theme_default.
    if (!isset($theme_info)) {
      $theme_path = drupal_get_path('theme', $conf['theme_default']);
    }
    else {
      $theme_path = dirname($theme_info->filename);
    }
  
    $output = $this->eval_phpcode($code, $static, $data);
  
    // Recover original theme path.
    $theme_path = $old_theme_path;
  
    return $output;
  }
  
  /**
   * Evaluate a string of PHP code.
   *
   * @param string $code
   * @param mixed $static
   * @param array $data
   * @return string
   */
  function eval_phpcode($code, &$static, $data) {
    ob_start();
    print eval('?>'. $code);
    return ob_get_clean();
  }
}
