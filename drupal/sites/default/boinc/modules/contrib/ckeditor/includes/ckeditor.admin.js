/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/
$(document).ready(function() {
  if (typeof(CKEDITOR) == "undefined")
      return;

  $('#edit-uicolor-textarea').show();

  Drupal.ckeditor_ver = Drupal.settings.ckeditor_version.split('.')[0];

  Drupal.editSkinEditorInit = function() {
    var skinframe_src = $('#skinframe').attr('src');
    skinframe_src = skinframe_src.replace(/skin=[^&]+/, 'skin='+$("#edit-skin").val());
    if ($('#edit-uicolor').val() == 'custom') {
      skinframe_src = skinframe_src.replace(/uicolor=[^&]+/, 'uicolor='+$('input[name$="uicolor_user"]').val().replace('#', '') || 'D3D3D3');
    }
    else {
      skinframe_src = skinframe_src.replace(/uicolor=[^&]+/, 'uicolor=D3D3D3');
    }
    $('#skinframe').attr('src', skinframe_src);

    if (Drupal.ckeditor_ver == 3) {
      if ($("#edit-skin").val() == "kama") {
        $("#edit-uicolor").removeAttr('disabled');
        $("#edit-uicolor").parent().removeClass('form-disabled');
      }
      else {
        $("#edit-uicolor").attr('disabled', 'disabled');
        $("#edit-uicolor").parent().addClass('form-disabled');
      }
    }
    else {
      $("#edit-uicolor").removeAttr('disabled');
      $("#edit-uicolor").parent().removeClass('form-disabled');
    }
  };
  Drupal.editSkinEditorInit();

  $("#edit-skin, #edit-uicolor").bind("change", function() {
    Drupal.editSkinEditorInit();
  });

  $(".cke_load_toolbar", "#ckeditor-admin-profile-form").click(function() {
    var id = $(this).attr("id").replace(/[^\w]/g, '');
    if (typeof(Drupal.settings[id]) == 'undefined') {
      return false;
    }
    var buttons = Drupal.settings[id];
    var text = "[\n";
    for(i in buttons) {
      if (typeof buttons[i] == 'string'){
        text = text + "    '/',\n";
      }
      else {
        text = text + "    [";
        max = buttons[i].length - 1;
        rows = buttons.length - 1;
        for (j in buttons[i]) {
          if (j < max){
            text = text + "'" + buttons[i][j] + "',";
          } else {
            text = text + "'" + buttons[i][j] + "'";
          }
        }
        if (i < rows){
          text = text + "],\n";
        } else {
          text = text + "]\n";
        }
      }
    }

    text = text + "]";
    text = text.replace(/\['\/'\]/g,"'/'");
    $("#edit-toolbar").attr('value',text);
    if (Drupal.settings.ckeditor_toolbar_wizard == 't'){
      Drupal.ckeditorToolbarReload();
    }
    return false;
  });

  if (Drupal.settings.ckeditor_toolbar_wizard == 'f'){
    $("form#ckeditor-admin-profile-form textarea#edit-toolbar, form#ckeditor-admin-profile-form #edit-toolbar + .grippie, form#ckeditor-admin-global-profile-form textarea#edit-toolbar, form#ckeditor-admin-global-profile-form #edit-toolbar + .grippie").show();
  }
});
