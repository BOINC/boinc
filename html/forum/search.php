<?php
require_once('../include.php');
require_once('forum.inc');
doHeader('Forum');

/* sanitize variable */
if (array_key_exists('id', $_GET))
	$_GET['id'] = stripslashes(strip_tags($_GET['id']));

$categories = getCategories();
?>

<p>
	<span class="title">Search</span>
	<br><a href="index.php"><?php echo $cfg['sitename'] ?> Forum</a>
</p>

<p style="text-align:center">
	<form action="post.php?id=<?php echo $_GET['id'] ?>" method="post">
		<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
			<tr>
				<th colspan="2">Search in Forum</th>
			</tr>
			<tr>
				<td style="width:150px"><b>Topic</b></td>
				<td><input type="text" name="topic" size="62"></td>
			</tr>
			<tr>
				<td style="vertical-align:top"><b>Message content</b></td>
				<td><textarea name="content" rows="12" cols="54"></textarea></td>
			</tr>
			<tr>
				<td colspan="2" style="text-align:center">
					<input type="submit" value="Post message">
				</td>
			</tr>
		</table>
	</form>
</p>

<?php
doFooter();
?>