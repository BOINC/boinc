<?
require_once("docutil.php");
page_head("Preferences implementation");
echo "
<pre>
In database:
    user.global_prefs: XML, within <lt;global_preferences> tags
        may be empty; nonempty only if user has actually seen
        always includes <lt;mod_time> element
        includes <lt;source_project>, <lt;source_scheduler> elements if
            prefs came from another project
    user.project_prefs: XML, within <lt;project_preferences> tags
        always present.
        contains at least <lt;resource_share> element

In client:
    global_prefs.xml (present ONLY if have obtained from a server)
        same as user.global_prefs,
        but the following is inserted at start:
            <lt;source_project>
            <lt;source_scheduler>
        stored in memory:
            in parsed form (as struct)
    account_XXX.xml
        same as user.project_prefs, but with the following added:
            <lt;master_url>
            <lt;authenticator>
        stored in memory:
            in PROJECT struct
                master_url, authenticator, resource share parsed;
                project_specific_prefs raw XML (with enclosing tags)


RPC request:
    includes global_prefs.xml if it's there

RPC handling and reply:
    always send project prefs
    if request message includes global prefs
        if missing in DB, or request copy is newer:
            install in DB
        else if DB copy is newer
            include DB copy in reply
    else
        if present in DB, include in reply

handling of RPC reply
    if includes global prefs
        insert <lt;source_project>, <lt;source_scheduler> elements if missing
        write to global_prefs.xml
        parse into memory
    project prefs
        insert <lt;master_url>, <lt;authenticator> elements,
        write to account_XXX.xml file
        parse; update resource share, project_specific_prefs in PROJECT struct
</pre>
";
page_tail();
?>
