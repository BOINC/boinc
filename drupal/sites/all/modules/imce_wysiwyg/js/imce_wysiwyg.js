// $Id: imce_wysiwyg.js,v 1.3.4.1 2010/02/21 00:07:04 sun Exp $

/**
 * Wysiwyg API integration helper function.
 */
function imceImageBrowser(field_name, url, type, win) {
  // TinyMCE.
  if (win !== 'undefined') {
    win.open(Drupal.settings.imce.url + encodeURIComponent(field_name), '', 'width=760,height=560,resizable=1');
  }
}

/**
 * CKeditor integration.
 */
var imceCkeditSendTo = function (file, win) {
  var parts = /\?(?:.*&)?CKEditorFuncNum=(\d+)(?:&|$)/.exec(win.location.href);
  if (parts && parts.length > 1) {
    CKEDITOR.tools.callFunction(parts[1], file.url);
    win.close();
  }
  else {
    throw 'CKEditorFuncNum parameter not found or invalid: ' + win.location.href;
  }
};
