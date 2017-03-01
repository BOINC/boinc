Drupal.behaviors.mobile_menu_toggle = function (context) {
  $("#mobile-menu-toggle", context).click(function (e, context) {
    if (typeof(Drupal.settings.mobile_menu_toggle) == 'undefined') {
      return false;
    }
    $(Drupal.settings.mobile_menu_toggle.css_class).slideToggle('fast');
    if ($(this).hasClass('mobile-menu-toggle-open')) {
      $(this).removeClass('mobile-menu-toggle-open');
    }
    else {
      $(this).addClass('mobile-menu-toggle-open');
    }
    e.preventDefault();
  });
};
