<?php
/**
 * @file
 * Returns a file icon based on its MIME type.
 */

class filefield_handler_field_icon extends views_handler_field {
  function construct() {
    parent::construct();
    $this->additional_fields['fid'] = 'fid';
    $this->additional_fields['filename'] = 'filename';
    $this->additional_fields['filesize'] = 'filesize';
  }

  function render($values) {
    $pseudo_file = array(
      'fid' => $values->{$this->aliases['fid']},
      'filemime' => $values->{$this->field_alias},
      'filename' => $values->{$this->aliases['filename']},
      'filesize' => $values->{$this->aliases['filesize']},
    );
    return theme('filefield_icon', $pseudo_file);
  }
}
