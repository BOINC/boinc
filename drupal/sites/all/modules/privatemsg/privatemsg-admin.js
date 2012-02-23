// $Id: privatemsg-admin.js,v 1.1.2.1 2009/11/26 18:42:54 berdir Exp $

Drupal.behaviors.privatemsgAdminSettings = function (context) {
  if (!$('#edit-privatemsg-view-use-max-as-default').attr('checked')) {
    $('#privatemsg-view-default-button').hide();
  }
  $('#edit-privatemsg-view-use-max-as-default').change( function () {
    $('#privatemsg-view-default-button').toggle();
  });
}