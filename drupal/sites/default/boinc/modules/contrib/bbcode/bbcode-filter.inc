<?php
// $Id: bbcode-filter.inc,v 1.66 2008/11/30 08:50:08 naudefj Exp $

function _bbcode_filter_process(&$body, $format = -1) {

  $quote_text = t('Quote:');
  $quote_user = t('\\1 wrote:');

  // Encode all script tags to prevent XSS html injection attacks
  $body = preg_replace(array('#<script([^>]*)>#i', '#</script([^>]*)>#i'), array('&lt;script\\1&gt;', '&lt;/script\\1&gt;'), $body);

  // Find all [code] tags and check if they contain a newline. If we find a newline, 
  // that [code] should be rendered as a block, otherwise it will still be inline
  $mode = variable_get("bbcode_paragraph_breaks_$format", 2);
  $pre = array(); $i = 0;
  if (preg_match_all('#\[code(?::\w+)?\](.*?)\[/code(?::\w+)?\]#si', $body, $code_tags, PREG_SET_ORDER)) {
    foreach ($code_tags as $code_tag) {
      $code_tag[1] = str_replace(array('<', '>'), array('&lt;', '&gt;'), $code_tag[1]);
      if (strpos($code_tag[1], "\n") === FALSE)
        $body = str_replace($code_tag[0], '<code class="bb-code">'. $code_tag[1] .'</code>', $body);
      elseif ($mode) {
        // Strip preformatted code blocks from text during line break processing, replaced below
        $body = str_replace($code_tag[0], "***pRe_sTrInG$i***", $body);
        $pre[$i++] = '<pre class="bb-code-block">'. $code_tag[1] .'</pre>';
      }
      else
        $body = str_replace($code_tag[0], '<pre class="bb-code-block">'. $code_tag[1] .'</pre>', $body);
    }
  }

  // Apply line and paragraph breaks (skipping preformatted code)
  if ($mode) {

    if ($mode == 1) {	// Line breaks only (starting with PHP 4.0.5, nl2br() is XHTML compliant)
      $body = nl2br($body);
    }

    if ($mode == 2) {	// Line and paragraph breaks (may not always be XHTML compliant)
      $body  = preg_replace("/(\r\n|\n|\r)/", "\n", $body);
      $body  = preg_replace("/\n\n+/", "\n\n", $body);
      $parts = explode("\n\n", $body);
      for ($i=0; $i<sizeof($parts); $i++) {
         // No linebreaks if paragraph starts with an HTML tag
         if ( !preg_match('/^<.*>/', $parts[$i]) ) {
           $parts[$i] = nl2br($parts[$i]);
         }

         // Some tags should not be in paragraph blocks
         if ( !preg_match('/^(?:0<|\[)(?:h1|h2|h3|h4|table|list|ol|ul|pre|select|form|blockquote|hr)/i', $parts[$i]) ) {
           $parts[$i] = '<p>'. $parts[$i] .'</p>';
         }
      }
      $body = implode("\n\n", $parts);
    }
  }

  // Reinsert preformatted code blocks
  foreach ($pre as $i => $code_tag) {
    $body = str_replace("***pRe_sTrInG$i***", $code_tag, $body);
  }
  
  // Implement [notag]
  $body = preg_replace_callback('#\[notag(?::\w+)?\](.*?)\[/notag(?::\w+)?\]#si',
    function ($matches) { return _bbcode_notag_tag($matches[1]); }, $body);

  // Replace any improper quote tags with proper quote tags
  $body = str_replace('[quote/]', '[/quote]', $body);
  
  // Add closing tags to prevent users from disruping your site's HTML
  // (required for nestable tags only: [list] and [quote])
  preg_match_all('/\[quote/i', $body, $matches);
  $opentags = count($matches['0']);
  preg_match_all('/\[\/quote\]/i', $body, $matches);
  $unclosed = $opentags - count($matches['0']);
  for ($i = 0; $i < $unclosed; $i++) {
    $body .= '[/quote]';
  }
  // Also add opening tags, if needed
  for ($i = $unclosed; $i < 0; $i++) {
    $body = '[quote]' . $body;
  }
  preg_match_all('/\[list/i', $body, $matches);
  $opentags = count($matches['0']);
  preg_match_all('/\[\/list\]/i', $body, $matches);
  $unclosed = $opentags - count($matches['0']);
  for ($i = 0; $i < $unclosed; $i++) {
    $body .= '[/list]';
  }
  for ($i = $unclosed; $i < 0; $i++) {
    $body = '[list]' . $body;
  }

  // begin processing for [size] 
  if (stristr($body, '[size=') !== FALSE) { // prevent useless processing
    $arr = array(
      'tag'         => 'size',
      'pattern' => '#\[\x07=([\w-]+)(?::\w+)?\]([^\x07]*)\[/\x07(?::\w+)?\]#si',
      'call' => function ($matches) { return '<span style="font-size:'. _bbcode_size_val($matches[1]) .'">'. str_replace('\"', '"', $matches[2]) .'</span>'; },
      'text'        => $body);
    $body = _bbcode_replace_nest_tag($arr);
  } // end processing for [size]

  // begin processing for [color]
  if (stristr($body, '[color=') !== FALSE) { // prevent useless processing
    $arr = array(
      'tag'         => 'color',
      'pattern'     => '#\[\x07=([\#\w]+)(?::\w+)?\]([^\x07]*)\[/\x07(?::\w+)?\]#si',
      'replacement' => '<span style="color:$1">$2</span>',
      'text'        => $body);
    $body = _bbcode_replace_nest_tag($arr);
  } // end processing for [color]

  // begin processing for [highlight]
  if (stristr($body, '[highlight=') !== FALSE) { // prevent useless processing
    $arr = array(
      'tag' => 'highlight',
      'pattern' => '#\[\x07=([\#\w]+)(?::\w+)?\]([^\x07]*)\[/\x07(?::\w+)?\]#si',
      'replacement' => '<span style="background-color:$1">$2</span>',
      'text' => $body,
    );
    $body = _bbcode_replace_nest_tag($arr);
  } // end processing for [highlight]

  // begin processing for [font]
  if (stristr($body, '[font=') !== FALSE) { // prevent useless processing
    $arr = array(
      'tag'         => 'font',
      'pattern'     => '#\[\x07=([\w\s]+)(?::\w+)?\]([^\x07]*)\[/\x07(?::\w+)?\]#si',
      'replacement' => '<span style="font-family:$1">$2</span>',
      'text'        => $body);
    $body = _bbcode_replace_nest_tag($arr);
  } // end processing for [font]

  // begin processing for [list] and [*]
  if (stristr($body, '[list') !== FALSE) { // prevent useless processing
    $l_type = array(
      NULL   => array('style' => 'circle', 'tag' => 'ul'),
      'c'    => array('style' => 'circle', 'tag' => 'ul'),
      'd'    => array('style' => 'disc', 'tag' => 'ul'),
      's'    => array('style' => 'square', 'tag' => 'ul'),
      '1'    => array('style' => 'decimal', 'tag' => 'ol'),
      'a'    => array('style' => 'lower-alpha', 'tag' => 'ol'),
      'A'    => array('style' => 'upper-alpha', 'tag' => 'ol'),
      'i'    => array('style' => 'lower-roman', 'tag' => 'ol'),
      'I'    => array('style' => 'upper-roman', 'tag' => 'ol')
      );
    $body = preg_replace('#(\[[/]*)list(.*?\])#si', "$1\x07$2", $body);

    // replace to <li> tags - [*]..[*]|[*]..[/list]
    $body = preg_replace('#\[\*(?::\w+)?\]([^\x07]*?)(?=\s*?(\[\*(?::\w+)?\]|\[/\x07(?::\w+)?\]))#si', '<li>$1</li>', $body);
    // add </li> tags to nested <li> - [/list]..[/list]
    $body = preg_replace('#(\[/\x07(?::\w+)?\])(?=[^\x07]*?\[/\x07(?::\w+)?\])#si', '$1</li>', $body);
    // add </li> tags to nested <li> - [/list]..[*]..[list]
    $body = preg_replace('#(\[/\x07(?::\w+)?\])(?=[^\x07]*?\[\*(?::\w+)?\][^\x07]*?\[\x07.*(?::\w+)?\])#si', '$1</li>', $body);
    // replace to <li> tags for nested <li> - [*]..[list]
    $body = preg_replace('#\[\*(?::\w+)?\]([^\x07]*)?(?=\[\x07.*(?::\w+)?\])#si', '<li>$1', $body);

    // replace to <ol>/<ul> and </ol>/</ul> tags
    // It will be better to use &count and do-while, if php 5 or higher.
    while (preg_match("#\[\x07[=]*((?-i)[cds1aAiI])*(?::\w+)?\]([^\x07]*)\[/\x07(?::\w+)?\]#si", $body)) {
      $body = preg_replace_callback("#\[\x07[=]*((?-i)[cds1aAiI])*(?::\w+)?\]([^\x07]*)\[/\x07(?::\w+)?\]#si",
	    function ($matches) use($l_type) { return '<'. $l_type[$matches[1]]["tag"] .' class="bb-list" style="list-style-type:'. $l_type[$matches[1]]["style"] .';">'. str_replace('\"', '"', $matches[2]) .'</'. $l_type[$matches[1]]["tag"] .'>'; },
		$body);
    }

    // remove <br /> tags
    $body = preg_replace('#(<[/]*([uo]l|li).*>.*)<br />#i', '$1', $body);
  } // end processing for [list] and [*]

  // PHP and HTML code blocks (syntax highlighted)
  $body = preg_replace_callback('#\[(php|html)(?::\w+)?\](?:[\r\n])*(.*?)\[/(php|html)(?::\w+)?\]#si',
    function ($matches) { return _bbcode_php_tag($matches[2]); }, $body);

  // Headings and indexes - articles will almost always need them
  $body = preg_replace_callback('#\[index style=(ol|ul)\]#si',
    function ($matches) { return _bbcode_generate_index($body, $matches[1]); }, $body);
  $body = preg_replace_callback('#\[index\s*/?\]#si',
    function ($matches) use ($body) { return _bbcode_generate_index($body); }, $body);
  $body = preg_replace_callback('#\[h([1-6])(?::\w+)?\](.*?)\[/h[1-6](?::\w+)?\]#si',
    function ($matches) { return _bbcode_generate_heading($matches[1], $matches[2]); }, $body);

  // Define BBCode tags
  $preg = array(
    // Font, text and alignment
	'#\[p\](.*?)\[/p\]#si' => '<p>\\1</p>',
    '#\[align=(\w+)(?::\w+)?\](.*?)\[/align(?::\w+)?\]#si'   => '<span style="text-align:\\1">\\2</span>',
    '#\[float=(left|right)(?::\w+)?\](.*?)\[/float(?::\w+)?\]#si' => '<span style="float:\\1">\\2</span>',
    '#\[justify(?::\w+)?\](.*?)\[/justify(?::\w+)?\]#si'     => '<div style="text-align:justify;">\\1</div>',
    '#\[indent(?::\w+)?\](.*?)\[/indent(?::\w+)?\]#si' => '<span style="margin-left:10px;">\\1</span>',
    '#\[tab\](\d*)\[/tab\]#si' => '<span style="margin-left:\\1px;"></span>',
    '#\[(b|strong)(?::\w+)?\](.*?)\[/(b|strong)(?::\w+)?\]#si' => '<span style="font-weight:bold">\\2</span>',
    '#\[(i|em)(?::\w+)?\](.*?)\[/(i|em)(?::\w+)?\]#si'       => '<span style="font-style:italic">\\2</span>',
    '#\[u(?::\w+)?\](.*?)\[/u(?::\w+)?\]#si'                 => '<span style="text-decoration:underline">\\1</span>',
    '#\[s(?::\w+)?\](.*?)\[/s(?::\w+)?\]#si'                 => '<s>\\1</s>',
    '#\[sup(?::\w+)?\](.*?)\[/sup(?::\w+)?\]#si'             => '<sup>\\1</sup>',
    '#\[sub(?::\w+)?\](.*?)\[/sub(?::\w+)?\]#si'             => '<sub>\\1</sub>',
    '#\[center(?::\w+)?\](.*?)\[/center(?::\w+)?\]#si'       => '<div style="text-align:center">\\1</div>',
    '#\[left(?::\w+)?\](.*?)\[/left(?::\w+)?\]#si'           => '<div style="text-align:left">\\1</div>',
    '#\[right(?::\w+)?\](.*?)\[/right(?::\w+)?\]#si'         => '<div style="text-align:right">\\1</div>',

    // Links without a protocol, with a protocol, and with good looking text
    '#\[url(?::\w+)?\]www\.([\w:;&,%+~!=@\/\.\-\#\?]+?)\[/url(?::\w+)?\]#si' => '<a href="http://www.\\1" class="bb-url">\\1</a>',
    '#\[url(?::\w+)?\]([\w:;&,%+~!=@\/\.\-\#\?]+?)\[/url(?::\w+)?\]#si'   => '<a href="\\1" class="bb-url">\\1</a>',
    '#\[url=www\.([\w:;&,%+~!=@\/\.\-\#\?]+?)\](.*?)\[/url(?::\w+)?\]#si' => '<a href="http://www.\\1" class="bb-url">\\2</a>',
    '#\[url="?\'?([\w:;&,%+~!=@\/\.\-\#\?]+?)"?\'?\](.*?)\[/url(?::\w+)?\]#si' => '<a href="\\1" class="bb-url">\\2</a>',

    // Anchor tags for linking within documents
    '#\[anchor=(\w+)(?::\w+)?\](.*?)\[/anchor(?::\w+)?\]#si' => '<a name="\\1">\\2</a>',
 
    // Images without or with client-side sizing
    '#\[img(?::\w+)?\]([\w:;&,~%+!=@\/\.\-\#\?]+)\[/img(?::\w+)?\]#si' => '<img src="\\1" alt="" class="bb-image" />',
    '#\[img=(\d+)x(\d+)(?::\w+)?\]([\w:;&,~%+!=@\/\.\-\#\?]+)\[/img(?::\w+)?\]#si' => '<img width="\\1" height="\\2" alt="" src="\\3" class="bb-image" />',
    '#\[img=([\w\s:;,\.\-\'\(\)]+)(?::\w+)?\]([\w:;&,~%+!=@\/\.\-\#\?]+)\[/img(?::\w+)?\]#si' => '<img alt="\\1" src="\\2" class="bb-image" />',
    '#\[img align=(left|right|center)(?::\w+)?\]([\w:;&,~%+!=@\/\.\-\#\?]+)\[/img(?::\w+)?\]#si' => '<img src="\\2" alt="" align="\\1" class="bb-image" />',

    // Flash animations and other special effects
    '#\[flash=(\d+)x(\d+)(?::\w+)?\]([\w:;&,~%+!=@\/\.\-\#\?]+)\[/flash(?::\w+)?\]#si' => '<object type="application/x-shockwave-flash" data="\\3" width="\\1" height="\\2"><param name="movie" value="\\3" /></object>', 

    // Acronyms & abbreviations - show description when mouse moves over tag
    '#\[acronym=([\w\s-,\.]+)(?::\w+)?\](.*?)\[/acronym(?::\w+)?\]#si' => '<acronym title="\\1">\\2</acronym>',
    '#\[abbr=([\w\s-,\.]+)(?::\w+)?\](.*?)\[/abbr(?::\w+)?\]#si'       => '<abbr title="\\1">\\2</abbr>',

    // Quoting with or without specifying the source
    '#\[quote(?::\w+)?\]#i'                                    => '<div class="bb-quote">'.$quote_text.'<blockquote class="bb-quote-body">',
    '#\[quote=(?:&quot;|"|\')?(.*?)["\']?(?:&quot;|"|\')?\]#i' => '<div class="bb-quote"><b>'.$quote_user.'</b><blockquote class="bb-quote-body">',
    '#\[/quote(?::\w+)?\]#si'                                  => '</blockquote></div>',

    // Links to popular sites
    '#\[google(?::\w+)?\]([\w\s-]+?)\[/google(?::\w+)?\]#si'       => '<a href="http://www.google.com/search?q=\\1">\\1</a>',
    '#\[wikipedia(?::\w+)?\]([\w\s-]+?)\[/wikipedia(?::\w+)?\]#si' => '<a href="http://www.wikipedia.org/wiki/\\1">\\1</a>',
    '#\[youtube\]([0-9a-zA-Z_\-]+)\[/youtube\]#si' => '<iframe width="425" height="366" src="//www.youtube.com/embed/\\1" frameborder="0" allowfullscreen></iframe>',
     // Table tags

    // Table tags
    '#\[table\](.+?)\[/table\]#si'                           => '<table class="bb-table">\\1</table>', 
    '#\[(row|r|tr)\](.+?)\[/(row|r|tr)\]#si'                 => '<tr>\\2</tr>', 
    '#\[(row|r|tr) color=([\#\w]+)\](.+?)\[/(row|r|tr)\]#si' => '<tr bgcolor=\\2>\\3</tr>', 
    '#\[(header|head|h)\](.+?)\[/(header|head|h)\]#si'       => '<th>\\2</th>',
    '#\[(col|c|td)\](.+?)\[/(col|c|td)\]#si'                 => '<td valign="top">\\2</td>',

    // Cleanup table output (td, th and tr tags)
    '#<([\/]?)t([dhr])><br />#si'                           => '<\\1t\\2>',
    '#<table(.+?)><br />#si'                                => '<table\\1>',
  );
  $body = preg_replace(array_keys($preg), array_values($preg), $body);

  // Simple replacements (str_replace is faster than preg_replace)
  $str = array(
    // Horizontal delimiter
    '[hr]'   => '<hr class="bb-hr" />',
    // Force line break
    '[br]'   => '<br class="bb-br" />',
    // Force space
    '[sp]'   => '&nbsp;',
  );
  $body = str_replace(array_keys($str), array_values($str), $body);

  // We cannot evaluate the variable in callback function because 
  // there is no way to pass the $format variable
  if (variable_get("bbcode_encode_mailto_$format", 1)) {
    // Replacing email addresses with encoded html
    $body = preg_replace_callback('#\[email(?::\w+)?\]([\w\.\-\+~@]+)\[/email(?::\w+)?\]#si', '_bbcode_encode_mailto', $body);
    $body = preg_replace_callback('#\[email=(.*?)(?::\w+)?\](.*?)\[/email(?::\w+)?\]#si', '_bbcode_encode_mailto', $body);
  }
  else {
    $body = preg_replace(
      array('#\[email(?::\w+)?\](.*?)\[/email(?::\w+)?\]#si','#\[email=(.*?)(?::\w+)?\]([\w\s]+)\[/email(?::\w+)?\]#si'),
      array('<a href="mailto:\\1" class="bb-email">\\1</a>', '<a href="mailto:\\1" class="bb-email">\\2</a>'),
      $body);
  }

  // Re-write email addresses
  if (variable_get("bbcode_rewrite_email_$format", 1)) {
    $body = preg_replace('#((?<=^|[\t\r\n >\(\[\]\|]))([a-z0-9\-_.]+?)@([\w\-]+\.([\w\-\.]+\.)*[\w]+)#i', '\\1\\2-[at]-\\3', $body);
  }

  // Turns web and e-mail addresses into clickable links
  if (variable_get("bbcode_make_links_$format", 1)) {

    // pad with a space so we can match things at the start of the 1st line
    $ret = ' ' . $body;
    // padding to already filtered links
    $ret = preg_replace('#(<a.+>)(.+</a>)#i', "$1\x07$2", $ret);

    // matches an "xxx://yyyy" URL at the start of a line, or after a space.
    // xxxx can only be alpha characters.
    // yyyy is anything up to the first space, newline, comma, double quote or <
    $ret = preg_replace('#(?<=^|[\t\r\n >\(\[\]\|])([a-z]+?://[\w\-]+\.([\w\-]+\.)*\w+(:[0-9]+)?(/[^ "\'\(\n\r\t<\)\[\]\|]*)?)((?<![,\.])|(?!\s))#i', '<a href="\1">\1</a>', $ret);

    // matches a "www|ftp.xxxx.yyyy[/zzzz]" kinda lazy URL thing
    // Must contain at least 2 dots. xxxx contains either alphanum, or "-"
    // zzzz is optional.. will contain everything up to the first space, newline,
    // comma, double quote or <.
     $ret = preg_replace('#([\t\r\n >\(\[\|])(www|ftp)\.(([\w\-]+\.)*[\w]+(:[0-9]+)?(/[^ \"\'\(\n\r\t<\)\[\]\|]*)?)#i', '\1<a href="http://\2.\3">\2.\3</a>', $ret);

    // matches an email@domain type address at the start of a line, or after a space.
    // Note: Only the followed chars are valid; alphanums, "-", "_" and or ".".
    if (variable_get("bbcode_encode_mailto_$format", 1)) {
      $ret = preg_replace_callback("#((?<=^|[\t\r\n >\(\[\]\|]))([a-z0-9\-_.]+?)@([\w\-]+\.([\w\-\.]+\.)*[\w]+)#i", '_bbcode_encode_mailto', $ret);
    } else {
      $ret = preg_replace('#((?<=^|[\t\r\n >\(\[\]\|]))([a-z0-9\-_.]+?)@([\w\-]+\.([\w\-\.]+\.)*[\w]+)#i', '\\1<a href="mailto:\\2@\\3">\\2@\\3</a>', $ret);
    }

    // Remove our padding
    $ret = str_replace("\x07", '', $ret);
    $body = substr($ret, 1);
  }

  if (variable_get("bbcode_filter_nofollow_$format", 0)) {
    $body = preg_replace('#<a([^>]+)>#i', '<a\\1 rel="nofollow">', $body);
  }

  return $body;
}

