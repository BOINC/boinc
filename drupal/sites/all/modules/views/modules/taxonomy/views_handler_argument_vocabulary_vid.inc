<?php
// $Id: views_handler_argument_vocabulary_vid.inc,v 1.1 2008/09/03 19:21:30 merlinofchaos Exp $

/**
 * Argument handler to accept a vocabulary id.
 */
class views_handler_argument_vocabulary_vid extends views_handler_argument {
  /**
   * Override the behavior of title(). Get the name of the user.
   */
  function title() {
    $title = db_result(db_query("SELECT v.name FROM {vocabulary} v WHERE v.vid = %d", $this->argument));

    if (empty($title)) {
      return t('No vocabulary');
    }

    return check_plain($title);
  }
}
