<?php
?>
<div class='clear-block features-components'>
  <div class='column'>
    <div class='info'>
      <h3><?php print $name ?></h3>
      <div class='description'><?php print $description ?></div>
      <?php print $dependencies ?>
    </div>
  </div>
  <div class='column'>
    <div class='components'>
      <?php print $components ?>
      <?php if (!empty($key)): ?>
        <div class='clear-block features-key'><?php print theme('links', $key) ?></div>
      <?php endif; ?>
      <?php if (!empty($buttons)): ?>
        <div class='buttons clear-block'><?php print $buttons ?></div>
      <?php endif; ?>
    </div>
  </div>
  <?php print $form ?>
</div>