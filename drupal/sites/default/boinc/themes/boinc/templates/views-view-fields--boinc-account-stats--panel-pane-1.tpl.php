<ul class="stats tab-list">
  <li class="first tab">
    <label><?php echo $fields['total_credit']->label; ?>:</label>
    <span class="field-content"><?php echo $fields['total_credit']->content; ?></span>
  </li>
  <li class="last tab">
    <label><?php echo $fields['expavg_credit']->label; ?>:</label>
    <span class="field-content"><?php echo $fields['expavg_credit']->content; ?></span>
  </li>
  <?php /*
  <li class="tab">
    <label><?php echo $fields['nothing_4']->label; ?></label>
    <span class="field-content"><?php echo $fields['nothing_4']->content; ?></span>
  </li>
  */ ?>
  <li class="first alt tab">
    <label><?php echo $fields['nothing']->label; ?></label>
    <span class="field-content"><?php echo $fields['nothing']->content; ?></span>
  </li>
</ul>
<div class="chart"><?php echo $fields['phpcode']->content; ?></div>