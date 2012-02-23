Drupal.behaviors.actionButtons = function (context) {
  $("div.privatemsg-op-button").hide();

  $('#privatemsg-list #edit-operation').change(function () {
    $('#edit-submit').click();
  });
};
