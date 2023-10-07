<?php
// $Id: page.tpl.php,v 1.26.2.3 2010/06/26 15:36:04 johnalbin Exp $

/**
 * @file
 * Theme implementation to display a single Drupal page.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $css: An array of CSS files for the current page.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/garland.
 * - $is_front: TRUE if the current page is the front page. Used to toggle the mission statement.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Page metadata:
 * - $language: (object) The language the site is being displayed in.
 *   $language->language contains its textual representation.
 *   $language->dir contains the language direction. It will either be 'ltr' or 'rtl'.
 * - $head_title: A modified version of the page title, for use in the TITLE tag.
 * - $head: Markup for the HEAD section (including meta tags, keyword tags, and
 *   so on).
 * - $styles: Style tags necessary to import all CSS files for the page.
 * - $scripts: Script tags necessary to load the JavaScript files and settings
 *   for the page.
 * - $classes: String of classes that can be used to style contextually through
 *   CSS. It should be placed within the <body> tag. When selecting through CSS
 *   it's recommended that you use the body tag, e.g., "body.front". It can be
 *   manipulated through the variable $classes_array from preprocess functions.
 *   The default values can be one or more of the following:
 *   - front: Page is the home page.
 *   - not-front: Page is not the home page.
 *   - logged-in: The current viewer is logged in.
 *   - not-logged-in: The current viewer is not logged in.
 *   - node-type-[node type]: When viewing a single node, the type of that node.
 *     For example, if the node is a "Blog entry" it would result in "node-type-blog".
 *     Note that the machine name will often be in a short form of the human readable label.
 *   - page-views: Page content is generated from Views. Note: a Views block
 *     will not cause this class to appear.
 *   - page-panels: Page content is generated from Panels. Note: a Panels block
 *     will not cause this class to appear.
 *   The following only apply with the default 'sidebar_first' and 'sidebar_second' block regions:
 *     - two-sidebars: When both sidebars have content.
 *     - no-sidebars: When no sidebar content exists.
 *     - one-sidebar and sidebar-first or sidebar-second: A combination of the
 *       two classes when only one of the two sidebars have content.
 * - $node: Full node object. Contains data that may not be safe. This is only
 *   available if the current page is on the node's primary url.
 * - $menu_item: (array) A page's menu item. This is only available if the
 *   current page is in the menu.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 * - $mission: The text of the site mission, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $search_box: HTML to display the search box, empty if search has been disabled.
 * - $primary_links (array): An array containing the Primary menu links for the
 *   site, if they have been configured.
 * - $secondary_links (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title: The page title, for use in the actual HTML content.
 * - $messages: HTML for status and error messages. Should be displayed prominently.
 * - $tabs: Tabs linking to any sub-pages beneath the current page (e.g., the
 *   view and edit tabs when displaying a node).
 * - $help: Dynamic help text, mostly for admin pages.
 * - $content: The main content of the current page.
 * - $feed_icons: A string of all feed icons for the current page.
 *
 * Footer/closing data:
 * - $footer_message: The footer message as defined in the admin settings.
 * - $closure: Final closing markup from any modules that have altered the page.
 *   This variable should always be output last, after all other dynamic content.
 *
 * Helper variables:
 * - $classes_array: Array of html class attribute values. It is flattened
 *   into a string within the variable $classes.
 *
 * Regions:
 * - $content_top: Items to appear above the main content of the current page.
 * - $content_bottom: Items to appear below the main content of the current page.
 * - $navigation: Items for the navigation bar.
 * - $sidebar_first: Items for the first sidebar.
 * - $sidebar_second: Items for the second sidebar.
 * - $header: Items for the header region.
 * - $footer: Items for the footer region.
 * - $page_closure: Items to appear below the footer.
 *
 * The following variables are deprecated and will be removed in Drupal 7:
 * - $body_classes: This variable has been renamed $classes in Drupal 7.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see zen_preprocess()
 * @see zen_process()
 */
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="<?php print $language->language; ?>" lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">

<head>
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
  <?php if ($is_front): ?>
    <?php
      // Insert the scheduler tags
      $scheduler_tags = boinc_get_scheduler_tags();
      if ($use_old_tags = variable_get('boinc_scheduler_tag_format_old', 1)) {
        print "<!-- Project scheduling servers -->\n";
        print "<!--\n";
        foreach ($scheduler_tags as $tag) {
          print "  <scheduler>{$tag}</scheduler>\n";
        }
        print "-->\n";
      }
      if ($use_new_tags = variable_get('boinc_scheduler_tag_format_new', 1)) {
        foreach ($scheduler_tags as $tag) {
          print "  <link rel=\"boinc_scheduler\" href=\"{$tag}\">\n";
        }
      }
    ?>
  <?php endif; ?>
