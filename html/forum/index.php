<?php
require_once('../include.php');
require_once('forum.inc');

if (!empty($_GET['lang'])) {
	$sql = "SELECT * FROM lang WHERE id = ".$_GET['lang']." LIMIT 1";
	$lang = sql_fetch_array(sql_query($sql));
	$_SESSION['lang']['id'] = $lang['id'];
	$_SESSION['lang']['charset'] = $lang['charset'];
}

doHeader('Forum', 'forum.css');
?>

<p class="title"><?php echo $cfg['sitename'] ?> Forum</p>
<p style="text-align:right">
	<form action="index.php" method="get">
		Language filter:
		<select name="lang">
			<?php
			$sql = "SELECT * FROM lang ORDER BY name";
			$langs = sql_query($sql);
			while ($lang = sql_fetch_array($langs)):
				?>
				<option value="<?php echo $lang['id'] ?>"<?php echo ($lang['id']==$_SESSION['lang']['id'])?' selected':'' ?>><?php echo $lang['name'] ?> (<?php echo $lang['charset'] ?>)</option>
				<?php
			endwhile;
			?>
		</select>
		<input type="submit" value="Go">
	</form>
</p>
<p style="text-align:center">
	<table class="content" border="0" cellpadding="5" cellspacing="0" width="100%">
		<tr>
			<th>Forum</th>
			<th style="width: 60px">Threads</th>
			<th style="width: 60px">Posts</th>
			<th style="width: 160px">Last Post</th>
		</tr>
		<?php

		$categories = getCategories();
		while ($category = getNextCategory($categories)):
			?>
			<tr class="subtitle">
				<td colspan="4"><?php echo $category->name ?></td>
			</tr>
			<?php
			$forums = $category->getForums();
			while ($forum = getNextForum($forums)):
				?>
				<tr style="font-size:8pt; text-align:right">
					<td style="text-align:left">
						<span style="font-size:10pt; font-weight:bold"><a href="forum.php?id=<?php echo $forum->id ?>"><?php echo $forum->title ?></a></span>
						<br><?php echo $forum->description ?>
					</td>
					<td><?php echo $forum->threads ?></td>
					<td><?php echo $forum->posts ?></td>
					<td><?php echo date('D M j, Y g:i a', $forum->timestamp) ?></td>
				</tr>
				<?php
			endwhile;
		endwhile;
		?>
	</table>
</p>

<?php
doFooter();
?>