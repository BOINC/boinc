<?php
require_once("db.inc");
require_once("util.inc");
require_once("project_specific/project.inc");
require_once("gallery.inc");

db_init();
$user = get_logged_in_user();  // Can't delete your profile unless you're logged in!

// Handle a delete request.
if ($_POST['delete']) {
  delete_profile($user);
  exit();
}

page_head("Profile Delete Confirmation");

echo "<form action=", $_SERVER['PHP_SELF'], " method=\"POST\">";

start_table_noborder();
row1("Delete your profile");


rowify("
    <h2>Are you sure?</h2><p>
    Deleted profiles are gone forever and cannot be recovered --
    you will have to start from scratch
    if you want another profile in the future.
");
rowify(
    "<br>If you're sure, click the \"Delete\" button below
    to remove your profile from our database."
);
echo "<tr><td align=\"center\"><br><input type=\"submit\" name=\"delete\" value=\"Delete\"></td></tr>";
end_table();
echo "</form>";

page_tail();


function delete_profile($user) {

  $result = mysql_query("DELETE FROM profile WHERE userid = $user->id");
  if ($result) {
    
    delete_user_pictures($user->id);
    page_head("Delete Confirmation");
    echo "Your profile has been deleted<br>";
  } else {

    // TODO: Change this to a standard dialog.
    page_head("Deletion Error");
    echo "There was a problem deleting your profile.";
  }
  page_tail();
}

?>
