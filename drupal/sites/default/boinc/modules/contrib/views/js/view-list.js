/**
 * @file view-list.js
 *
 * Handles JS things for view listing.
 */
Drupal.behaviors.ViewsList = function() {
  var timeoutID = 0;
  $('form#views-ui-list-views-form select:not(.views-processed)')
    .addClass('views-processed')
    .change(function() {
      $('#edit-views-apply').click();
    });
};
