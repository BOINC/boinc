// $Id: ignore_user.js,v 1.1 2008/04/18 07:19:12 jaydub Exp $

if (Drupal.jsEnabled) {
  $(document).ready(function () {
    $('div.ignore-user-container > div').hide();
    $('div.ignore-user-container > a.ignore-user-content-link').click(function() {
      $(this).next('div').slideToggle('fast');
      return false;
    });
  });
}
