<?php

$light_blue = "#d5f0ff";

// independent of any particular poll

function list_item2($x, $y) {
    global $light_blue;
    echo "
        <tr>
            <td width=50% bgcolor=$light_blue valign=top >$x</td>
            <td colspan=8>$y</td>
        </tr>
    ";
}

function boxes($options) {
    $x = "<font size=-2>".tra("[check all that apply]")."</font><br>\n";
    foreach ($options as $name => $text) {
        $x .= "<input type=checkbox name=$name> $text<br>\n";
    }
    return $x;
}

function radio($radio_name, $options) {
    $x = "";
    foreach ($options as $name => $text) {
        $x .= "<input type=radio name=$radio_name value=$name> $text<br>\n";
    }
    return $x;
}

function show_choice($choice, $outer_radio) {
    $rname = $choice["rname"];
    $text = $choice["text"];
    $radio_name = $choice["radio_name"];
    $x = "";
    if ($rname) {
        $x .= "<input onclick=\"$rname()\" type=radio name=$outer_radio value=$rname >";
    }
    $x .= $text;
    if ($radio_name) {
        $y = radio($radio_name, $choice["options"]);
    } else {
        $y = boxes($choice["options"]);
        $other_name = $choice['other_name'];
        $y .= tra("Other:")." <input size=30 name=$other_name>";
    }
    list_item2($x, $y);
}

function show_choices($choices, $outer_radio) {
    foreach ($choices as $choice) {
        show_choice($choice, $outer_radio);
    }
}

function generate_functions($choices) {
    echo "<script>\n";
    echo "function disable_all() {\n";
    foreach ($choices as $choice) {
        $radio_name = $choice["radio_name"];
        if ($radio_name) {
            $options = $choice["options"];
            $i = 0;
            foreach ($options as $name => $text) {
                echo "document.forms.blah.".$radio_name."[".$i."].disabled=true;\n";
                $i++;
            }
        } else {
            $options = $choice["options"];
            foreach ($options as $name => $text) {
                echo "document.forms.blah.$name.disabled=true;\n";
            }
            $other_name = $choice['other_name'];
            echo "document.forms.blah.$other_name.disabled=true;\n";
        }
    }
    echo "}\n";
    foreach ($choices as $choice) {
        $rname = $choice["rname"];
        echo "function $rname() {\n";
        echo "disable_all();\n";
        $radio_name = $choice["radio_name"];
        if ($radio_name) {
            $options = $choice["options"];
            $i = 0;
            foreach ($options as $name => $text) {
                echo "document.forms.blah.".$radio_name."[".$i."].disabled=false;\n";
                $i++;
            }
        } else {
            $options = $choice["options"];
            foreach ($options as $name => $text) {
                echo "document.forms.blah.$name.disabled=false;\n";
            }
            $other_name = $choice['other_name'];
            echo "document.forms.blah.$other_name.disabled=false;\n";
        }
        echo "}\n";
    }
    echo "</script>\n";
}

function new_response($uid, $xml) {
    $now = time();
    return mysql_query("insert into response (uid, create_time, update_time, xml) values ('$uid', $now, $now, '$xml')");
}

function update_response($uid, $xml) {
    $now = time();
    return mysql_query("update response set update_time=$now, xml='$xml' where uid='$uid'");
}

function select_response($uid) {
    $result = mysql_query("select * from response where uid='$uid'");
    $response = mysql_fetch_object($result);
    mysql_free_result($result);
    return $response;
}

function parse_form_choice($x, $choice) {
    $radio_name= $choice["radio_name"];
    if ($radio_name) {
        $x[$radio_name] = get_str($radio_name);
    } else {
        $options = $choice["options"];
        foreach ($options as $name=>$text) {
            $x[$name] = get_str($name);
        }
        $other_name = $choice['other_name'];
        $x[$other_name] = get_str($other_name);
    }
    return $x;
}

function parse_form_choices($x, $choices) {
    foreach ($choices as $choice) {
        $x = parse_form_choice($x, $choice);
    }
    return $x;
}

function gen_xml_choice($x, $choice) {
    $xml = "";
    $radio_name= $choice["radio_name"];
    if ($radio_name) {
        $val = urlencode($x[$radio_name]);
        $xml .= "<$radio_name>$val</$radio_name>\n";
    } else {
        $options = $choice["options"];
        foreach ($options as $name=>$text) {
            $val = urlencode($x[$name]);
            $xml .= "<$name>$val</$name>\n";
        }
        $other_name = $choice['other_name'];
        $text = urlencode($x[$other_name]);
        $xml .= "<$other_name>$text</$other_name>\n";
    }
    return $xml;
}

function gen_xml_choices($x, $choices) {
    foreach ($choices as $choice) {
        $xml .= gen_xml_choice($x, $choice);
    }
    return $xml;
}

?>
