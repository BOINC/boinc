/*
Copyright (c) 2003-2011, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/
Drupal.ckeditor = (typeof(CKEDITOR) != 'undefined');
Drupal.ckeditor_ver = false;
// this object will store teaser information
Drupal.ckeditorTeaser = {
  lookup: {},
  lookupSetup: false,
  cache: {}
};

Drupal.ckeditorToggle = function(textarea_id, TextTextarea, TextRTE, xss_check){
  if (!CKEDITOR.env.isCompatible) {
    return;
  }
  if (typeof(CKEDITOR.instances) != 'undefined' && typeof(CKEDITOR.instances[textarea_id]) != 'undefined') {
    Drupal.ckeditorOff(textarea_id);
    $('#switch_' + textarea_id).text(TextRTE);
  }
  else {
    $("#" + textarea_id).val(Drupal.ckeditorLinebreakConvert(textarea_id, $("#" + textarea_id).val()));
    Drupal.ckeditorOn(textarea_id);
    $('#switch_' + textarea_id).text(TextTextarea);
  }
};

/**
 * CKEditor starting function
 *
 * @param string textarea_id
 */
Drupal.ckeditorInit = function(textarea_id) {
  $("#" + textarea_id).next(".grippie").css("display", "none");
  $("#" + textarea_id).addClass("ckeditor-processed");

  var textarea_settings = false;
  if (typeof(Drupal.settings.ckeditor.settings[textarea_id].toolbar) != 'object') {
    Drupal.settings.ckeditor.settings[textarea_id].toolbar = Drupal.ckeditorToolbarToArray(Drupal.settings.ckeditor.settings[textarea_id].toolbar);
  }
  textarea_settings = Drupal.settings.ckeditor.settings[textarea_id];
  textarea_settings['on'] =
  {
    configLoaded  : function(ev)
    {
      Drupal.ckeditor_ver = CKEDITOR.version.split('.')[0];
      if (Drupal.ckeditor_ver == 3) {
        ev.editor.addCss(ev.editor.config.extraCss);
      }
      else {
        CKEDITOR.addCss(ev.editor.config.extraCss);
      }
    },
    instanceReady : function(ev)
    {
      var body = $(ev.editor.document.$.body);
      // Don't enter line breaks after paragraph so we can be friendly to drupal's line break filter.
      if (typeof(ev.editor.dataProcessor.writer.setRules) != 'undefined') {
        ev.editor.dataProcessor.writer.setRules('p', {
          breakAfterOpen: false
        });
        if (typeof(textarea_settings.custom_formatting) != 'undefined') {
          var dtd = CKEDITOR.dtd;
          for ( var e in CKEDITOR.tools.extend( {}, dtd.$block, dtd.$listItem, dtd.$tableContent ) ) {
            ev.editor.dataProcessor.writer.setRules( e, textarea_settings.custom_formatting);
          }
          ev.editor.dataProcessor.writer.setRules( 'pre',
          {
            indent: textarea_settings.output_pre_indent
          });
        }
      }

      if (ev.editor.config.bodyClass)
        body.addClass(ev.editor.config.bodyClass);
      if (ev.editor.config.bodyId)
        body.attr('id', ev.editor.config.bodyId);
      if (typeof(Drupal.smileysAttach) != 'undefined' && typeof(ev.editor.dataProcessor.writer) != 'undefined')
        ev.editor.dataProcessor.writer.indentationChars = '    ';
    },
    focus : function(ev)
    {
      Drupal.ckeditorInstance = ev.editor;
      Drupal.ckeditorActiveId = ev.editor.name;
    }
  };

  if (typeof textarea_settings['js_conf'] != 'undefined'){
      for (var add_conf in textarea_settings['js_conf']){
          var data;
          if (add_conf == 'toolbar') {
            data = Drupal.ckeditorToolbarToArray(textarea_settings['js_conf'][add_conf]);
          } else if (typeof textarea_settings['js_conf'][add_conf] === "boolean" ) {
            data = textarea_settings['js_conf'][add_conf];
          } else {
            data = JSON.parse(textarea_settings['js_conf'][add_conf].replace(/'/g, '"'));
          }

          textarea_settings[add_conf] = data;
      }
  }

  if (CKEDITOR.loadFullCore) {
    CKEDITOR.on('loaded', function() {
      textarea_settings = Drupal.ckeditorLoadPlugins(textarea_settings);
      if (CKEDITOR.instances[textarea_id]) {
        CKEDITOR.instances[textarea_id].destroy(true);
      }
      Drupal.ckeditorInstance = CKEDITOR.replace(textarea_id, textarea_settings);
    });
    CKEDITOR.loadFullCore();
  }
  else {
    textarea_settings = Drupal.ckeditorLoadPlugins(textarea_settings);
    if (CKEDITOR.instances[textarea_id]) {
      CKEDITOR.instances[textarea_id].destroy(true);
    }
    Drupal.ckeditorInstance = CKEDITOR.replace(textarea_id, textarea_settings);
  }
};

Drupal.ckeditorOn = function(textarea_id) {
  if ((typeof(Drupal.settings.ckeditor.load_timeout) == 'undefined') && (typeof(CKEDITOR.instances[textarea_id]) != 'undefined')) {
    return;
  }
  if (typeof(Drupal.settings.ckeditor.settings[textarea_id]) == 'undefined') {
    return;
  }
  if (!CKEDITOR.env.isCompatible) {
    return;
  }
  var teaser = Drupal.ckeditorTeaserInfo(textarea_id);
  if (teaser) {
    var ch_checked = teaser.checkbox.attr('checked');
    var tv = teaser.textarea.val();
    if (teaser.textarea.attr("disabled") !== true && teaser.textarea.attr("disabled") !== 'disabled') {
      $("#" + textarea_id).val(tv + '\n<!--break-->\n' + $("#" + textarea_id).val());
      teaser.textarea.val('');
    }

    // [#653498]
    if (teaser.button.attr('value') != Drupal.t('Split summary at cursor')) {
      try {
        teaser.button.click();
      }
      catch (e) {
        teaser.button.val(Drupal.t('Split summary at cursor'));
      }
    }

    teaser.buttonContainer.hide();
    teaser.textareaContainer.hide();
    teaser.checkboxContainer.show();
    teaser.checkbox.attr('checked', ch_checked);
  }

  if (( $("#" + textarea_id).length > 0 && $("#" + textarea_id).val().length > 0) && ($("#" + textarea_id).attr('class').indexOf("filterxss1") != -1 || $("#" + textarea_id).attr('class').indexOf("filterxss2") != -1)) {
    if (typeof Drupal.settings.ckeditor.settings[textarea_id].input_format == 'undefined') {
        Drupal.settings.ckeditor.settings[textarea_id].input_format = Drupal.settings.ckeditor.default_input_format;
    }
    else if (typeof Drupal.settings.ckeditor.settings[textarea_id].input_format == 'object') {
        Drupal.settings.ckeditor.settings[textarea_id].input_format = Drupal.settings.ckeditor.settings[textarea_id].input_format.pop();
    }
    $.post(Drupal.settings.ckeditor.xss_url, {
      'text': $('#' + textarea_id).val(),
      'token': Drupal.settings.ckeditor.ajaxToken,
      'textarea_id': textarea_id,
      'query': Drupal.settings.ckeditor.query,
      'theme': Drupal.settings.ckeditor.theme,
      'input_format': Drupal.settings.ckeditor.settings[textarea_id].input_format
    }, function(text){
      $("#" + textarea_id).val(text);
      Drupal.ckeditorInit(textarea_id);
    });
  }
  else {
    Drupal.ckeditorInit(textarea_id);
  }

};

/**
 * CKEditor destroy function
 *
 * @param string textarea_id
 */
Drupal.ckeditorOff = function(textarea_id) {
  if (typeof(CKEDITOR.instances[textarea_id]) == 'undefined') {
    return;
  }
  if (!CKEDITOR.env.isCompatible) {
    return;
  }
  if (Drupal.ckeditorInstance && Drupal.ckeditorInstance.name == textarea_id)
    delete Drupal.ckeditorInstance;

  var data = CKEDITOR.instances[textarea_id].getData();
  CKEDITOR.instances[textarea_id].destroy();
  var teaser = Drupal.ckeditorTeaserInfo(textarea_id);
  if (teaser) {
    var brcode = /<!--break-->/;
    data = data.split(brcode);
    if (data.length > 1) {
      teaser.textareaContainer.show();
      teaser.textarea.attr('disabled', '');
      if (teaser.button.attr('value') != Drupal.t('Join summary')) {
        try {
          teaser.button.click();
        }
        catch (e) {
          teaser.button.val(Drupal.t('Join summary'));
        }
      }
      teaser.textarea.val(data[0]);
      $("#" + textarea_id).val(data[1]);
    }
    else {
      $("#" + textarea_id).val(data[0]);
      teaser.textarea.attr('disabled', 'disabled');
      teaser.checkboxContainer.hide();
      if (teaser.button.attr('value') != Drupal.t('Split summary at cursor')) {
        try {
          teaser.button.click();
        }
        catch (e) {
          teaser.button.val(Drupal.t('Split summary at cursor'));
        }
      }
    }
    teaser.buttonContainer.show();
  }

  $("#" + textarea_id).next(".grippie").css("display", "block");
  $("#" + textarea_id).removeClass("ckeditor-processed");
};

/**
 * Loading selected CKEditor plugins
 *
 * @param object textarea_settings
 */
Drupal.ckeditorLoadPlugins = function(textarea_settings) {
  if (typeof(textarea_settings.extraPlugins) == 'undefined') {
    textarea_settings.extraPlugins = '';
  }
  if (typeof CKEDITOR.plugins != 'undefined') {
    for (var plugin in textarea_settings['loadPlugins']) {
      if (typeof(textarea_settings['loadPlugins'][plugin]['active']) == 'undefined' || textarea_settings['loadPlugins'][plugin]['active'] == 1) {
        textarea_settings.extraPlugins += (textarea_settings.extraPlugins) ? ',' + textarea_settings['loadPlugins'][plugin]['name'] : textarea_settings['loadPlugins'][plugin]['name'];
        CKEDITOR.plugins.addExternal(textarea_settings['loadPlugins'][plugin]['name'], textarea_settings['loadPlugins'][plugin]['path']);
      }
    }
  }
  return textarea_settings;
}

/**
 * CKEditor popup mode function
 */
function ckeditorOpenPopup(jsID, textareaID, width){
  popupUrl = Drupal.settings.ckeditor.module_path + '/includes/ckeditor.popup.html?var=' + jsID + '&el=' + textareaID;
  var percentPos = width.indexOf('%');
  if (percentPos != -1) {
    width = width.substr(0, percentPos);
    width = width / 100 * screen.width;
  }
  window.open(popupUrl, null, 'width=' + width + ',toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=no,resizable=1,dependent=yes');
  return false;
};

/**
 * Returns true if CKEDITOR.version >= version
 */
Drupal.ckeditorCompareVersion = function (version){
  var ckver = CKEDITOR.version;
  ckver = ckver.match(/(([\d]\.)+[\d]+)/i);
  version = version.match(/((\d+\.)+[\d]+)/i);
  ckver = ckver[0].split('.');
  version = version[0].split('.');
  for (var x in ckver) {
    if (ckver[x]<version[x]) {
      return false;
    }
    else if (ckver[x]>version[x]) {
      return true;
    }
  }

  return true;
};

/**
 * This function retrieves information about a possible teaser field associated
 * with the mentioned field.
 *
 * @param taid
 *            string HTML id of the main text area
 */
Drupal.ckeditorTeaserInfo = function(taid) {
  // if the result is cached, return it
  if (Drupal.ckeditorTeaser.cache[taid]) {
    return Drupal.ckeditorTeaser.cache[taid];
  }
  // build a lookup table
  if (!Drupal.ckeditorTeaser.lookupSetup) {
    Drupal.ckeditorTeaser.lookupSetup = true;
    for (var x in Drupal.settings.teaser) {
      Drupal.ckeditorTeaser.lookup[Drupal.settings.teaser[x]] = x;
    }
  }
  // find the elements
  if (Drupal.ckeditorTeaser.lookup[taid]) {
    var obj;
    if (window.opener && window.ckeditor_was_opened_in_popup_window) {
      obj = {
        textarea: window.opener.$('#' + Drupal.ckeditorTeaser.lookup[taid]),
        checkbox: window.opener.$('#' + Drupal.settings.teaserCheckbox[Drupal.ckeditorTeaser.lookup[taid]])
      };
    } else {
      obj = {
        textarea: $('#' + Drupal.ckeditorTeaser.lookup[taid]),
        checkbox: $('#' + Drupal.settings.teaserCheckbox[Drupal.ckeditorTeaser.lookup[taid]])
      };
    }
    obj.textareaContainer = obj.textarea.parent();
    obj.checkboxContainer = obj.checkbox.parent();
    obj.button = $('input.teaser-button', obj.checkbox.parents('div.teaser-checkbox').get(0));
    obj.buttonContainer = obj.button.parent();
    Drupal.ckeditorTeaser.cache[taid] = obj;
  }
  else {
    Drupal.ckeditorTeaser.cache[taid] = null;
  }

  return Drupal.ckeditorTeaser.cache[taid];
};

Drupal.ckeditorInsertHtml = function(html) {
  if (!Drupal.ckeditorInstance)
    return false;

  if (Drupal.ckeditorInstance.mode == 'wysiwyg') {
    Drupal.ckeditorInstance.insertHtml(html);
    return true;
  }
  else {
    alert(Drupal.t('Content can only be inserted into CKEditor in the WYSIWYG mode.'));
    return false;
  }
};

/**
 * Converts \n to <br />
 * It in no way tries to compete with Line break converter filter
 */
Drupal.ckeditorEnterModeConvert = function(enterMode){
  if (enterMode == 1)
    return {startTag: '<p>', endTag: '</p>'};
  if (enterMode == 2)
    return {startTag: '', endTag: '<br/>'};
  if (enterMode == 3)
    return {startTag: '<div>', endTag: '</div>'};
  return {startTag: '', endTag: ''}
}

Drupal.ckeditorLinebreakConvert = function(textarea_id, text) {
  var enterMode = Drupal.ckeditorEnterModeConvert(Drupal.settings.ckeditor.settings[textarea_id].enterMode);
  if (!text.match(/<(div|p|br).*\/?>/i) && text) {
    text = enterMode.startTag +  text.replace(/\r\n|\n\r/g, '\n').replace(/\n\n/g, enterMode.endTag+enterMode.startTag).replace(/\n/g, '<br />')  + enterMode.endTag;
  }
  return text;
}

if (typeof(Drupal.ckeditorToolbarToArray) == 'undefined') {
  Drupal.ckeditorToolbarToArray = function (toolbar) {
    toolbar = toolbar.replace(/\r?\n|\r/gmi, '')
        .replace(/\s/gmi, '')
        .replace(/([a-zA-Z0-9]+?):/g, '"$1":')
        .replace(/'/g, '"');

    return JSON.parse(toolbar);
  };
}

/**
 * Ajax support [#741572]
 */
if (typeof(Drupal.Ajax) != 'undefined' && typeof(Drupal.Ajax.plugins) != 'undefined') {
  Drupal.Ajax.plugins.CKEditor = function(hook, args) {
    if (hook === 'submit' && typeof(CKEDITOR.instances) != 'undefined') {
      for (var i in CKEDITOR.instances)
        CKEDITOR.instances[i].updateElement();
    }
    return true;
  };
}

/**
 * IMCE support
 */
function ckeditor_fileUrl(file, win){
  var cfunc = win.location.href.split('&');

  for (var x in cfunc) {
    if (cfunc[x].match(/^CKEditorFuncNum=\d+$/)) {
      cfunc = cfunc[x].split('=');
      break;
    }
  }

  CKEDITOR.tools.callFunction(cfunc[1], file.url);
  win.close();
}

//Support for Panels [#679976]
Drupal.ckeditorSubmitAjaxForm = function () {
  if (typeof(CKEDITOR.instances) != 'undefined' && typeof(CKEDITOR.instances['edit-body']) != 'undefined') {
    Drupal.ckeditorOff('edit-body');
  }
};

/**
 * Drupal behaviors
 */
var imagefield_data = new Object();
Drupal.behaviors.ckeditor = function (context) {
  if ((typeof(CKEDITOR) == 'undefined') || !CKEDITOR.env.isCompatible) {
    return;
  }

  if (typeof CKEDITOR.instances != 'undefined')
  {
      $.each(CKEDITOR.instances, function(index, value){
        if ($('#'+index).length == 0)
        {
          delete CKEDITOR.instances[index];
        }
      });
  }
  $('.ckeditor_links').show();
  // make sure the textarea behavior is run first, to get a correctly sized grippie
  // the textarea behavior requires the teaser behavior, so load that one as well
  if (Drupal.behaviors.teaser) {
    Drupal.behaviors.teaser(context);
  }
  if (Drupal.behaviors.textarea) {
    Drupal.behaviors.textarea(context);
  }

  //Support for imageField [#1286192]
  if (Drupal.behaviors.filefieldButtons) {
    $('input[id$="-add-more"]:submit').each(function(){
      $(this).mousedown(function(){
        $('.form-item textarea.ckeditor-mod', $(this).parent().prev().html()).each(function(){
          if (typeof CKEDITOR.instances[$(this).attr('id')] != 'undefined') {
            imagefield_data[$(this).attr('id')] = CKEDITOR.instances[$(this).attr('id')].getData();
          }
          Drupal.ckeditorOff($(this).attr('id'));
        });
      });
    });
    if ($('.form-item textarea', $(context)).length == 1) {
      var url = document.location.pathname;
      if (url.indexOf('/') == 0 ) url =  url.substr(1);
      var path = Drupal.settings.basePath;
      var imagefield_id = $('textarea', $(context)).attr('id');
      if (!CKEDITOR.instances[imagefield_id]){
        $.ajax({
          url: path + 'admin/ckeditor/get_settings',
          dataType: 'json',
          data: {'id': imagefield_id, 'url': url, 'token': Drupal.settings.ckeditor.ajaxToken},
          type: 'POST',
          success: function( data ) {
            Drupal.settings.ckeditor.settings[imagefield_id] = data;
            if ($(data).length > 0) {
              Drupal.settings.ckeditor.autostart[imagefield_id] = true;
              Drupal.ckeditorOn(imagefield_id);
            }
          }
        });
        $('input[id$="-filefield-remove"]', $(context)).mousedown(function(){
          $('textarea.ckeditor-mod', $(context)).each(function(){
            if (CKEDITOR.instances[$(this).attr('id')]){
              Drupal.ckeditorOff($(this).attr('id'));
              imagefield_data[$(this).attr('id')] = "";
            }
          })
        });
      }
    }
  }

  //Added for support [#1288664] Views
  if ($(context).attr('id') === 'views-ajax-pad'){
    $("div.form-buttons input#edit-submit").click(function(){
        if (typeof CKEDITOR.instances['edit-header'] != 'undefined' )$('#edit-header').attr('value',CKEDITOR.instances['edit-header'].document.getBody().getHtml());
        if (typeof CKEDITOR.instances['edit-footer'] != 'undefined' )$('#edit-footer').attr('value',CKEDITOR.instances['edit-footer'].document.getBody().getHtml());
        if (typeof CKEDITOR.instances['edit-empty'] != 'undefined' )$('#edit-empty').attr('value',CKEDITOR.instances['edit-empty'].document.getBody().getHtml());
    });
    var views_textarea_id = $("textarea", $(context)).attr('id');
    if (typeof views_textarea_id != 'undefined' && views_textarea_id != 'edit-options-alter-text'){
      path = Drupal.settings.basePath;
      views_path = $("form", $(context)).attr('action');
      if (views_path.indexOf('/') == 0) {
        views_path = views_path.substr(1)
      }
      $.ajax({
        url: path + 'admin/ckeditor/get_settings',
        dataType: 'json',
        data: {'id': views_textarea_id, 'url': views_path, 'token': Drupal.settings.ckeditor.ajaxToken},
        type: 'POST',
        success: function( data ) {
          if ($(data).length > 0 && typeof CKEDITOR.instances[views_textarea_id] == 'undefined'){
            Drupal.settings.ckeditor.settings[views_textarea_id] = data;
            Drupal.ckeditorOn(views_textarea_id);
          }
        }
      });
    }
  }

  // Support for Panels [#679976]
  if ($(context).attr('id') == 'modal-content') {
    if (CKEDITOR.instances['edit-body'] != 'undefined') {
      Drupal.ckeditorOff('edit-body');
    }
    $('input#edit-return', context).bind('mouseup', Drupal.ckeditorSubmitAjaxForm);
    $('.close').bind('mouseup', Drupal.ckeditorSubmitAjaxForm);
    CKEDITOR.on('dialogDefinition', function (ev) {
      var dialogDefinition = ev.data.definition;
      var _onShow = dialogDefinition.onShow;
      dialogDefinition.onShow = function () {
        if ( _onShow ) {
          _onShow.apply( this );
        }
        $('body').unbind('keypress');
      };
    });
  }

  $("textarea.ckeditor-mod:not(.ckeditor-processed)").each(function () {
    var ta_id=$(this).attr("id");
    if ((typeof(Drupal.settings.ckeditor.autostart) != 'undefined') && (typeof(Drupal.settings.ckeditor.autostart[ta_id]) != 'undefined') ) {
      if(typeof imagefield_data[ta_id] == "string") {
        $("#"+ta_id).attr('value', imagefield_data[ta_id]);
      }
      Drupal.ckeditorOn(ta_id);
    }
  });

  $("form").bind('submit', function() {
    $(this).find('textarea.ckeditor-processed').each(function() {
      $(this).val(Drupal.ckeditorLinebreakConvert($(this).attr("id"), $(this).val()));
    });
  });
};

if (Drupal.tableDrag) {
    Drupal.tableDrag.prototype.onDrag = function() {
      $(this.rowObject.element).find('textarea.ckeditor-processed').each(
        function() {
          if (typeof(CKEDITOR.instances) != 'undefined' && typeof(CKEDITOR.instances[$(this).attr('id')]) != 'undefined') {
            data = CKEDITOR.instances[$(this).attr('id')].document.getBody().getHtml();
            $("#"+$(this).attr('id')).attr('value',data);
            Drupal.ckeditorOff($(this).attr('id'));
          }
        }
      );
    };
    Drupal.tableDrag.prototype.onDrop = function() {

      $(this.rowObject.element).find('textarea.ckeditor-mod:not(.ckeditor-processed)').each(
        function() {
          if ((typeof(Drupal.settings.ckeditor.autostart) != 'undefined') && (typeof(Drupal.settings.ckeditor.autostart[$(this).attr('id')]) != 'undefined')) {
            Drupal.ckeditorOn($(this).attr('id'));
          }
        }
      );
    };
}

