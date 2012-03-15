// $Id: context_layouts_reaction_block.js,v 1.1.2.1 2009/12/14 22:36:12 yhahn Exp $

Drupal.behaviors.contextLayoutsReactionBlock = function(context) {
  // ContextBlockForm: Init.
  $('.context-blockform-layout:not(.contextLayoutsProcessed)').each(function() {
    $(this).addClass('contextLayoutsProcessed');
    $(this).change(function() {
      var layout = $(this).val();
      if (Drupal.settings.contextLayouts.layouts[layout]) {
        $('#context-blockform td.blocks table').hide();
        $('#context-blockform td.blocks div.label').hide();
        for (var key in Drupal.settings.contextLayouts.layouts[layout]) {
          var region = Drupal.settings.contextLayouts.layouts[layout][key];
          $('.context-blockform-regionlabel-'+region).show();
          $('#context-blockform-region-'+region).show();
        }
        if (Drupal.contextBlockForm) {
          Drupal.contextBlockForm.setState();
        }
      }
    });
    $(this).change();
  });
};
