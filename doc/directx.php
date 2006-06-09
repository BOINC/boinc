<?php
require_once("docutil.php");
page_head("DirectX version requirements");
echo "
Microsoft Windows has a component called DirectX
that manages graphics and sound.
If you are running BOINC on Windows
and you don't have a recent version (9.0c or later) of DirectX,
applications may crash with error messages like
<pre>
There are no child processes to wait for. (0x80) - exit code 128 (0x80)
</pre>
If this is happening, or you having other problems involving graphics,
you may want to
<a href=http://www.webcamsoft.com/en/directx.html>check your DirectX version</a>.

<p>
If your version of DirectX is older than 9.0c
(or if you are unable to check your version)
then you may want to
<a href=http://www.microsoft.com/downloads/details.aspx?FamilyId=2DA43D38-DB71-4C1B-BC6A-9B6652CD92A3&displaylang=en>download the current version of DirectX</a>.

";
page_tail();
?>