function _bbcode_generate_heading($level, $text) {
  $anchor = preg_replace('/([\s]+)/', '_', $text);
  $anchor = preg_replace('/([\W]+)/', '',  $anchor);
  return '<h'. $level .'><a name="'. $anchor .'">'. $text .'</a></h'. $level .'>';
}

function _bbcode_generate_index($body, $tag = 'ol') {
  $level = 0;
  $index = '<'. $tag .">\n";
  $close_tags = 0;

  if (preg_match_all('#\[h([1-6]).*?\](.*?)\[/h([1-6]).*?\]#si', $body, $head_tags, PREG_SET_ORDER)) {
    foreach ($head_tags as $head_tag) {
      if ($level == 0) $level = $head_tag[1];
      $anchor = preg_replace('/([\s]+)/', '_', $head_tag[2]);
      $anchor = preg_replace('/([\W]+)/', '',  $anchor);

      if ($head_tag[1] > $level) {
        $index .= '<'. $tag .">\n";
        $index .= '<li><a href="#'. $anchor .'">'. $head_tag[2] ."</a>\n";
        $close_tags++;
        $level = $head_tag[1];
      } else if ($head_tag[1] < $level) {
        while ($close_tags > 0) {
           $index .= '</'. $tag .">\n";
           $close_tags--;
        }
        $index .= '<li><a href="#'. $anchor .'">'. $head_tag[2] ."</a>\n";
        $level = $head_tag[1];
      } else {
        $index .= '<li><a href="#'. $anchor .'">'. $head_tag[2] ."</a>\n";
        $level = $head_tag[1];
      }
    }
  }
  while ($close_tags >= 0) {
    $index .= '</'. $tag .">\n";
    $close_tags--;
  }
  return $index;
}

