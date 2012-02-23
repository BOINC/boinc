// $Id: jump-menu.js,v 1.1.2.2 2009/10/28 01:53:15 merlinofchaos Exp $

(function($) {
  Drupal.behaviors.CToolsJumpMenu = function(context) {
    $('.ctools-jump-menu-hide:not(.ctools-jump-menu-processed)')
      .addClass('ctools-jump-menu-processed')
      .hide();

    $('.ctools-jump-menu-change:not(.ctools-jump-menu-processed)')
      .addClass('ctools-jump-menu-processed')
      .change(function() {
        var loc = $(this).val();
        if (loc) {
          location.href = loc;
        }
        return false;
      });

    $('.ctools-jump-menu-button:not(.ctools-jump-menu-processed)')
      .addClass('ctools-jump-menu-processed')
      .click(function() {
        // Instead of submitting the form, just perform the redirect.

        // Find our sibling value.
        var $select = $(this).parents('form').find('.ctools-jump-menu-select');
        var loc = $select.val();
        if (loc) {
          location.href = loc;
        }
        return false;
      });
  };

})(jQuery);