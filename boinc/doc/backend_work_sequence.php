<?
require_once("docutil.php");
page_head("Back ends and work sequences");
echo "
<p>
Project that use work sequences will need two additional processes:
<p>
<table border=1 cellpadding=8>
<tr>
<th>Component</th>
<th>BOINC-supplied part</th>
<th>project-supplied part</th>
</tr>
<tr>
<td valign=top><b>Work sequence relocater</b>:
detects work sequences whose hosts have failed,
and relocates them to other hosts.</td>
<td valign=top>A program <b>seq_relocate</b></td>
<td valign=top>Some parameters used by seq_relocate</td>
</tr>
<tr>
<td valign=top><b>Work sequence validation and accounting</b>:
Similar to result validation, but for work sequences.
</td>
<td valign=top>A program <b>seq_validate</b></td>
<td valign=top>An application-specific function,
linked with <b>seq_validate</b>, that compares sets of redundant results.</td>
</tr>
</table>
";
page_tail();
?>
