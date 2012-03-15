$Id: README.txt,v 1.1.4.3 2010/02/09 20:20:19 yhahn Exp $

Context UI
----------
Context UI provides an administrative interface for managing and editing
Contexts. It is not necessary for the proper functioning of contexts once they
are built and can be turned off on most production sites.


Requirements
------------
- Context, Context UI modules enabled (`admin/build/modules`)
- [jQuery UI 1.x][1] and [Admin 2.x][2] to use the inline context editor.
  Optional.


Basic usage
-----------
As a site administrator you can manage your site's contexts at
`admin/build/context`. The main page will show you a list of the contexts on the
site and give you some options for managing each context.

When editing or adding a new context, you will be presented with a form to
manage some basic information about the context and then alter its conditions
and reactions.

- `name`: The name of your context. This is the main identifier for your context
  and cannot be changed after you've created it.
- `description`: A description or human-readable name for your context. This is
  displayed in the inline editor if available instead of the name.
- `tag`: A category for organizing contexts in the administrative context
  listing. Optional.

**Conditions**

When certain conditions are true, your context will be made active. You can
customize the conditions that trigger the activation of your context.

- **Condition mode**: you can choose to have your context triggered if **ANY**
  conditions are met or only active when **ALL** conditions are met.
- **Adding/removing conditions**: you can add or remove to the conditions on
  your context using the conditions dropdown.
- **Individual settings**: most conditions provide a simple form for selecting
  individual settings for that condition. For example, the node type condition
  allows you to choose which node types activate the context.

**Reactions**

Whenever a particular context is active, all of its reactions will be run.
Like conditions, reactions can be added or removed and have settings that can
be configured.


Using the inline editor
-----------------------
If you have installed jQuery UI and Admin, you can enable the inline context
editor:

1. As an administrative user go to `admin/settings/admin`.
2. Check the 'Context editor' block and save.
3. When viewing a page with one or more active contexts, you will see the
  `Context editor` in the admin toolbar.
4. You can use the context editor to adjust the conditions under which each
  context is active and alter its reactions.


[1]: http://drupal.org/project/jquery_ui
[2]: http://drupal.org/project/admin
