<div class='context-editor clear-block'>
  <?php print drupal_render($form) ?>
  <div class='contexts'>
    <?php foreach (element_children($contexts) as $context): ?>
      <div class='context-editable' id='context-editable-<?php print $context ?>'><?php print drupal_render($contexts[$context]) ?></div>
    <?php endforeach; ?>
    <?php print drupal_render($contexts) ?>
  </div>
  <div class='buttons'><?php print drupal_render($buttons) ?></div>
</div>
