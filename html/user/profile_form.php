<?php

echo "
    <form method=post enctype="multipart/form-data" action=profile_action.php>
    <br>
    About ".PROJECT.": <textarea name=project_text cols=80 rows=20>
    <br>
    About yourself: <textarea name=self_text cols=80 rows=20>
    <br>
    Picture:
    <input name=picture type=file size=40>
    </form>
";

?>
