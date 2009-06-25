<?php

$bbcode_js = "<script type=\"text/javascript\" src=\"bbcode_toolbar.js\"></script>";

$bbcode_html = <<<EOT
<table width="40%" border="0" cellspacing="0" cellpadding="2">
		  <tr align="center" valign="middle">
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="b" name="addbbcode0" value=" B " style="font-weight:bold; width: 30px" onClick="bbstyle(0)" title="Bold text: [b]text[/b]  (alt+b)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="i" name="addbbcode2" value=" i " style="font-style:italic; width: 30px" onClick="bbstyle(2)" title="Italic text: [i]text[/i]  (alt+i)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="u" name="addbbcode4" value=" u " style="text-decoration: underline; width: 30px" onClick="bbstyle(4)" title="Underline text: [u]text[/u]  (alt+u)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="q" name="addbbcode6" value="Quote" style="width: 50px" onClick="bbstyle(6)" title="Quote text: [quote]text[/quote]  (alt+q)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="c" name="addbbcode8" value="Code" style="width: 40px" onClick="bbstyle(8)" title="Code display: [code]code[/code]  (alt+c)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="l" name="addbbcode10" value="List" style="width: 40px" onClick="bbstyle(10)" title="List: [list]text[/list] (alt+l)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="o" name="addbbcode12" value="List=" style="width: 40px" onClick="bbstyle(12)" title="Ordered list: [list=]text[/list]  (alt+o)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="p" name="addbbcode14" value="Img" style="width: 40px"  onClick="bbstyle(14)" title="Insert image: [img]http://image_url[/img]  (alt+p)" />
			  </span></td>
			<td><span class="genmed">
			  <input type="button" class="button" accesskey="w" name="addbbcode16" value="URL" style="text-decoration: underline; width: 40px" onClick="bbstyle(16)" title="Insert URL: [url]http://url[/url] or [url=http://url]URL text[/url]  (alt+w)" />
			  </span></td>
		  </tr>
		  <tr>
			<td colspan="9">
			  <table width="100%" border="0" cellspacing="0" cellpadding="0">
				<tr>
				  <td><nobr><span class="genmed"> &nbsp;Font color:
					<select name="addbbcode22" onChange="bbfontstyle('[color=' + this.form.addbbcode22.options[this.form.addbbcode22.selectedIndex].value + ']', '[/color]');this.selectedIndex=0;" title="Font color: [color=red]text[/color]  Tip: you can also use color=#FF0000" >
					  <option style="color:black" value="#000000" class="genmed">Default</option>
					  <option style="color:darkred" value="darkred" class="genmed">Dark Red</option>
					  <option style="color:red" value="red" class="genmed">Red</option>
					  <option style="color:orange" value="orange" class="genmed">Orange</option>
					  <option style="color:brown" value="brown" class="genmed">Brown</option>
					  <option style="color:yellow" value="yellow" class="genmed">Yellow</option>
					  <option style="color:green" value="green" class="genmed">Green</option>
					  <option style="color:olive" value="olive" class="genmed">Olive</option>
					  <option style="color:cyan" value="cyan" class="genmed">Cyan</option>
					  <option style="color:blue" value="blue" class="genmed">Blue</option>
					  <option style="color:darkblue" value="darkblue" class="genmed">Dark Blue</option>
					  <option style="color:indigo" value="indigo" class="genmed">Indigo</option>
					  <option style="color:violet" value="violet" class="genmed">Violet</option>
					</select> &nbsp;Font size:<select name="addbbcode24" onChange="bbfontstyle('[size=' + this.form.addbbcode24.options[this.form.addbbcode24.selectedIndex].value + ']', '[/size]');this.selectedIndex=0;" title="Font size: [size=x-small]small text[/size]" >
					  <option value="12" selected class="genmed">Default</option>
            <option value="9" class="genmed">Small</option>
					  <option value="12" class="genmed">Normal</option>
					  <option value="18" class="genmed">Large</option>
					</select>
					</span></nobr></td>
				  <td nowrap="nowrap" align="right"><span class="gensmall"><a href="javascript:bbstyle(-1)" class="genmed" title="Close all open bbCode tags">Close Tags</a></span></td>
				</tr>
			  </table>
			</td>
		  </tr>
		</table>
EOT;
