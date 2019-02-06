(function ($) {
  
/**
 * Provide the summary information for the block settings vertical tabs.
 */
Drupal.behaviors.currentSearch = {
  attach: function (context) {
    // The drupalSetSummary method required for this behavior is not available
    // on the Blocks administration page, so we need to make sure this
    // behavior is processed only if drupalSetSummary is defined.
    if (typeof jQuery.fn.drupalSetSummary == 'undefined') {
      return;
    }

    $('fieldset#edit-current-search', context).drupalSetSummary(function (context) {
      var $radio = $('input[name="searcher"]:checked', context);
      return $radio.next('label').text();
    });
  }
};

})(jQuery);
