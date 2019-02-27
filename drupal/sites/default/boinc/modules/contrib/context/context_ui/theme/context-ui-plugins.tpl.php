<div class='context-plugins clear-block'>
  <div class='context-plugin-forms'>
    <?php foreach (element_children($form['plugins']) as $plugin): ?>
      <div class='context-plugin-form context-plugin-form-<?php print $plugin ?>'>
        <?php print drupal_render($form['plugins'][$plugin]) ?>
      </div>
    <?php endforeach; ?>
  </div>

  <div class='context-plugin-selector'>
    <div class='context-plugin-info'>
      <h2 class='context-plugin-title'><?php print $title ?></h2>
      <div class='description'><?php print $description ?></div>
      <?php print drupal_render($form['selector']) ?>
      <?php print drupal_render($form['state']) ?>
    </div>
    <?php print theme('links', $plugins, array('class' => 'context-plugin-list')) ?>
  </div>

  <?php print drupal_render($form) ?>

</div>
