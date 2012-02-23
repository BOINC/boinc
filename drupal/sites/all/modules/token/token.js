// $Id: token.js,v 1.3.2.2 2010/03/26 00:03:19 davereid Exp $

(function ($) {

Drupal.behaviors.tokenTree = function() {
  $('table.token-tree').each(function() {
    $(this).treeTable();
  });
};

Drupal.behaviors.tokenInsert = function() {
  // Keep track of which textfield was last selected/focused.
  $('textarea, input[type="text"]').focus(function() {
    Drupal.settings.tokenFocusedField = this;
  });

  $('.token-click-insert .token-key').each(function() {
    var newThis = $('<a href="javascript:void(0);" title="' + Drupal.t('Insert this token into your form') + '">' + $(this).html() + '</a>').click(function(){
      if (typeof Drupal.settings.tokenFocusedField == 'undefined') {
        alert(Drupal.t('First click a text field to insert your tokens into.'));
      }
      else {
        var myField = Drupal.settings.tokenFocusedField;
        var myValue = $(this).text();

        //IE support
        if (document.selection) {
          myField.focus();
          sel = document.selection.createRange();
          sel.text = myValue;
        }

        //MOZILLA/NETSCAPE support
        else if (myField.selectionStart || myField.selectionStart == '0') {
          var startPos = myField.selectionStart;
          var endPos = myField.selectionEnd;
          myField.value = myField.value.substring(0, startPos)
                        + myValue
                        + myField.value.substring(endPos, myField.value.length);
        } else {
          myField.value += myValue;
        }

        $('html,body').animate({scrollTop: $(myField).offset().top}, 500);
      }
      return false;
    });
    $(this).html(newThis);
  });
};

})(jQuery);
