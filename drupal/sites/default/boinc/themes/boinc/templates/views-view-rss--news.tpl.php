<?php
/**
 * @file views-view-rss.tpl.php
 * Default template for feed displays that use the RSS style.
 *
 * @ingroup views_templates
 */

 // Set the feed description. Note that the title is set in the News view...
 // that works there, this works here. Setting title here does not set the
 // title in the RSS auto discovery link on the front page, and setting the
 // description in the view does not allow the site_name variable.
 $site_name = variable_get('site_name', 'Drupal-BOINC');
 $description = bts('The latest news from the @site_name project',
    array('@site_name' => $site_name), NULL, 'boinc:rss-feed-description');

?>
<?php print "<?xml"; ?> version="1.0" encoding="utf-8" <?php print "?>"; ?>
<rss version="2.0" xml:base="<?php print $link; ?>"<?php print $namespaces; ?>>
  <channel>
    <title><?php print $title; ?></title>
    <link><?php print $link; ?></link>
    <description><?php print $description; ?></description>
    <language><?php print $langcode; ?></language>
    <?php print $channel_elements; ?>
    <?php print $items; ?>
  </channel>
</rss>
