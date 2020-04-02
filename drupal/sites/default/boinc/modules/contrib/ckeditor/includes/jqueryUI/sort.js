/*
Copyright (c) 2003-2013, CKSource - Frederico Knabben. All rights reserved.
For licensing, see LICENSE.html or http://ckeditor.com/license
*/
jQuery(document).ready(function() {
    function Tools(event, ui) {
        //outer loop for rows
        var tools = "[\n";
        rows = jQuery("#groupLayout div.sortableListDiv").length;
        jQuery.each(jQuery("#groupLayout div.sortableListDiv"), function(rowIndex, rowValue) {
            if (jQuery("li",rowValue).length > 0) {
                tools = tools + "    [";
            }
            //inner loop for toolbar buttons
            jQuery.each(jQuery("li",rowValue), function(buttonIndex, buttonValue) {
                if (jQuery(buttonValue).hasClass('spacer')) {
                    tools = tools + ",'-'";
                }
                else if (jQuery(buttonValue).hasClass('group')) {
                    tools = tools + "],\n    [";
                }
                else {
                    tools = tools + ",'" + jQuery(buttonValue).attr('id') + "'" ;
                }
            });

            if (jQuery("li" ,rowValue).length > 0) {
                if (rowIndex < (rows -1)) {
                    tools = tools + "],\n    '/',\n";
                }
                else {
                    tools = tools + "]\n";
                }
            }
        });
        tools = tools + "]";
        tools = tools.replace(/\[,/g, '[');
        tools = tools.replace(/\[],/g, '');
        jQuery("#edit-toolbar").attr('value', tools);
    }

    Drupal.ckeditorToolbaInit = function() {
        Drupal.ckeditorToolbarUsedRender();
        Drupal.ckeditorToolbarAllRender();

        var firefox = navigator.userAgent.toLowerCase().match(/firefox\/[0-9]\./);
        jQuery(".sortableList").sortable({
            connectWith: ".sortableList",
            items: "div.sortableListDiv",
            sort: function(event, ui) {
                if (firefox){
                    ui.helper.css({'top' : ui.position.top - 35 + 'px'});
                }
            },
            stop: function(event, ui) {
                Tools(event, ui);
            }
        }).disableSelection();

        jQuery(".sortableRow").sortable({
            connectWith: ".sortableRow",
            items: "li.sortableItem",
            sort: function(event, ui) {
                if (firefox){
                    ui.helper.css({'top' : ui.position.top - 35 + 'px'});
                }
            },
            stop: function(event, ui) {
                Tools(event, ui);
            }
        }).disableSelection();

        jQuery("li.sortableItem").mouseover(function(){
            jQuery(".sortableList").sortable("disable");
        });
        jQuery("li.sortableItem").mouseout(function(){
            jQuery(".sortableList").sortable("enable");
        });
    };

    Drupal.ckeditorToolbarReload = function() {
        jQuery(".sortableList").sortable('destroy');
        jQuery(".sortableRow").sortable('destroy');
        jQuery("li.sortableItem").unbind();
        Drupal.ckeditorToolbaInit();
    };

    Drupal.ckeditorToolbarUsedRender = function() {
        var toolbar = jQuery('#edit-toolbar').val();
        toolbar = Drupal.ckeditorToolbarToArray(toolbar);
        var html = '<div class="sortableListDiv"><span class="sortableListSpan"><ul class="sortableRow">';
        var group = false;

        for (var row in toolbar) {
            if (typeof toolbar[row] == 'string' && toolbar[row] == '/') {
                group = false;
                html += '</ul></span></div><div class="sortableListDiv"><span class="sortableListSpan"><ul class="sortableRow">';
            }
            else {
                if (group == false){
                    group = true;
                }
                else {
                    html += '<li class="sortableItem group"><img src="' + Drupal.settings.cke_toolbar_buttons_all['__group']['icon'] + '" alt="group" title="group" /></li>';
                }
                for (var button in toolbar[row]) {
                    if (toolbar[row][button] == '-') {
                        html += '<li class="sortableItem spacer"><img src="' + Drupal.settings.cke_toolbar_buttons_all['__spacer']['icon'] + '" alt="spacer" title="spacer" /></li>';
                    }
                    else if (Drupal.settings.cke_toolbar_buttons_all[toolbar[row][button]]) {
                        html += '<li class="sortableItem" id="' + Drupal.settings.cke_toolbar_buttons_all[toolbar[row][button]]['name'] + '"><img src="' + Drupal.settings.cke_toolbar_buttons_all[toolbar[row][button]]['icon'] + '" alt="' + Drupal.settings.cke_toolbar_buttons_all[toolbar[row][button]]['title'] + '" title="' + Drupal.settings.cke_toolbar_buttons_all[toolbar[row][button]]['title'] + '" /></li>';
                    }
                }
            }
        }
        html += '</ul></span></div>';
        jQuery('#groupLayout').empty().append(html);
    };

    Drupal.ckeditorToolbarAllRender = function() {
        var toolbarUsed = jQuery('#edit-toolbar').val();
        var toolbarAll = Drupal.settings.cke_toolbar_buttons_all;

        var htmlArray = [];
        var html = '';

        for (var i in toolbarAll) {
            if (new RegExp("\'[\s]*" + toolbarAll[i].name + "[\s]*\'").test(toolbarUsed) == false) {
                if (toolbarAll[i].name == false) continue;
                if (typeof htmlArray[toolbarAll[i].row] == 'undefined') htmlArray[toolbarAll[i].row] = '';
                htmlArray[toolbarAll[i].row] += '<li class="sortableItem" id="' + toolbarAll[i].name + '"><img src="' + toolbarAll[i].icon + '" alt="' + toolbarAll[i].title + '" title="' + toolbarAll[i].title + '" /></li>';
            }
        }

        if (typeof htmlArray[5] == 'undefined') htmlArray[5] = '';
        htmlArray[5] += '<li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li><li class="sortableItem group"><img src="' + toolbarAll['__group'].icon + '" alt="' + toolbarAll['__group'].title + '" title="' + toolbarAll['__group'].title + '" /></li>';

        if (typeof htmlArray[6] == 'undefined') htmlArray[6] = '';
        htmlArray[6] += '<li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li><li class="sortableItem spacer"><img src="' + toolbarAll['__spacer'].icon + '" alt="' + toolbarAll['__spacer'].title + '" title="' + toolbarAll['__spacer'].title + '" /></li>';

        if (typeof htmlArray[7] == 'undefined') htmlArray[7] = '';

        for (var j in htmlArray){
            html += '<div class="sortableListDiv"><span class="sortableListSpan"><ul class="sortableRow">' + htmlArray[j] + '</ul></span></div>';
        }
        jQuery('#allButtons').empty().append(html);
    };

    if (typeof(Drupal.ckeditorToolbarToArray) == 'undefined') {
        Drupal.ckeditorToolbarToArray = function (toolbar) {
            toolbar = toolbar.replace(/\r?\n|\r/gmi, '')
                .replace(/\s/gmi, '')
                .replace(/([a-zA-Z0-9]+?):/g, '"$1":')
                .replace(/'/g, '"');

            return JSON.parse(toolbar);
        };
    }

    Drupal.ckeditorToolbaInit();
});