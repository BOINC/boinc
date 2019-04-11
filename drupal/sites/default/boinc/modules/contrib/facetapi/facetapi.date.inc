<?php

/**
 * @file
 * Date handling functions.
 */

/**
 * Helper function to convert dates from Unix timestamps into ISO 8601 format.
 *
 * @param $timestamp
 *   An integer containing the Unix timestamp being converted.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values. Defaults to FACETAPI_DATE_SECOND.
 *
 * @return
 *   A string containing the date in ISO 8601 format.
 */
function facetapi_isodate($timestamp, $gap = FACETAPI_DATE_SECOND) {
  switch ($gap) {
    case FACETAPI_DATE_SECOND:
      $format = FACETAPI_DATE_ISO8601;
      break;

    case FACETAPI_DATE_MINUTE:
      $format = 'Y-m-d\TH:i:00\Z';
      break;

    case FACETAPI_DATE_HOUR:
      $format = 'Y-m-d\TH:00:00\Z';
      break;

    case FACETAPI_DATE_DAY:
      $format = 'Y-m-d\T00:00:00\Z';
      break;

    case FACETAPI_DATE_MONTH:
      $format = 'Y-m-01\T00:00:00\Z';
      break;

    case FACETAPI_DATE_YEAR:
      $format = 'Y-01-01\T00:00:00\Z';
      break;

    default:
      $format = FACETAPI_DATE_ISO8601;
      break;
  }
  return gmdate($format, $timestamp);
}

/**
 * Return a date gap one increment smaller than the one passed.
 *
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values.
 * @param $min_gap
 *   A string containing the the minimum gap that can be returned, defaults to
 *   FACETAPI_DATE_SECOND. This is useful for defining the smallest increment
 *   that can be used in a date drilldown.
 *
 * @return
 *   A string containing the smaller date gap, NULL if there is no smaller gap.
 *   See FACETAPI_DATE_* constants for valid values.
 */
function facetapi_get_next_date_gap($gap, $min_gap = FACETAPI_DATE_SECOND) {
  // Array of numbers used to determine whether the next gap is smaller than
  // the minimum gap allowed in the drilldown.
  $gap_numbers = array(
    FACETAPI_DATE_YEAR => 6,
    FACETAPI_DATE_MONTH => 5,
    FACETAPI_DATE_DAY => 4,
    FACETAPI_DATE_HOUR => 3,
    FACETAPI_DATE_MINUTE => 2,
    FACETAPI_DATE_SECOND => 1,
  );

  // Gets gap numbers for both the gap and minimum gap, checks if the next gap
  // is within the limit set by the $min_gap parameter.
  $gap_num = isset($gap_numbers[$gap]) ? $gap_numbers[$gap] : 6;
  $min_num = isset($gap_numbers[$min_gap]) ? $gap_numbers[$min_gap] : 1;
  return ($gap_num > $min_num) ? array_search($gap_num - 1, $gap_numbers) : $min_gap;
}

/**
 * Determines the best search gap to use for an arbitrary date range.
 *
 * Generally, we use the maximum gap that fits between the start and end date.
 * If they are more than a year apart, 1 year; if they are more than a month
 * apart, 1 month; etc.
 *
 * This function uses Unix timestamps for its computation and so is not useful
 * for dates outside that range.
 *
 * @param $start_date
 *   A string containing the start date as an ISO date string.
 * @param $end_date
 *   A string containing the end date as an ISO date string.
 *
 * @return
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values. Returns FALSE of either of the dates cannot be converted to a
 *   timestamp.
 */
function facetapi_get_timestamp_gap($start_time, $end_time) {
  $time_diff = $end_time - $start_time;
  switch (TRUE) {
    // NOTE: 31536000 == 60 * 60 * 24 * 365
    case ($time_diff >= 31536000):
      return FACETAPI_DATE_YEAR;

    case ($time_diff >= 86400 * gmdate('t', $start_time)):
      return FACETAPI_DATE_MONTH;

    case ($time_diff >= 86400):
      return FACETAPI_DATE_DAY;

    case ($time_diff >= 3600):
      return FACETAPI_DATE_HOUR;

    case ($time_diff >= 60):
      return FACETAPI_DATE_MINUTE;

    default:
      return FACETAPI_DATE_SECOND;
  }
}