</head>
<body class="<?php print $classes; ?>">

  <?php if ($primary_links): ?>
    <div id="skip-link"><a href="#main-menu"><?php print t('Jump to Navigation'); ?></a></div>
  <?php endif; ?>

  <div id="page-wrapper"><div id="page">

    <div id="header-wrapper" class="shadow">

    <div id="header"><div class="section clearfix">

      <?php if ($logo): ?>
        <a href="<?php print $front_page; ?>" title="<?php print bts('Home', array(), NULL, 'boinc:menu-link'); ?>" rel="home" id="logo"><img src="<?php print $logo; ?>" alt="<?php print bts('Home', array(), NULL, 'boinc:menu-link'); ?>" /></a>
      <?php endif; ?>

      <?php if ($site_name || $site_slogan): ?>
        <div id="name-and-slogan">
          <?php if ($site_name): ?>
            <?php if ($title): ?>
              <div id="site-name"><strong>
                <a href="<?php print $front_page; ?>" title="<?php print bts('Home', array(), NULL, 'boinc:menu-link'); ?>" rel="home"><span><?php print strtoupper($site_name); ?></span></a>
              </strong></div>
            <?php else: // Use h1 when the content title is empty  ?>
              <h1 id="site-name">
                <a href="<?php print $front_page; ?>" title="<?php print bts('Home', array(), NULL, 'boinc:menu-link'); ?>" rel="home"><span><?php print strtoupper($site_name); ?></span></a>
              </h1>
            <?php endif; ?>
          <?php endif; ?>

          <?php if ($site_slogan): ?>
            <div id="site-slogan"><?php print $site_slogan; ?></div>
          <?php endif; ?>
        </div> <!-- /#name-and-slogan -->
      <?php endif; ?>

      <?php if ($search_box): ?>
        <div id="search-box"><?php print $search_box; ?></div>
      <?php endif; ?>

      <?php print $header; ?>

    </div></div> <!-- /.section, /#header -->

    <?php if ($primary_links || $navigation): ?>
      <div id="navigation"><div class="section clearfix">
        <div id="main-menu">
          <?php print theme(array('links__system_main_menu', 'links'), $primary_links,
            array(
              'id' => 'main-menu',
              'class' => 'links clearfix',
            ),
            array(
              'text' => t('Main menu'),
              'level' => 'h2',
              'class' => 'element-invisible',
            ));
          ?>
        </div>

        <div id="action-links">
          <ul><li class="first">
            <?php
              global $user;
              if ($user->uid):
                echo '<a href="' . url('logout') . '"><span class="secondary-link tab">' . bts('Logout', array(), NULL, 'boinc:menu-link') . '</span></a>';
              else:
                echo '<a href="' . url('user/login', array('query' => drupal_get_destination()) ) . '"><span class="secondary-link tab">' . bts('Login', array(), NULL, 'boinc:menu-link') . '</span></a>';
              endif;
            ?>
            </li>
            <?php if (module_exists('global_search') OR module_exists('global_search_solr')): ?>
              <li class="last"><a class="search" href="<?php print url('search/site') ?>"><span class="tab"><?php print bts('search', array(), NULL, 'boinc:menu-link'); ?></span></a></li>
            <?php endif; ?>
          </ul>
        </div>

        <div id="sub-menu">
          <?php print theme(array('links__system_secondary_menu', 'links'), $secondary_links,
            array(
              'id' => 'secondary-menu',
              'class' => 'links clearfix',
            ),
            array(
              'text' => t('Secondary menu'),
              'level' => 'h2',
              'class' => 'element-invisible',
            ));
          ?>
        </div>

        <?php if (isset($tertiary_links)) : ?>
          <div id="sub-menu">
            <?php print theme(array('links__system_tertiary_menu', 'links'), $tertiary_links,
              array(
                'id' => 'tertiary-menu',
                'class' => 'links clearfix',
              ),
              array(
                'text' => t('Tertiary menu'),
                'level' => 'h2',
                'class' => 'element-invisible',
              ));
            ?>
          </div>
        <?php endif; ?>

        <div id="navigation-mmt">
          <div id="hamburger-menu" class="block">
            <?php print render($menu_tree_onlyactive); ?>
            <?php print $action_links; ?>
          </div>
        </div>

      </div></div> <!-- /.section, /#navigation -->
    <?php endif; ?>

    </div> <!-- /.shadow -->

    <div id="main-wrapper"><div id="main" class="clearfix<?php if ($primary_links || $navigation) { print ' with-navigation'; } ?>">

      <?php print $messages; ?>

    <?php if ($tabs): ?>
      <?php
        $active_menu_item = '';
        $current_path = $_GET['q'];
        $heading_overrides = array(
          'join' => bts('Join now', array(), NULL, 'boinc:front-page'),
          'user/login' => bts('Account', array(), NULL, 'boinc:user-account'),
          'user/password' => bts('Account', array(), NULL, 'boinc:user-account'),
        );
        if (isset($heading_overrides[$current_path])) {
          $active_menu_item = $heading_overrides[$current_path];
        }
        else {
          $target_menu = ($secondary_links) ? $secondary_links : $primary_links;
          foreach ($target_menu as $key => $entry) {
            if (strstr($key, 'active')) {
              $active_menu_item = $target_menu[$key]['title'];
              break;
            }
          }
        }
      ?>
      <div class="tabs container shadow">
      <?php if ($active_menu_item): ?>
        <h1><?php print $active_menu_item; ?></h1>
      <?php endif; ?>
      <?php print $tabs; ?>
      </div>
    <?php endif; ?>

      <?php $class_list = explode(' ', $classes); ?>
      <div id="content" class="column">

        <div class="section<?php echo (!array_intersect(array('page-panels','section-users'), $class_list)) ? ' framing container shadow' : ''; ?>">

          <?php if ($mission): ?>
            <div id="mission"><?php print $mission; ?></div>
          <?php endif; ?>

          <?php print $highlight; ?>

          <?php if (!empty($title)): ?>
            <h1 class="title"><?php print $title; ?></h1>
          <?php endif; ?>
          <?php print $help; ?>

          <?php print $content_top; ?>

          <div id="content-area">
            <?php print $content; ?>
          </div>

        </div> <!-- /.section -->

        <?php if ($content_bottom): ?>
          <div class="section bottom<?php echo (!array_intersect(array('page-panels','section-users'), $class_list)) ? ' framing container shadow' : ''; ?>">

            <?php print $content_bottom; ?>

            <?php if ($feed_icons AND !$is_front): ?>
              <div class="feed-icons"><?php print $feed_icons; ?></div>
            <?php endif; ?>

          </div> <!-- /.section.bottom -->
        <?php endif; ?>

      </div> <!-- /#content -->

      <?php print $sidebar_first; ?>

      <?php print $sidebar_second; ?>

    </div></div> <!-- /#main, /#main-wrapper -->

    <?php $footer_links = menu_navigation_links('menu-footer-links',0); ?>
    <?php if ($footer || $footer_message || $footer_links): ?>
      <div id="footer"><div class="section">

        <div id="footer-menu">

          <?php print theme(array('links__system_secondary_menu', 'links'), $footer_links,
            array(
              'id' => 'footer-links',
              'class' => 'links clearfix',
            ),
            array(
              'text' => t('Footer links'),
              'level' => 'h2',
              'class' => 'element-invisible',
            ));
          ?>

          <ul id="server-status" class="tab-list">

            <li class="first tab">
              <?php print l(bts('Applications', array(), NULL, 'boinc:footer-link:-1:ignoreoverwrite'), $app_list_url); ?>
            </li>
            <li class="last tab">
              <?php print l(bts('Server status', array(), NULL, 'boinc:footer-link'), $server_status_url); ?>
            </li>
            <!--<li class="first tab">Server status</li>
            <li class="tab">
              <ul id="server-status-highlights">
                <li class="first good">Work available</li>
                <li class="good">Website</li>
              </ul>
            </li>
            <li class="last tab"><a href="status">More</a></li>-->

          </ul>
          <div class="clearfix"></div>

        </div>

        <div id="footer-info">

          <div id="language"
            style="background: url(<?php print $flag_path; ?>) no-repeat right;">
              <?php print bts('Language', array(), NULL, 'boinc:footer-link:-1:ignoreoverwrite'); ?>
          </div>

          <?php if ($footer_message): ?>
            <div id="footer-message"><?php print $footer_message; ?></div>
          <?php endif; ?>

          <?php if (user_access('create page content') OR user_access('create news content')): ?>
            <div id="content-management-links">
              <?php print l(bts('Create content', array(), NULL, 'boinc:footer-link'), 'node/add'); ?>
            </div>
          <?php endif; ?>

          <div class="clearfix"></div>

          <?php print $footer; ?>
        </div>

      </div></div> <!-- /.section, /#footer -->
    <?php endif; ?>

  </div></div> <!-- /#page, /#page-wrapper -->

  <?php print $page_closure; ?>

  <?php print $closure; ?>

</body>
</html>
