<?php
/**
 * @file
 *
 * Display the box for rounded corners.
 *
 * - $content: The content of the box.
 * - $classes: The classes that must be applied to the top divs.
 */
?>
<div class="rounded-shadow <?php print $classes ?>">
  <div class="rounded-shadow-background">
    <div class="rounded-shadow-wrap-corner">
      <div class="rounded-shadow-top-edge">
        <div class="rounded-shadow-left"></div>
        <div class="rounded-shadow-right"></div>
      </div>
      <div class="rounded-shadow-left-edge">
        <div class="rounded-shadow-right-edge clear-block">
          <?php print $content; ?>
        </div>
      </div>
      <div class="rounded-shadow-bottom-edge">
      <div class="rounded-shadow-left"></div><div class="rounded-shadow-right"></div>
      </div>
    </div>
  </div>
</div>
