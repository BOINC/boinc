<?php {

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    db_init();
    $user = get_logged_in_user();

    $query = sprintf(
        "select * from team where id = %d",
        $_POST["id"]
    );
    $result = mysql_query($query);
    if ($result) {
      $team = mysql_fetch_object($result);
      mysql_free_result($result);
    }
    require_founder_login($user, $team);

    page_head("Removing users from $team->name", $user);
    $nmembers = 0;
    for ($i=0; $i<$_POST["ninactive_users"]; $i++) {
        if ($_POST["remove_$i"] != 0) {
            $query = sprintf(
                "select * from user where id = %d",
                $_POST["remove_$i"]
            );
            $result = mysql_query($query);
            $user = mysql_fetch_object($result);
            if ($user->teamid != $team->id) {
                echo "<br>$user->name is not a member of $team->name";
            } else {
                $query_user_table = sprintf(
                    "update user set teamid = 0 where id = %d",
                   $_POST["remove_$i"]
                );
                $nmembers++;
                $result_user_table = mysql_query($query_user_table);
                echo "<br>$user->name has been removed";
             }
         }
    }
    $new_nusers = $team->nusers - $nmembers;
    if ($new_nusers > 0) {
        $query = "update team set nusers = $new_nusers where id = $team->id";
    } else {
        $query = "remove from team where id=$team->id";
        echo "<p><b>The team has been disbanded because there are no more members.</b>";
    }
    $result = mysql_query($query);

    page_tail();

} ?>
