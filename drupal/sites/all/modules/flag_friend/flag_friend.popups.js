/**
 * @file flag_friend.popups.js
 * Popups submit handler.
 */
(function ($) {
  
    Drupal.flagFriendPopupsAfterSubmit = function(data, options, element) {
      var $element = $(element);
      var msg = '';
      if ($element.hasClass('unflag-action')) {
        msg = 'Friend request canceled.';
      }
      if ($element.hasClass('flag-action')) {
        msg = 'Friend requested.';
      }
      if ($element.text() == 'Approve') {
        msg = 'Friend approved.';
      }
      Popups.message('Flag friend', msg);
      location.reload();
      return false;
    };
  
})(jQuery);