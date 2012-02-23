<?php
// $Id: views_handler_sort_formula.inc,v 1.1 2008/09/03 19:21:28 merlinofchaos Exp $
/**
 * Base sort handler that has no options and performs a simple sort
 *
 * Definition items:
 * - formula: The formula to use to sort on, such as with a random sort.
 *            The formula should be an array, with keys for database
 *            types, and 'default' for non-specified. 'default' is
 *            required, all others ('mysql', 'mysqli' and 'pgsql' are
 *            optional). It is recommended you use 'default' for mysql
 *            and create specific overrides for pgsql when the formulae
 *            differ.
 *
 * @ingroup views_sort_handlers
 */
class views_handler_sort_formula extends views_handler_sort {
  /**
   * Constructor to take the formula this sorts on.
   */
  function construct() {
    $this->formula = $this->definition['formula'];
    if (is_array($this->formula) && !isset($this->formula['default'])) {
      $this->error = t('views_handler_sort_formula missing default: @formula', array('@formula' => var_export($this->formula, TRUE)));
    }
    parent::construct();
  }
  /**
   * Called to add the sort to a query.
   */
  function query() {
    if (is_array($this->formula)) {
      global $db_type;
      if (isset($this->formula[$db_type])) {
        $formula = $this->formula[$db_type];
      }
      else {
        $formula = $this->formula['default'];
      }
    }
    else {
      $formula = $this->formula;
    }
    $this->ensure_my_table();
    // Add the field.
    $this->query->add_orderby(NULL, $formula, $this->options['order'], $this->table_alias . '_' . $this->field);
  }
}
