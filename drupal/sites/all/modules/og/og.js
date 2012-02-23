// $Id: og.js,v 1.9.2.3 2010/01/24 14:44:56 amitaibu Exp $

// Content type form
Drupal.behaviors.og_content_type = function() {
  // Disable the group limit textarea if the content type is not a standard group post
  $('input[name="og_content_type_usage"]').click(function(){
    if (!$('#edit-og-content-type-usage-group-post-standard').attr('checked')) {
      $('#edit-og-max-groups').attr('disabled','disabled');
    } else {
      $('#edit-og-max-groups').removeAttr('disabled');
    }
  });
  
  // Initial check to see if content type is standard group post
  if (!$('#edit-og-content-type-usage-group-post-standard').attr('checked')) {
    $('#edit-og-max-groups').attr('disabled','disabled');
  };
}


Drupal.verticalTabs = Drupal.verticalTabs || {};

Drupal.verticalTabs.og_nodeapi = function() {
  var values = [];
  $('.vertical-tabs-og_nodeapi #edit-og-groups :selected')
    .each(function (i, selected) {
      values[i] = $(selected).text();
    });

  return values.join(', ');
}

Drupal.verticalTabs.og = function() {
  var type = $('.vertical-tabs-og input[type=radio]:checked').val();
  
  switch (type) {
    case 'group':
      return Drupal.t('Group node');
      break;
      
    case 'omitted':
      return Drupal.t('May not be posted into a group.');
      break;
      
    case 'group_post_standard':
      return Drupal.t('Standard group post');
      break;
      
    case 'group_post_wiki':
      return Drupal.t('Wiki group post');
      break;
  }
}
