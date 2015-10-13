
Drupal.behaviors.privatemsgAdminSettings = function (context) {
  if (!$('#edit-privatemsg-view-use-max-as-default').attr('checked')) {
    $('#privatemsg-view-default-button').hide();
  }
  $('#edit-privatemsg-view-use-max-as-default').change( function () {
    $('#privatemsg-view-default-button').toggle();
  });
}