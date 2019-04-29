Drupal.behaviors.facetapi = function(context) {
  var settings = Drupal.settings;
  // Iterates over facet settings, applies functionality like the "Show more"
  // links for block realm facets.
  // @todo We need some sort of JS API so we don't have to make decisions
  // based on the realm.
  if (settings.facetapi) {
    for (var index in settings.facetapi.facets) {
      if (null != settings.facetapi.facets[index].makeCheckboxes) {
        // Find all checkbox facet links and give them a checkbox.
        // Drupal 6 problem, it did not want to select on the real ID
        $('.facetapi-facetapi_checkbox_links a.facetapi-checkbox', context).each(Drupal.facetapi.makeCheckbox);
      }
      if (null != settings.facetapi.facets[index].limit) {
        // Applies soft limit to the list.
        Drupal.facetapi.applyLimit(settings.facetapi.facets[index]);
      }
    }
  }
}

/**
 * Class containing functionality for Facet API.
 */
Drupal.facetapi = {}

/**
 * Applies the soft limit to facets in the block realm.
 */
Drupal.facetapi.applyLimit = function(settings) {
  if (settings.limit > 0 && !$('ul#' + settings.id).hasClass('facetapi-processed')) {
    // Only process this code once per page load.
    $('ul#' + settings.id).addClass('facetapi-processed');

    // Ensures our limit is zero-based, hides facets over the limit.
    var limit = settings.limit - 1;
    $('ul#' + settings.id).find('li:gt(' + limit + ')').hide();

    // Adds "Show more" / "Show fewer" links as appropriate.
    $('ul#' + settings.id).filter(function() {
      return $(this).find('li').length > settings.limit;
    }).each(function() {
      $('<a href="#" class="facetapi-limit-link"></a>').text(Drupal.t('Show more')).click(function() {
        if ($(this).prev().find('li:hidden').length > 0) {
          $(this).prev().find('li:gt(' + limit + ')').slideDown();
          $(this).addClass('open').text(Drupal.t('Show fewer'));
        }
        else {
          $(this).prev().find('li:gt(' + limit + ')').slideUp();
          $(this).removeClass('open').text(Drupal.t('Show more'));
        }
        return false;
      }).insertAfter($(this));
    });
  }
}

/**
 * Constructor for the facetapi redirect class.
 */
Drupal.facetapi.Redirect = function(href) {
  this.href = href;
}

/**
 * Method to redirect to the stored href.
 */
Drupal.facetapi.Redirect.prototype.gotoHref = function() {
  window.location.href = this.href;
}

/**
 * Replace an unclick link with a checked checkbox.
 */
Drupal.facetapi.makeCheckbox = function() {
  var $link = $(this);
  if (!$link.hasClass('facetapi-checkbox-processed')) {
    var active;
    if ($link.hasClass('facetapi-inactive')) {
      active = false;
    }
    else if ($link.hasClass('facetapi-active')) {
      active = true;
    }
    else {
      // Not a facet link.
      return;
    }
    var checkbox = active ? $('<input type="checkbox" class="facetapi-checkbox" checked="true" />') : $('<input type="checkbox" class="facetapi-checkbox" />');
    // Get the href of the link that is this DOM object.
    var href = $link.attr('href');
    redirect = new Drupal.facetapi.Redirect(href);
    checkbox.click($.proxy(redirect, 'gotoHref'));
    if (active) {
      // Add the checkbox, hide the link.
      $link.before(checkbox).hide();
    }
    else {
      $link.before(checkbox);
    }
    $link.removeClass('facetapi-checkbox').addClass('facetapi-checkbox-processed');
  }
}

/**
 * Taken from https://gist.github.com/2158219. All credits go to that url
 */
if(typeof jQuery.proxy === 'undefined'){
  jQuery.extend({
    proxy : function( fn, proxy, thisObject ) {
      if ( arguments.length === 2 ) {
        if ( typeof proxy === "string" ) {
          thisObject = fn;
          fn = thisObject[ proxy ];
          proxy = undefined;

        } else if ( proxy && !jQuery.isFunction( proxy ) ) {
          thisObject = proxy;
          proxy = undefined;
        }
      }

      if ( !proxy && fn ) {
        proxy = function() {
          return fn.apply( thisObject || this, arguments );
        };
      }

      // Set the guid of unique handler to the same of original handler, so it can be removed
      if ( fn ) {
        proxy.guid = fn.guid = fn.guid || proxy.guid || jQuery.guid++;
      }

      // So proxy can be declared as an argument
      return proxy;
    }
  });
}
