
Context layouts
---------------
Context layouts provides a formalized way for themes to declare and switch
between page templates using Context. It is a continuation of an old Drupal
themer's trick to switch to something besides the standard `page.tpl.php` file
for a variety of special-case pages like the site frontpage, login page, admin
section, etc.


Requirements
------------
In order to use context layouts, your site must meet a few conditions:

- Context and Context layouts modules are enabled (`admin/build/modules`).
- You are using a theme which provides and has declared multiple layouts. (See
  "Example themes" for themes you can try.)


Basic usage
-----------
Once you have layouts enabled, you can have a context trigger the usage of a
particular layout in either the admin interface (`admin/build/context`) or
inline context editor. Different layouts may have fewer or greater regions than
the default page template, so adjust your blocks accordingly.


Supporting context layouts in your theme
----------------------------------------
You can add layouts support to your theme by declaring additional layouts in
your theme's info file. Here is an example:

`example.info`

    name = "Example"
    description = "Example theme"
    core = "6.x"
    engine = "phptemplate"

    regions[left] = "Left sidebar"
    regions[right] = "Right sidebar"
    regions[content] = "Content"
    regions[footer] = "Footer"

    ; Layout: Default
    layouts[default][name] = "Default"
    layouts[default][description] = "Simple two column page."
    layouts[default][template] = "page"
    layouts[default][regions][] = "content"
    layouts[default][regions][] = "right"

    ; Layout: Columns
    layouts[columns][name] = "3 columns"
    layouts[columns][description] = "Three column page."
    layouts[columns][stylesheet] = "layout-columns.css"
    layouts[columns][template] = "layout-columns"
    layouts[columns][regions][] = "left"
    layouts[columns][regions][] = "content"
    layouts[columns][regions][] = "right"
    layouts[columns][regions][] = "footer"

Each layout is declared under `layouts` with the key as the identifier that will
be used by context for this layout. You may use any reasonable machine name for
each layout, but note that `default` is special -- it will be the default layout
for your theme if no other layout is specified.

The following keys can be declared for each layout:

- `name`: The human readable name for this layout, shown in the admin UI.
- `description`: A short description of your layout, same as above.
- `stylesheet`: A stylesheet to be included with the layout. Optional.
- `template`: The name of the template file for this layout, without the
  `.tpl.php` extension.
- `region`: An array of regions supported by this layout. Note that any of the
  regions listed here **must also be declared** in the standard theme `regions`
  array.


Example themes
--------------
- Cube, a subtheme included with [Rubik][1] provides a variety of layouts.
- [Ginkgo][2] the default theme included with Open Atrium.

[1]: http://github.com/developmentseed/rubik/downloads
[2]: http://github.com/developmentseed/ginkgo/downloads
