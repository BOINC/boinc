<?php

function list_item2($x, $y) {
    global $light_blue;
    echo "
        <tr>
            <td width=50% bgcolor=$light_blue valign=top >$x</td>
            <td colspan=8>$y</td>
        </tr>
    ";
}

function mult_choice($name, $text, $choices) {
    $x = "";
    foreach ($choices as $cname=>$ctext) {
        $x .= "<input type=radio name=$name value=\"$cname\"> $ctext<br>\n";
    }
    list_item2($text, $x);
}

function boxes($boxes) {
    $x = "<font size=-2>[check all that apply]</font><br>\n";
    foreach ($boxes as $name => $text) {
        $x .= "<input type=checkbox disabled name=$name> $text<br>\n";
    }
    return $x;
}

function radio($radio_name, $boxes) {
    $x = "";
    foreach ($boxes as $name => $text) {
        $x .= "<input type=radio disabled name=$radio_name value=$name> $text<br>\n";
    }
    return $x;
}

function show_overall_choices($choices) {
    foreach ($choices as $choice) {
        $rname = $choice["rname"];
        $text = $choice["text"];
        $radio_name = $choice["radio_name"];
        $x = "<input onclick=\"$rname()\" type=radio name=overall_choice value=$rname> $text";
        if ($radio_name) {
            $y = radio($radio_name, $choice["boxes"]);
        } else {
            $y = boxes($choice["boxes"]);
        }
        list_item2($x, $y);
    }
}

function generate_functions($choices) {
    echo "<script>\n";
    echo "function disable_all() {\n";
    foreach ($choices as $choice) {
        $radio_name = $choice["radio_name"];
        if ($radio_name) {
            $boxes = $choice["boxes"];
            $i = 0;
            foreach ($boxes as $name => $text) {
                echo "document.forms.blah.".$radio_name."[".$i."].disabled=true;\n";
                $i++;
            }
        } else {
            $boxes = $choice["boxes"];
            foreach ($boxes as $name => $text) {
                echo "document.forms.blah.$name.disabled=true;\n";
            }
        }
    }
    echo "}\n";
    foreach ($choices as $choice) {
        $rname = $choice["rname"];
        echo "function $rname() {\n";
        echo "disable_all();\n";
        $radio_name = $choice["radio_name"];
        if ($radio_name) {
            $boxes = $choice["boxes"];
            $i = 0;
            foreach ($boxes as $name => $text) {
                echo "document.forms.blah.".$radio_name."[".$i."].disabled=false;\n";
                $i++;
            }
        } else {
            $boxes = $choice["boxes"];
            foreach ($boxes as $name => $text) {
                echo "document.forms.blah.$name.disabled=false;\n";
            }
        }
        echo "}\n";
    }
    echo "</script>\n";
}

?>