/**
 * Converts ISO date strings to Unix timestamps, passes values to the
 * facetapi_get_timestamp_gap() function to calculate the gap.
 *
 * @param $start_date
 *   A string containing the start date as an ISO date string.
 * @param $end_date
 *   A string containing the end date as an ISO date string.
 *
 * @return
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values. Returns FALSE of either of the dates cannot be converted to a
 *   timestamp.
 *
 * @see facetapi_get_timestamp_gap()
 */
function facetapi_get_date_gap($start_date, $end_date) {
  $range = array(strtotime($start_date), strtotime($end_date));
  if (!in_array(FALSE, $range, TRUE)) {
    return facetapi_get_timestamp_gap($range[0], $range[1]);
  }
  return FALSE;
}

/**
 * Returns a formatted date based on the passed timestamp and gap.
 *
 * This function assumes that gaps less than one day will be displayed in a
 * search context in which a larger containing gap including a day is already
 * displayed. So, HOUR, MINUTE, and SECOND gaps only display time information,
 * without date.
 *
 * @param $timestamp
 *   An integer containing the Unix timestamp.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values, defaults to YEAR.
 *
 * @return
 *   A gap-appropriate display date used in the facet link.
 */
function facetapi_format_timestamp($timestamp, $gap = FACETAPI_DATE_YEAR) {
  switch ($gap) {
    case FACETAPI_DATE_MONTH:
      return format_date($timestamp, 'custom', 'F Y', 'UTC');

    case FACETAPI_DATE_DAY:
      return format_date($timestamp, 'custom', 'F j, Y', 'UTC');

    case FACETAPI_DATE_HOUR:
      return format_date($timestamp, 'custom', 'g A', 'UTC');

    case FACETAPI_DATE_MINUTE:
      return format_date($timestamp, 'custom', 'g:i A', 'UTC');

    case FACETAPI_DATE_SECOND:
      return format_date($timestamp, 'custom', 'g:i:s A', 'UTC');

    default:
      return format_date($timestamp, 'custom', 'Y', 'UTC');
  }
}

/**
 * Returns a formatted date based on the passed ISO date string and gap.
 *
 * @param $date
 *   A string containing the date as an ISO date string.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values, defaults to YEAR.
 * @param $callback
 *   The formatting callback, defaults to "facetapi_format_timestamp".
 *
 * @return
 *   A gap-appropriate display date used in the facet link.
 *
 * @see facetapi_format_timestamp()
 */
function facetapi_format_date($date, $gap = FACETAPI_DATE_YEAR, $callback = 'facetapi_format_timestamp') {
  $timestamp = strtotime($date);
  return $callback($timestamp, $gap);
}

/**
 * Returns the next increment from the given ISO date and gap. This function is
 * useful for getting the upper limit of a date range from the given start
 * date.
 *
 * @param $date
 *   A string containing the date as an ISO date string.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values, defaults to YEAR.
 *
 * @return
 *   A string containing the date, FALSE if the passed date could not be parsed.
 */
function facetapi_get_next_date_increment($date, $gap) {
  if (preg_match(FACETAPI_REGEX_DATE, $date, $match)) {

    // Increments the timestamp.
    switch ($gap) {
      case FACETAPI_DATE_MONTH:
        $match[2] += 1;
        break;
      case FACETAPI_DATE_DAY:
        $match[3] += 1;
        break;
      case FACETAPI_DATE_HOUR:
        $match[4] += 1;
        break;
      case FACETAPI_DATE_MINUTE:
        $match[5] += 1;
        break;
      case FACETAPI_DATE_SECOND:
        $match[6] += 1;
        break;
      default:
        $match[1] += 1;
        break;
    }

    // Gets the next incremenet.
    return facetapi_isodate(
      gmmktime($match[4], $match[5], $match[6], $match[2], $match[3], $match[1])
    );
  }
  return FALSE;
}
