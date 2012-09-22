ZEN'S STYLESHEETS
-----------------

Don't panic!

There are 28 CSS files in this sub-theme, but its not as bad as it first seems:
- The drupal6-reference.css is just a reference file and isn't used directly by
  your sub-theme. See below.
- There are 9 CSS files whose names end in "-rtl.css". Those are CSS files
  needed to style content written in Right-to-Left languages, such as Arabic and
  Hebrew. If your website doesn't use such languages, you can safely delete all
  of those CSS files.
- If you aren't using this theme while doing wireframes of the functionality of
  your sub-theme, you can remove wireframes.css from your sub-theme's .info file
  and delete the file as well.

That leaves just 17 CSS files. (Okay, still quite a few, but better than 28.)

- Instead of one monolithic stylesheet, your sub-theme's CSS files are organized
  into several smaller stylesheets that are grouped to allow cascading across
  common Drupal template files.
- The order of the stylesheets is designed to allow CSS authors to use the
  lowest specificity possible to achieve the required styling.


ORDER AND PURPOSE OF DEFAULT STYLESHEETS
----------------------------------------

First off, if you find you don't like this organization of stylesheets, you are
free to change it; simply edit the stylesheet declarations in your sub-theme's
.info file. This structure was crafted based on several years of experience
theming Drupal websites.

- html-reset.css:
  This is the place where you should set the default styling for all HTML
  elements and standardize the styling across browsers. If you prefer a specific
  reset method, feel free to add it.

- layout-fixed.css:
- layout-liquid.css:
  Zen's default layout is based on the Zen Columns layout method. The
  layout-fixed.css file is used by default and can be swapped with the
  layout-liquid.css file. These files are designed to be easily replaced. If you
  are more familiar with a different CSS layout method, such as Blueprint or
  960.gs, you can replace these files with your choice of layout CSS file.

- page-backgrounds.css:
  Layered backgrounds across scattered divs can be easier to manage if they are
  centralized in one location.

- tabs.css:
- messages.css:
  While most of the CSS rulesets in your sub-theme are guidelines without any
  actual properties, the tabs and messages stylesheets contain actual styling
  for Drupal tabs and Drupal status messages; two common Drupal elements that
  are often neglected by site desiners. Zen provides some basic styling which
  you are free to use or to rip out and replace.

- pages.css:
  Page styling for the markup in the page.tpl.php template.

- blocks.css:
  Block styling for the markup in the block.tpl.php template.

- navigation.css:
  The styling for your site's menus can get quite bulky and its easier to see
  all the styles if they are grouped together rather then across the
  header/footer sections of pages.css and in blocks.css.

- views-styles.css:
  Views styling for the markup in various views templates. You'll notice this
  stylesheet isn't called "views.css" as that would override (remove) the Views
  module's stylesheet.

- nodes.css:
  Node styling for the markkup in the node.tpl.php template.

- comments.css:
  Comment styling for the markup in the comment-wrapper.tpl.php and
  comments.tpl.php templates.

- forms.css:
  Form styling for the markup in various Drupal forms.

- fields.css:
  Field styling for the markup produced by theme_field().

- print.css:
  The print styles for all markup.

- ie.css:
- ie6.css:
  The Internet Explorer stylesheets are added via conditional comments. Many CSS
  authors find using IE "conditional stylesheets" much easier then writing
  rulesets with CSS hacks that are known to only apply to various versions of
  IE. An alternative method presented by Paul Irish can be found at
  http://paulirish.com/2008/conditional-stylesheets-vs-css-hacks-answer-neither/

In these stylesheets, we have included all of the classes and IDs from this
theme's tpl.php files. We have also included many of the useful Drupal core
styles to make it easier for theme developers to see them.


DRUPAL CORE'S STYLESHEETS
-------------------------

Many of these styles are over-riding Drupal's core stylesheets, so if you remove
a declaration from them, the styles may still not be what you want since
Drupal's core stylesheets are still styling the element. See the
drupal6-reference.css file for a complete list of all Drupal 6.x core styles.

In addition to the style declarations in these stylesheets, other Drupal styles
that you might want to override or augment are those for:

  Book Navigation  See line 74  of drupal6-reference.css file
  Forum            See line 197 of drupal6-reference.css file
  Menus            See line 667 of drupal6-reference.css file
  News Aggregator  See line 20  of drupal6-reference.css file
  Polls            See line 287 of drupal6-reference.css file
  Search           See line 320 of drupal6-reference.css file
  User Profiles    See line 945 of drupal6-reference.css file


INTERNET EXLORER HATES YOU
--------------------------

All versions of IE limit you to 31 stylesheets total. What that means is that
only the first 31 stylesheets will load, ignoring the others. So you'll have
missing styles in IE7 and later and a broken layout in IE6.

This is a known bug in IE: http://support.microsoft.com/kb/262161

Please read http://john.albin.net/css/ie-stylesheets-not-loading for the gory
details.
