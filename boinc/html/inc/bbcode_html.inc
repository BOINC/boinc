<?php

require_once("../inc/translation.inc");

$bbcode_js = "<script type=\"text/javascript\" src=\"bbcode_toolbar.js\"></script>";

$bbcode_html = '
<table width="40%" border="0" cellspacing="0" cellpadding="2" class="bbcode_buttons">
          <tr align="center" valign="middle">
            <td><span class="genmed">
              <input type="button" class="button" accesskey="b" name="addbbcode0" value=" B " style="font-weight:bold; width: 30px" onClick="bbstyle(0)" title="'.tra("Bold text: [b]text[/b]  (alt+b)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="i" name="addbbcode2" value=" i " style="font-style:italic; width: 30px" onClick="bbstyle(2)" title="'.tra("Italic text: [i]text[/i]  (alt+i)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="u" name="addbbcode4" value=" u " style="text-decoration: underline; width: 30px" onClick="bbstyle(4)" title="'.tra("Underline text: [u]text[/u]  (alt+u)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="q" name="addbbcode6" value="Quote" style="width: 50px" onClick="bbstyle(6)" title="'.tra("Quote text: [quote]text[/quote]  (alt+q)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="c" name="addbbcode8" value="Code" style="width: 40px" onClick="bbstyle(8)" title="'.tra("Code display: [code]code[/code]  (alt+c)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="l" name="addbbcode10" value="List" style="width: 40px" onClick="bbstyle(10)" title="'.tra("List: [list]text[/list] (alt+l)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="o" name="addbbcode12" value="List=" style="width: 40px" onClick="bbstyle(12)" title="'.tra("Ordered list: [list=]text[/list]  (alt+o)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="p" name="addbbcode14" value="Img" style="width: 40px"  onClick="bbstyle(14)" title="'.tra("Insert image: [img]http://image_url[/img]  (alt+p)").'" />
              </span></td>
            <td><span class="genmed">
              <input type="button" class="button" accesskey="w" name="addbbcode16" value="URL" style="text-decoration: underline; width: 40px" onClick="bbstyle(16)" title="'.tra("Insert URL: [url]http://url[/url] or [url=http://url]URL text[/url]  (alt+w)").'" />
              </span></td>
          </tr>
          <tr>
            <td colspan="9">
              <table width="100%" border="0" cellspacing="0" cellpadding="0">
                <tr>
                  <td><nobr><span class="genmed">&nbsp;'.tra("Font color").':
                    <select name="addbbcode22" onChange="bbfontstyle(\'[color=\' + this.form.addbbcode22.options[this.form.addbbcode22.selectedIndex].value + \']\', \'[/color]\');this.selectedIndex=0;" title="'.tra("Font color: [color=red]text[/color]  Tip: you can also use color=#FF0000").'" >
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
                    </select>&nbsp;'.tra("Font size").':
		    <select name="addbbcode24" onChange="bbfontstyle(\'[size=\' + this.form.addbbcode24.options[this.form.addbbcode24.selectedIndex].value + \']\', \'[/size]\');this.selectedIndex=0;" title="'.tra("Font size: [size=x-small]small text[/size]").'" >
                      <option value="12" selected class="genmed">'.tra("Default").'</option>
            	      <option value="9" class="genmed">'.tra("Small").'</option>
                      <option value="12" class="genmed">'.tra("Normal").'</option>
                      <option value="18" class="genmed">'.tra("Large").'</option>
                    </select>
                    </span></nobr></td>
                  <td nowrap="nowrap" align="right"><span class="gensmall"><a href="javascript:bbstyle(-1)" class="genmed" title='.tra("Close all open bbCode tags").'>'.tra("Close Tags").'</a></span></td>
                </tr>
              </table>
            </td>
          </tr>
        </table>
';