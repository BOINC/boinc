<?php
    include_once("htmlfilter.inc");

// $tag_list = Array( false, 'blink', 'object', 'meta', 'font', 'html', 'link', 'frame', 'iframe', 'layer', 'ilayer');

$tag_list = Array(true, "b", "a", "i", "img", "strong", "em", "p");

$rm_tags_with_content = Array(
                              'script',
                              'style',
                              'applet',
                              'embed',
                              'head',
                              'frameset'
                              );

$self_closing_tags =  Array(
                            'img',
                            'br',
                            'hr',
                            'input'
                            );

$force_tag_closing = false;

$rm_attnames = Array(
    '/.*/' =>
        Array(
              '/target/i',
              '/^on.*/i',
              '/^dynsrc/i',
              '/^datasrc/i',
              '/^data.*/i'
              )
    );

/**
 * Yeah-yeah, so this looks horrible. Check out htmlfilter.inc for
 * some idea of what's going on here. :)
 */

$bad_attvals = Array(
    '/.*/' =>
        Array(
              '/.*/' =>
                  Array(
                        Array(
                          '/^([\'\"])\s*\S+\s*script\s*:*(.*)([\'\"])/i',
                          '/^([\'\"])\s*https*\s*:(.*)([\'\"])/i',
                          '/^([\'\"])\s*mocha\s*:*(.*)([\'\"])/i',
                          '/^([\'\"])\s*about\s*:(.*)([\'\"])/i'
                             ),
                        Array(
                      '\\1oddjob:\\2\\3',
                      '\\1uucp:\\2\\3',
                      '\\1amaretto:\\2\\3',
                      '\\1round:\\2\\3'
                             )
                        ),     
                                                
              '/^style/i' =>
                  Array(
                        Array(
                              '/expression/i',
                              '/behaviou*r/i',
                              '/binding/i',
                              '/url\(([\'\"]*)\s*https*:.*([\'\"]*)\)/i',
                              '/url\(([\'\"]*)\s*\S+script:.*([\'\"]*)\)/i'
                             ),
                        Array(
                              'idiocy',
                              'idiocy',
                              'idiocy',
                              'url(\\1http://securityfocus.com/\\2)',
                              'url(\\1http://securityfocus.com/\\2)'
                             )
                        )
              )
    );

$add_attr_to_tag = Array(
                         '/^a$/i' => Array('target' => '"_new"')
                         );
                         
function sanitize_html($body) {
    global $tag_list;
    global $rm_tags_with_content;
    global $self_closing_tags;
    global $force_tag_closing;
    global $rm_attnames;
    global $bad_attvals;
    global $add_attr_to_tag;
    return sanitize(
        $body, 
        $tag_list, 
        $rm_tags_with_content,
        $self_closing_tags,
        $force_tag_closing,
        $rm_attnames,
        $bad_attvals,
        $add_attr_to_tag
    );
}

?>
