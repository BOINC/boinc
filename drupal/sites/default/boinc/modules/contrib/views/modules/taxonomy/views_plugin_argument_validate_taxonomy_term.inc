<?php
/**
 * @file
 * Contains the 'taxonomy term' argument validator plugin.
 */

/**
 * Validate whether an argument is an acceptable node.
 */
class views_plugin_argument_validate_taxonomy_term extends views_plugin_argument_validate {
  function validate_form(&$form, &$form_state) {
    $vocabularies = taxonomy_get_vocabularies();
    $options = array();
    foreach ($vocabularies as $voc) {
      $options[$voc->vid] = check_plain($voc->name);
    }

    $form['validate_argument_vocabulary'] = array(
      '#type' => 'checkboxes',
      '#prefix' => '<div id="edit-options-validate-argument-vocabulary-wrapper">',
      '#suffix' => '</div>',
      '#title' => t('Vocabularies'),
      '#options' => $options,
      '#default_value' => isset($this->argument->options['validate_argument_vocabulary']) ? $this->argument->options['validate_argument_vocabulary'] : array(),
      '#description' => t('If you wish to validate for specific vocabularies, check them; if none are checked, all terms will pass.'),
      '#process' => array('expand_checkboxes', 'views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_type'] = array(
      '#type' => 'select',
      '#title' => t('Argument type'),
      '#options' => array(
        'tid' => t('Term ID'),
        'tids' => t('Term IDs separated by , or +'),
        'name' => t('Term name or synonym'),
        'convert' => t('Term name/synonym converted to Term ID'),
      ),
      '#default_value' => isset($this->argument->options['validate_argument_type']) ? $this->argument->options['validate_argument_type'] : 'tid',
      '#description' => t('Select the form of this argument; if using term name, it is generally more efficient to convert it to a term ID and use Taxonomy: Term ID rather than Taxonomy: Term Name" as an argument.'),
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-type' => array($this->id)),
    );

    $form['validate_argument_transform'] = array(
      '#type' => 'checkbox',
      '#title' => t('Transform dashes in URL to spaces in term name arguments'),
      '#default_value' => isset($this->argument->options['validate_argument_transform']) ? $this->argument->options['validate_argument_transform'] : FALSE,
      '#process' => array('views_process_dependency'),
      '#dependency' => array('edit-options-validate-argument-type' => array('convert', 'name')),
    );
  }

  function validate_argument($argument) {
    $vids = isset($this->argument->options['validate_argument_vocabulary']) ? array_filter($this->argument->options['validate_argument_vocabulary']) : array();
    $type = isset($this->argument->options['validate_argument_type']) ? $this->argument->options['validate_argument_type'] : 'tid';
    $transform = isset($this->argument->options['validate_argument_transform']) ? $this->argument->options['validate_argument_transform'] : FALSE;

    switch ($type) {
      case 'tid':
        if (!is_numeric($argument)) {
          return FALSE;
        }

        $result = db_fetch_object(db_query(db_rewrite_sql("SELECT t.* FROM {term_data} t WHERE t.tid = %d", 't', 'tid', array($argument)),  $argument));
        if (!$result) {
          return FALSE;
        }

        return empty($vids) || !empty($vids[$result->vid]);
      case 'tids':
        // An empty argument is not a term so doesn't pass.
        if (empty($argument)) {
          return FALSE;
        }

        $tids = new stdClass();
        $tids->value = $argument;
        $tids = views_break_phrase($argument, $tids);
        if ($tids->value == array(-1)) {
          return FALSE;
        }

        $test = drupal_map_assoc($tids->value);
        $titles = array();

        // check, if some tids already verified
        static $validated_cache = array();
        foreach ($test as $tid) {
          if (isset($validated_cache[$tid])) {
            if ($validated_cache[$tid] === FALSE) {
              return FALSE;
            }
            else {
              $titles[] = $validated_cache[$tid];
              unset($test[$tid]);
            }
          }
        }


        // if unverified tids left - verify them and cache results
        if (count($test)) {
          $placeholders = implode(', ', array_fill(0, count($test), '%d'));

          $result = db_query(db_rewrite_sql("SELECT t.* FROM {term_data} t WHERE t.tid IN ($placeholders)", 't', 'tid', array($test)), $test);
          while ($term = db_fetch_object($result)) {
            if ($vids && empty($vids[$term->vid])) {
              $validated_cache[$term->tid] = FALSE;
              return FALSE;
            }

            $titles[] = $validated_cache[$term->tid] = check_plain($term->name);
            unset($test[$term->tid]);
          }
        }

        // Remove duplicate titles
        $titles = array_unique($titles);

        $this->argument->validated_title = implode($tids->operator == 'or' ? ' + ' : ', ', $titles);
        // If this is not empty, we did not find a tid.
        return empty($test);
      case 'name':
      case 'convert':
        $and = '';
        if (!empty($vids)) {
          $and = " AND t.vid IN(" . implode(', ', $vids) . ')';
        }
        if ($transform) {
          $result = db_fetch_object(db_query(db_rewrite_sql("SELECT t.* FROM {term_data} t LEFT JOIN {term_synonym} ts ON ts.tid = t.tid WHERE (replace(t.name, ' ', '-') = '%s' OR replace(ts.name, ' ', '-') = '%s')$and", 't', 'tid', array($argument, $argument)), $argument, $argument));
        }
        else {
          $result = db_fetch_object(db_query(db_rewrite_sql("SELECT t.* FROM {term_data} t LEFT JOIN {term_synonym} ts ON ts.tid = t.tid WHERE (t.name = '%s' OR ts.name = '%s')$and", 't', 'tid', array($argument, $argument)), $argument, $argument));
        }
        if (!$result) {
          return FALSE;
        }

        if ($type == 'convert') {
          $this->argument->argument = $result->tid;
        }
        $this->argument->validated_title = check_plain($result->name);

        return TRUE;
    }
  }
}
