; $Id: README.txt,v 1.2.4.1 2009/06/10 16:23:06 weitzman Exp $

The og_actions module is a collection of actions. Their most common use case is in conjunction with the Views Bulk Operations module. Together, these modules make a terrific admin dashboard for putitng content into and out of groups, and adding removing members from groups. 

Requirements:
og.module

Suggested:
trigger.module OR
workflow.module OR
rules.module OR
views_bulk_options.module

Actions
There are 11 actions in this module. Here is a sampling.

Non-configurable actions:
-------------------------
"Make the node publicly visible" -- This action will make the node visible to the public. This has the same effect as checking the "Public" box on node creation.

"Make the node private to its groups" -- This action has the opposite of "Make the node publicly visible."  This action is equivalent to unchecking the "public" box on node editing.

"Remove the node from all groups" -- This is action will remove all group ties to this node. This will occur even if you have selected "Audience Required" in your organic group settings.

Configurable actions:
---------------------
"Add the node to the specified group..." -- This action allows an administrator to select a group and add nodes to it. Any currently published, organic group node type will be listed. In large lists, this could potentially be a very long list. This action could potentially add a node that is in the excluded content type list.

"Remove the node from the specified group..." -- This action removes the node from the selected groups. Potentially, it could remove the last group from the node, even if "Audience Required" is selected in organic groups.

Notes
If you are using workflow.module with this module, you may find that your actions are not taking effect during node creation. To fix this bug use the following SQL query in your database:

UPDATE SYSTEM SET weight = 10 WHERE name = "workflow"

This will ensure that og adds its data to the node before workflow attempts to act on it. You could alternatively set workflow's weight in the system table using the weight module.