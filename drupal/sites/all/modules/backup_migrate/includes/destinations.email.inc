<?php


/**
 * @file
 * Functions to handle the email backup destination.
 */

/**
 * A destination for emailing database backups.
 *
 * @ingroup backup_migrate_destinations
 */
class backup_migrate_destination_email extends backup_migrate_destination {
  var $supported_ops = array('scheduled backup', 'manual backup', 'configure');

  /**
   * Save to (ie. email the file) to the email destination.
   */
  function save_file($file, $settings) {
    $size = filesize($file->filepath());
    $max = variable_get('backup_migrate_max_email_size', 20971520);
    if ($size > $max) {
      _backup_migrate_message('Could not email the file @file because it is @size and Backup and Migrate only supports emailing files smaller than @max.', array('@file' => $file->filename(), '@size' => format_size($size), '@max' => format_size($max)), 'error');
      return FALSE;
    }
    $attachment = new stdClass();
    $attachment->filename = $file->filename();
    $attachment->path = $file->filepath();
    _backup_migrate_destination_email_mail_backup($attachment, $this->get_location());
    return $file;
  }

  /**
   * Get the form for the settings for this filter.
   */
  function edit_form() {
    $form = parent::edit_form();
    $form['location'] = array(
      "#type" => "textfield",
      "#title" => t("Email Address"),
      "#default_value" => $this->get_location(),
      "#required" => TRUE,
      "#description" => t('Enter the email address to send the backup files to. Make sure the email sever can handle large file attachments'),
    );
    return $form;
  }

  /**
   * Validate the configuration form. Make sure the email address is valid.
   */
  function settings_form_validate($values) {
    if (!valid_email_address($values['location'])) {
      form_set_error('[location]', t('The e-mail address %mail is not valid.', array('%mail' => $form_state['values']['location'])));
    }
  }
}

/**
 * @function
 * Temporary mail handler class.
 *
 * Defines a mail class to send a message with an attachment. Eventually Drupal
 * core should provide this functionality, at which time this code will be
 * removed.
 *
 * More info on sending email at <http://php.net/function.mail>.
 * This function taken from dba.module.
 *
 * @param $attachment
 *   An object which contains two variables "path" the path to the file and
 *   filename and "filename" which is just the filename.
 */
function _backup_migrate_destination_email_mail_backup($attachment, $to) {
  // Send mail
  $attach        = fread(fopen($attachment->path, "r"), filesize($attachment->path));
  $mail          = new mime_mail();
  $mail->from    = variable_get('site_mail', ini_get('sendmail_from'));
  $mail->headers = 'Errors-To: [EMAIL='. $mail->from .']'. $mail->from .'[/EMAIL]';
  $mail->to      = $to;
  $mail->subject = t('Database backup from !site: !file', array('!site' => variable_get('site_name', 'drupal'), '!file' => $attachment->filename));
  $mail->body    = t('Database backup attached.') ."\n\n";

  $mail->add_attachment("$attach", $attachment->filename, "Content-Transfer-Encoding: base64 /9j/4AAQSkZJRgABAgEASABIAAD/7QT+UGhvdG9zaG", NULL, TRUE);
  $mail->send();
}

class mime_mail {
  var $parts;
  var $to;
  var $from;
  var $headers;
  var $subject;
  var $body;

  function mime_mail() {
    $this->parts   = array();
    $this->to      = "";
    $this->from    = "";
    $this->headers = "";
    $this->subject = "";
    $this->body    = "";
  }

  function add_attachment($message, $name = "", $ctype = "application/octet-stream", $encode = NULL, $attach = FALSE) {
    $this->parts[] = array(
      "ctype" => $ctype,
      "message" => $message,
      "encode" => $encode,
      "name" => $name,
      "attach" => $attach,
    );
  }

  function build_message($part) {
    $message  = $part["message"];
    $message  = chunk_split(base64_encode($message));
    $encoding = "base64";
    $disposition = $part['attach'] ? "Content-Disposition: attachment; filename=$part[name]\n" : '';
    return "Content-Type: ". $part["ctype"] . ($part["name"] ? "; name = \"". $part["name"] ."\"" : "") ."\nContent-Transfer-Encoding: $encoding\n$disposition\n$message\n";
  }

  function build_multipart() {
    $boundary = "b". md5(uniqid(time()));
    $multipart = "Content-Type: multipart/mixed; boundary = $boundary\n\nThis is a MIME encoded message.\n\n--$boundary";
    for ($i = sizeof($this->parts) - 1; $i >= 0; $i--) {
      $multipart .= "\n". $this->build_message($this->parts[$i]) ."--$boundary";
    }
    return $multipart .= "--\n";
  }

  function send() {
    $mime = "";
    if (!empty($this->from)) $mime .= "From: ". $this->from ."\n";
    if (!empty($this->headers)) $mime .= $this->headers ."\n";
    if (!empty($this->body)) $this->add_attachment($this->body, "", "text/plain");
    $mime .= "MIME-Version: 1.0\n". $this->build_multipart();
    mail($this->to, $this->subject, "", $mime);
  }
}
