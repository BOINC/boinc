<?php

require_once("../inc/util.inc");

$bbcode_js = "<script type=\"text/javascript\" src=\"bbcode_toolbar.js\"></script>";

$bbcode_html = '
    <p>
    <input type="button" class="btn btn-default btn-sm" accesskey="b" name="addbbcode0" value=" B " style="font-weight:bold; " onClick="bbstyle(0)" title="'.tra("Bold text: [b]text[/b]  (alt+b)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="i" name="addbbcode2" value=" i " style="font-style:italic; " onClick="bbstyle(2)" title="'.tra("Italic text: [i]text[/i]  (alt+i)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="u" name="addbbcode4" value=" u " style="text-decoration: underline; " onClick="bbstyle(4)" title="'.tra("Underline text: [u]text[/u]  (alt+u)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="k" name="addbbcode20" value=" k " style="text-decoration: line-through; " onClick="bbstyle(20)" title="'.tra("Strikethrough text: [s]text[/s]  (alt+k)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="q" name="addbbcode6" value="Quote" onClick="bbstyle(6)" title="'.tra("Quote text: [quote]text[/quote]  (alt+q)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="c" name="addbbcode8" value="Code" style="" onClick="bbstyle(8)" title="'.tra("Code display: [code]code[/code]  (alt+c)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="l" name="addbbcode10" value="List" style="" onClick="bbstyle(10)" title="'.tra("List: [list]text[/list] (alt+l)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="o" name="addbbcode12" value="List=1" style="" onClick="bbstyle(12)" title="'.tra("Ordered list: [list=1]text[/list]  (alt+o)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="p" name="addbbcode14" value="Img" style=""  onClick="bbstyle(14)" title="'.tra("Insert image: [img]http://image_url[/img]  (alt+p)").'" />
    <input type="button" class="btn btn-default btn-sm" accesskey="w" name="addbbcode16" value="URL" style="text-decoration: underline; " onClick="bbstyle(16)" title="'.tra("Insert URL: [url]http://url[/url] or [url=http://url]URL text[/url]  (alt+w)").'" />
    <p>
    &nbsp;'.tra("Font color").':
    <select style="color:#000;" name="addbbcode22" onChange="bbfontstyle(\'[color=\' + this.form.addbbcode22.options[this.form.addbbcode22.selectedIndex].value + \']\', \'[/color]\');this.selectedIndex=0;" title="'.tra("Font color: [color=red]text[/color]  Tip: you can also use color=#FF0000").'" >
    <option style="color:black" value="#000000" class="genmed">'.tra("Default").'</option>
    <option style="color:darkred" value="darkred" class="genmed">'.tra("Dark Red").'</option>
    <option style="color:red" value="red" class="genmed">'.tra("Red").'</option>
    <option style="color:orange" value="orange" class="genmed">'.tra("Orange").'</option>
    <option style="color:brown" value="brown" class="genmed">'.tra("Brown").'</option>
    <option style="color:yellow" value="yellow" class="genmed">'.tra("Yellow").'</option>
    <option style="color:green" value="green" class="genmed">'.tra("Green").'</option>
    <option style="color:olive" value="olive" class="genmed">'.tra("Olive").'</option>
    <option style="color:cyan" value="cyan" class="genmed">'.tra("Cyan").'</option>
    <option style="color:blue" value="blue" class="genmed">'.tra("Blue").'</option>
    <option style="color:darkblue" value="darkblue" class="genmed">'.tra("Dark Blue").'</option>
    <option style="color:indigo" value="indigo" class="genmed">'.tra("Indigo").'</option>
    <option style="color:violet" value="violet" class="genmed">'.tra("Violet").'</option>
    </select>
    &nbsp;'.tra("Font size").':
    <select style="color:#000;" name="addbbcode24" onChange="bbfontstyle(\'[size=\' + this.form.addbbcode24.options[this.form.addbbcode24.selectedIndex].value + \']\', \'[/size]\');this.selectedIndex=0;" title="'.tra("Font size: [size=x-small]small text[/size]").'" >
    <option value="12" selected class="genmed">'.tra("Default").'</option>
    <option value="9" class="genmed">'.tra("Small").'</option>
    <option value="12" class="genmed">'.tra("Normal").'</option>
    <option value="18" class="genmed">'.tra("Large").'</option>
    </select>
    &nbsp;
    <a href="javascript:bbstyle(-1)" title='.tra("Close all open bbCode tags").'>'.tra("Close Tags").'</a>
    <p>
';

?>
