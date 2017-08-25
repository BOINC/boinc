<?php
/**
 * @file
 *
 * Display the box for rounded corners.
 *
 * - $content: The content of the box.
 */
?>
<div class="rounded-corner">
  <div class="wrap-corner">
    <div class="t-edge"><div class="l"></div><div class="r"></div></div>
    <div class="l-edge">
      <div class="r-edge clear-block">
        <?php print $content; ?>
      </div>
    </div>
    <div class="b-edge"><div class="l"></div><div class="r"></div></div>
  </div>
</div>
