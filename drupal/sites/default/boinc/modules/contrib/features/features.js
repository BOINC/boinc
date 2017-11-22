
Drupal.behaviors.features = function() {
  // Features management form package tabs
  $("ul#features-form-links li a:not(.features-processed)").each(function() {
    $(this).addClass('features.processed').click(function() {
      $(".features-package-active").removeClass('features-package-active');
      var panel = $(this).attr('href').split('#')[1];
      $("div.package-" + panel).addClass('features-package-active');
      $(this).addClass('features-package-active');
      return false;
    });
  });

  // Features management form
  $('table.features:not(.processed)').addClass('processed').each(function() {
    // Check the overridden status of each feature
    Drupal.features.checkStatus();

    // Add some nicer row hilighting when checkboxes change values
    $('input', this).bind('change', function() {
      if (!$(this).attr('checked')) {
        $(this).parents('tr').removeClass('enabled').addClass('disabled');
      }
      else {
        $(this).parents('tr').addClass('enabled').removeClass('disabled');
      }
    });
  });

  // Export form component selector
  $('form.features-export-form select.features-select-components:not(.processed)').each(function() {
    $(this)
      .addClass('processed')
      .change(function() {
        var target = $(this).val();
        $('div.features-select').hide();
        $('div.features-select-' + target).show();
        return false;
    });
  });

  // Export form machine-readable JS
  $('.feature-name:not(.processed)').each(function() {
    $('.feature-name')
      .addClass('processed')
      .after(' <small class="feature-module-name-suffix">&nbsp;</small>');
    if ($('.feature-module-name').val() === $('.feature-name').val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_') || $('.feature-module-name').val() === '') {
      $('.feature-module-name').parents('.form-item').hide();
      $('.feature-name').bind('keyup change', function() {
        var machine = $(this).val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_');
        if (machine !== '_' && machine !== '') {
          $('.feature-module-name').val(machine);
          $('.feature-module-name-suffix').empty().append(' Machine name: ' + machine + ' [').append($('<a href="#">'+ Drupal.t('Edit') +'</a>').click(function() {
            $('.feature-module-name').parents('.form-item').show();
            $('.feature-module-name-suffix').hide();
            $('.feature-name').unbind('keyup');
            return false;
          })).append(']');
        }
        else {
          $('.feature-module-name').val(machine);
          $('.feature-module-name-suffix').text('');
        }
      });
      $('.feature-name').keyup();
    }
  });
};

Drupal.features = {
  'checkStatus': function() {
    if ($('table.features').is('.loading')) {
      return;
    }
    $('table.features tbody tr:not(.processed):first').addClass('processed').each(function() {
      var uri = $(this).find('a.admin-check').attr('href'), elem = $(this);
      if (uri) {
        $('table.features').addClass('loading');
        $.get(uri, [], function(data) {
          $(elem).find('.admin-loading').hide();
          switch (data.storage) {
            case 3:
              $(elem).find('.admin-rebuilding').show();
              break;
            case 2:
              $(elem).find('.admin-needs-review').show();
              break;
            case 1:
              $(elem).find('.admin-overridden').show();
              break;
            default:
              $(elem).find('.admin-default').show();
              break;
          }
          $('table.features').removeClass('loading');
          Drupal.features.checkStatus();
        }, 'json');
      }
      else {
        Drupal.features.checkStatus();
      }
    });
  }
};
