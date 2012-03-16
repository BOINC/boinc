<?php

backup_migrate_include('destinations.db');

/**
 * @file
 * Functions to handle the direct to database destination.
 */

/**
 * A destination type for saving to a database server.
 *
 * @ingroup backup_migrate_destinations
 */

class backup_migrate_destination_db_mysql extends backup_migrate_destination_db {
  function type_name() {
    return t("MySQL Database");
  }

  /**
   * Return a list of backup filetypes.
   */
  function file_types() {
    return array(
      "sql" => array(
        "extension" => "sql",
        "filemime" => "text/x-sql",
        "backup" => TRUE,
        "restore" => TRUE,
      ),
      "mysql" => array(
        "extension" => "mysql",
        "filemime" => "text/x-sql",
        "backup" => TRUE,
        "restore" => TRUE,
      ),
    );
  }

  /**
   * Declare any mysql databases defined in the settings.php file as a possible destination.
   */
  function destinations() {
    $out = array();
    global $db_url;
    $urls = is_array($db_url) ? $db_url : array('default' => $db_url);
    foreach ((array)$urls as $key => $url) {
      // Only mysql/mysqli supported by this destination.
      if (strpos($url, 'mysql') === 0) {
        if ($destination = backup_migrate_create_destination('mysql', array('url' => $url))) {
          // Treat the default database differently because it is probably the only one available.
          if ($key == 'default') {
            $destination->set_id('db');
            $destination->set_name(t('Default Database'));
            // Dissalow backing up to the default database because that's confusing and potentially dangerous.
            $destination->remove_op('scheduled backup');
            $destination->remove_op('manual backup');
          }
          else {
            $destination->set_id('db:'. $key);
            $destination->set_name($key .": ". $destination->get_display_location());
          }
          $out[$destination->get_id()] = $destination;
        }
      }
    }
    return $out;
  }

  /**
   * Get the file type for to backup this destination to.
   */
  function get_file_type_id() {
    return 'mysql';
  }

  /**
   * Backup the databases to a file.
   *
   *  Returns a list of sql commands, one command per line.
   *  That makes it easier to import without loading the whole file into memory.
   *  The files are a little harder to read, but human-readability is not a priority
   */
  function _backup_db_to_file($file, $settings) {
    $lines = 0;
    $exclude = !empty($settings->filters['exclude_tables']) ? $settings->filters['exclude_tables'] : array();
    $nodata = !empty($settings->filters['nodata_tables']) ? $settings->filters['nodata_tables'] : array();
    if ($file->open(TRUE)) {
      $file->write($this->_get_sql_file_header());
      $alltables = $this->_get_tables();
      foreach ($alltables as $table) {
        if (_backup_migrate_check_timeout()) {
          return FALSE;
        }
        if ($table['Name'] && !isset($exclude[$table['Name']])) {
          $file->write($this->_get_table_structure_sql($table));
          $lines++;
          if (!in_array($table['Name'], $nodata)) {
            $lines += $this->_dump_table_data_sql_to_file($file, $table);
          }
        }
      }
      $file->write($this->_get_sql_file_footer());
      $file->close();
      return $lines;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Backup the databases to a file.
   */
  function _restore_db_from_file($file, $settings) {
    $num = 0;
    if ($file->open()) {
      // Read one line at a time and run the query.
      while ($line = $file->read()) {
        if (_backup_migrate_check_timeout()) {
          return FALSE;
        }
  
        $line = trim($line);
        if ($line) {
          // Use the helper instead of the api function to avoid substitution of '{' etc.
          _db_query($line);
          $num++;
        }
      }
      // Close the file with fclose/gzclose.
      $file->close();
    }
    else {
      drupal_set_message(t("Unable to open file %file to restore database", array("%file" => $file->filepath())), 'error');
      $num = FALSE;
    }
    return $num;
  }

  /**
   * Get a list of tables in the database.
   */
  function _get_table_names() {
    $out = array();
    foreach ($this->_get_tables() as $table) {
      $out[$table['Name']] = $table['Name'];
    }
    return $out;
  }

  /**
   * Lock the list of given tables in the database.
   */
  function _lock_tables($tables) {
    if ($tables) {
      $tables_escaped = array();
      foreach ($tables as $table) {
        $tables_escaped[] = '`'. db_escape_table($table) .'`  WRITE';
      }
      db_query('LOCK TABLES '. implode(', ', $tables_escaped));
    }
  }

  /**
   * Unlock all tables in the database.
   */
  function _unlock_tables() {
    db_query('UNLOCK TABLES');
  }

  /**
   * Get a list of tables in the db.
   */
  function _get_tables() {
    $out = array();
    // get auto_increment values and names of all tables
    $tables = db_query("show table status");
    while ($table = db_fetch_array($tables)) {
      $out[$table['Name']] = $table;
    }
    return $out;
  }

  /**
   * Get the sql for the structure of the given table.
   */
  function _get_table_structure_sql($table) {
    $out = "";
    $result = db_query("SHOW CREATE TABLE `". $table['Name'] ."`");
    if ($create = db_fetch_array($result)) {
      $out .= "DROP TABLE IF EXISTS `". $table['Name'] ."`;\n";
      $out .= strtr($create['Create Table'], "\n", " ");
      if ($table['Auto_increment']) {
        $out .= " AUTO_INCREMENT=". $table['Auto_increment'];
      }
      $out .= ";\n";
    }
    return $out;
  }
  
  /**
   *  Get the sql to insert the data for a given table
   */
  function _dump_table_data_sql_to_file($file, $table) {
    $rows_per_line = variable_get('backup_migrate_data_rows_per_line', 30);
    $bytes_per_line = variable_get('backup_migrate_data_bytes_per_line', 2000);
  
    $lines = 0;
    $data = db_query("SELECT * FROM `". $table['Name'] ."`");
    $rows = $bytes = 0;
  
    $line = array();
    while ($row = db_fetch_array($data)) {
      // DB Escape the values.
      $items = array();
      foreach ($row as $key => $value) {
        $items[] = is_null($value) ? "null" : "'". db_escape_string($value) ."'";
      }
  
      // If there is a row to be added.
      if ($items) {
        // Start a new line if we need to.
        if ($rows == 0) {
          $file->write("INSERT INTO `". $table['Name'] ."` VALUES ");
          $bytes = $rows = 0;
        }
        // Otherwise add a comma to end the previous entry.
        else {
          $file->write(",");
        }
  
        // Write the data itself.
        $sql = implode(',', $items);
        $file->write('('. $sql .')');
        $bytes += strlen($sql);
        $rows++;
  
        // Finish the last line if we've added enough items
        if ($rows >= $rows_per_line || $bytes >= $bytes_per_line) {
          $file->write(";\n");
          $lines++;
          $bytes = $rows = 0;
        }
      }
    }
    // Finish any unfinished insert statements.
    if ($rows > 0) {
      $file->write(";\n");
      $lines++;
    }
  
    return $lines;
  }
  
  /**
   * The header for the top of the sql dump file. These commands set the connection
   *  character encoding to help prevent encoding conversion issues.
   */
  function _get_sql_file_header() {
    return "/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE=NO_AUTO_VALUE_ON_ZERO */;

SET NAMES utf8;

";
  }
  
  /**
   * The footer of the sql dump file.
   */
  function _get_sql_file_footer() {
    return "
  
/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
";
  }
}

