<?php
/**
 * Argument handler to accept a file id.
 */
class views_handler_argument_file_fid extends views_handler_argument {
  /**
   * Override the behavior of title(). Get the title of the file.
   */
  function title() {
    $title = db_result(db_query(db_rewrite_sql("SELECT f.filename FROM {files} f WHERE f.fid = %d", $this->argument)));
    if (empty($title)) {
      return t('No title');
    }

    return check_plain($title);
  }
}
