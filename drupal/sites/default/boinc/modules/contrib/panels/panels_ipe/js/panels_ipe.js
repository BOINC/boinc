
// Ensure the $ alias is owned by jQuery.
(function($) {

Drupal.PanelsIPE = {
  editors: {},
  bindClickDelete: function(context) {
    $('a.pane-delete:not(.pane-delete-processed)', context)
      .addClass('pane-delete-processed')
      .click(function() {
        if (confirm('Remove this pane?')) {
          $(this).parents('div.panels-ipe-portlet-wrapper').fadeOut('medium', function() {
            $(this).empty().remove();
          });
          $(this).parents('div.panels-ipe-display-container').addClass('changed');
        }
        return false;
      });
  }
}

// A ready function should be sufficient for this, at least for now
$(function() {
  $.each(Drupal.settings.PanelsIPECacheKeys, function() {
    Drupal.PanelsIPE.editors[this] = new DrupalPanelsIPE(this, Drupal.settings.PanelsIPESettings[this]);
  });
});

Drupal.behaviors.PanelsIPE = function(context) {
  Drupal.PanelsIPE.bindClickDelete(context);
};

Drupal.CTools.AJAX.commands.initIPE = function(data) {
  if (Drupal.PanelsIPE.editors[data.key]) {
    Drupal.PanelsIPE.editors[data.key].initEditing(data.data);
  }
};

Drupal.CTools.AJAX.commands.unlockIPE = function(data) {
  if (confirm(data.message)) {
    var ajaxOptions = {
      type: "POST",
      url: data.break_path,
      data: { 'js': 1 },
      global: true,
      success: Drupal.CTools.AJAX.respond,
      error: function(xhr) {
        Drupal.CTools.AJAX.handleErrors(xhr, ipe.cfg.formPath);
      },
      dataType: 'json'
    };

    $.ajax(ajaxOptions);
  };
};

Drupal.CTools.AJAX.commands.endIPE = function(data) {
  if (Drupal.PanelsIPE.editors[data.key]) {
    Drupal.PanelsIPE.editors[data.key].endEditing(data);
  }
};



/**
 * Base object (class) definition for the Panels In-Place Editor.
 *
 * A new instance of this object is instanciated for every unique IPE on a given
 * page.
 *
 * Note that this form is provisional, and we hope to replace it with a more
 * flexible, loosely-coupled model that utilizes separate controllers for the
 * discrete IPE elements. This will result in greater IPE flexibility.
 */
function DrupalPanelsIPE(cache_key, cfg) {
  var ipe = this;
  this.key = cache_key;
  this.state = {};
  this.control = $('div#panels-ipe-control-' + cache_key);
  this.initButton = $('div.panels-ipe-startedit', this.control);
  this.cfg = cfg;
  this.changed = false;
  this.sortableOptions = $.extend({
    revert: 200,
    dropOnEmpty: true, // default
    opacity: 0.75, // opacity of sortable while sorting
    // placeholder: 'draggable-placeholder',
    // forcePlaceholderSize: true,
    items: 'div.panels-ipe-portlet-wrapper',
    handle: 'div.panels-ipe-draghandle',
    tolerance: 'pointer',
    cursorAt: 'top',
    update: this.setChanged,
    scroll: true
    // containment: ipe.topParent,
  }, cfg.sortableOptions || {});

  this.initEditing = function(formdata) {
    ipe.topParent = $('div#panels-ipe-display-' + cache_key);
    ipe.backup = this.topParent.clone();

    // See http://jqueryui.com/demos/sortable/ for details on the configuration
    // parameters used here.
    ipe.changed = false;

    $('div.panels-ipe-sort-container', ipe.topParent).sortable(ipe.sortable_options);

    // Since the connectWith option only does a one-way hookup, iterate over
    // all sortable regions to connect them with one another.
    $('div.panels-ipe-sort-container', ipe.topParent)
      .sortable('option', 'connectWith', ['div.panels-ipe-sort-container']);

    $('div.panels-ipe-sort-container', ipe.topParent).bind('sortupdate', function() {
      ipe.changed = true;
    });

    $('.panels-ipe-form-container', ipe.control).append(formdata);
    // bind ajax submit to the form
    $('form', ipe.control).submit(function(event) {
      url = $(this).attr('action');
      try {
        var ajaxOptions = {
          type: 'POST',
          url: url,
          data: { 'js': 1 },
          global: true,
          success: Drupal.CTools.AJAX.respond,
          error: function(xhr) {
            Drupal.CTools.AJAX.handleErrors(xhr, url);
          },
          dataType: 'json'
        };
        $(this).ajaxSubmit(ajaxOptions);
      }
      catch (err) {
        alert("An error occurred while attempting to process " + url);
        return false;
      }
      return false;
    });

    $('input:submit', ipe.control).each(function() {
      if ($(this).attr('id') == 'panels-ipe-save') {
        $(this).click(ipe.saveEditing);
      };
      if ($(this).attr('id') == 'panels-ipe-cancel') {
        $(this).click(ipe.cancelEditing);
      };
    });

    // Perform visual effects in a particular sequence.
    ipe.initButton.css('position', 'absolute');
    ipe.initButton.fadeOut('normal');
    $('.panels-ipe-on').show('normal');
//    $('.panels-ipe-on').fadeIn('normal');
    ipe.topParent.addClass('panels-ipe-editing');
  }

  this.endEditing = function(data) {
    $('.panels-ipe-form-container', ipe.control).empty();
    // Re-show all the IPE non-editing meta-elements
    $('div.panels-ipe-off').show('fast');

    // Re-hide all the IPE meta-elements
    $('div.panels-ipe-on').hide('fast');
    ipe.initButton.css('position', 'static');
    ipe.topParent.removeClass('panels-ipe-editing');
   $('div.panels-ipe-sort-container', ipe.topParent).sortable("destroy");
  };

  this.saveEditing = function() {
    // Put our button in.
    this.form.clk = this;

    $('div.panels-ipe-region', ipe.topParent).each(function() {
      var val = '';
      var region = $(this).attr('id').split('panels-ipe-regionid-')[1];
      $(this).find('div.panels-ipe-portlet-wrapper').each(function() {
        var id = $(this).attr('id').split('panels-ipe-paneid-')[1];
        if (id) {
          if (val) {
            val += ',';
          }
          val += id;
        }
      });
      $('input#edit-panel-pane-' + region, ipe.control).val(val);
    });
  }

  this.cancelEditing = function() {
    // Put our button in.
    this.form.clk = this;

    if (ipe.topParent.hasClass('changed')) {
      ipe.changed = true;
    }

    if (!ipe.changed || confirm(Drupal.t('This will discard all unsaved changes. Are you sure?'))) {
      ipe.topParent.fadeOut('medium', function() {
        ipe.topParent.replaceWith(ipe.backup.clone());
        ipe.topParent = $('div#panels-ipe-display-' + ipe.key);

        // Processing of these things got lost in the cloning, but the classes remained behind.
        // @todo this isn't ideal but I can't seem to figure out how to keep an unprocessed backup
        // that will later get processed.
        $('.ctools-use-modal-processed', ipe.topParent).removeClass('ctools-use-modal-processed');
        $('.pane-delete-processed', ipe.topParent).removeClass('pane-delete-processed');
        ipe.topParent.fadeIn('medium');
        Drupal.attachBehaviors();
      });
    }
    else {
      // Cancel the submission.
      return false;
    }
  };

  this.createSortContainers = function() {
    $('div.panels-ipe-region', this.topParent).each(function() {
      $('div.panels-ipe-portlet-marker', this).parent()
        .wrapInner('<div class="panels-ipe-sort-container" />');

      // Move our gadgets outside of the sort container so that sortables
      // cannot be placed after them.
      $('div.panels-ipe-portlet-static', this).each(function() {
        $(this).appendTo($(this).parent().parent());
      });

      // Add a marker so we can drag things to empty containers.
      $('div.panels-ipe-sort-container', this).append('<div>&nbsp;</div>');
    });
  }

  this.createSortContainers();

  var ajaxOptions = {
    type: "POST",
    url: ipe.cfg.formPath,
    data: { 'js': 1 },
    global: true,
    success: Drupal.CTools.AJAX.respond,
    error: function(xhr) {
      Drupal.CTools.AJAX.handleErrors(xhr, ipe.cfg.formPath);
    },
    dataType: 'json'
  };

  $('div.panels-ipe-startedit', this.control).click(function() {
    var $this = $(this);
    $.ajax(ajaxOptions);
  });
};

})(jQuery);
