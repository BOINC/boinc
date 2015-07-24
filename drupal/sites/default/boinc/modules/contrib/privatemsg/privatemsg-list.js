Drupal.behaviors.actionButtons = function (context) {
  $("div.privatemsg-op-button").hide();

  $('#privatemsg-list #edit-operation').change(function () {
    $('div.privatemsg-op-button input.form-submit').click();
  });
};