function _bbcode_encode_mailto($matches) {
  if (isset($matches[3])) 
    $link = 'document.write(\'<a href="mailto:' . $matches[2].'@'.$matches[3] . '">' . $matches[2].'@'.$matches[3] . '</a>\');';
  else 
    $link = 'document.write(\'<a href="mailto:' . $matches[1] . '" class="bb-email">' . (isset($matches[2]) ? $matches[2] : $matches[1]) . '</a>\');';

  $js_encode = '';
  for ($x = 0; $x < strlen($link); $x++) 
    $js_encode .= '%' . bin2hex($link{$x});

  $link = '<script type="text/javascript">eval(unescape(\''.$js_encode.'\'))</script>';
  if (isset($matches[3]))
    $link = $matches[1] . $link;

  return $link;
}

function _bbcode_notag_tag($text = NULL) {
  return str_replace( array('[', ']', '@'), array('&#91;', '&#93;', '&#64;'), stripslashes($text));
}

function _bbcode_php_tag($text = NULL) {
  return '<pre>'. highlight_string( str_replace('<br />', '', stripslashes($text)), true) .'</pre>';
}

function _bbcode_size_val($size) {
  if (!is_numeric($size))	  // Return values like 10px or xx-small.
    return $size;

  if ($size < 6)	// Normalize too small size values.
    return '6px';
  elseif ($size <= 48)	// Less than 49 is px, more is % values.
    return $size . 'px';
  else
    return $size . '%';
}

function _bbcode_replace_nest_tag($arr = NULL) {
  $text = preg_replace('#(\[[/]*)' . $arr['tag'] . '(.*?\])#si', "$1\x07$2", $arr['text']);
  // It will be better to use &count and do-while, if php 5 or higher.

  if (array_key_exists('call', $arr)) {
    while (preg_match($arr['pattern'], $text)) {
      $text = preg_replace_callback($arr['pattern'], $arr['call'], $text);
    }
  } else {
    while (preg_match($arr['pattern'], $text)) {
      $text = preg_replace($arr['pattern'], $arr['replacement'], $text);
    }
  }
  return $text;
}

