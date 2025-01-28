/**
 * @file
 *
 * Javascript for flag_abuse_reason. Shows the dropdown box defined in
 * the template.
 */

/**
 * Useful variables/parameters
 *
 * data.contentId   - Id of content, e.g., node id, comment id, etc.
 * data.contentType - Type of content, e.g., node, comment, etc.
 * data.flagName    - Name of the flag.
 * data.flagStatus  - State of the flag.
 */

$(document).bind('flagGlobalBeforeLinkUpdate', function(event, data) {
  /* dd is the dropdown defined the boinc template.
   */
  var dd = 'flag_abuse_reason-dropdown-' + data.contentType + '-' + data.contentId;
  var ddelement = document.getElementById(dd);

  // Only node, comment, and user types are defined.
  switch (data.contentType) {
    case 'node':
    /**
     * Node
     */
    // User clicks on the Report link
    if ( window.getComputedStyle(ddelement).display === "none" &&
         data.flagName == "abuse_node_meta" &&
         data.flagStatus == "flagged" ) {
      ddelement.style.display = "block";
    }

    if ( window.getComputedStyle(ddelement).display === "block" ) {
      // User clicks on any of the flags in the drop down
      if ( data.flagName != "abuse_node_meta" &&
  	 data.flagStatus == "flagged" ) {
        ddelement.style.display = "none";
      }
      // User clicks on Cancel Report
      else if ( data.flagName == "abuse_node_meta" &&
  	      data.flagStatus == "unflagged" ) {
        ddelement.style.display = "none";
      }
    }

    // User unflags by Cancel Report - refresh page
    if ( window.getComputedStyle(ddelement).display === "none" &&
         data.flagName == "abuse_node_meta" &&
         data.flagStatus == "unflagged" ) {
	window.location.reload();
    }

    break;

    case 'comment':
    /**
     * Comments
     */
    // User clicks on the Report link
    if ( window.getComputedStyle(ddelement).display === "none" &&
         data.flagName == "abuse_comment_meta" &&
         data.flagStatus == "flagged" ) {
      ddelement.style.display = "block";
    }

    if ( window.getComputedStyle(ddelement).display === "block" ) {
      // User clicks on any of the flags in the drop down
      if ( data.flagName != "abuse_comment_meta" &&
  	 data.flagStatus == "flagged" ) {
        ddelement.style.display = "none";
      }
      // User clicks on Cancel Report
      else if ( data.flagName == "abuse_comment_meta" &&
  	      data.flagStatus == "unflagged" ) {
        ddelement.style.display = "none";
      }
    }

    // User unflags by Cancel Report - refresh page
    if ( window.getComputedStyle(ddelement).display === "none" &&
         data.flagName == "abuse_comment_meta" &&
         data.flagStatus == "unflagged" ) {
	window.location.reload();
    }

    break;

    case 'user':
  /**
   * User
   */
  // User clicks on the Report link
    if ( window.getComputedStyle(ddelement).display === "none" &&
         data.flagName == "abuse_user_meta" &&
         data.flagStatus == "flagged" ) {
      ddelement.style.display = "block";
    }

    if ( window.getComputedStyle(ddelement).display === "block" ) {
      // User clicks on any of the flags in the drop down
      if ( data.flagName != "abuse_user_meta" &&
  	 data.flagStatus == "flagged" ) {
        ddelement.style.display = "none";
      }
      // User clicks on Cancel Report
      else if ( data.flagName == "abuse_user_meta" &&
  	      data.flagStatus == "unflagged" ) {
        ddelement.style.display = "none";
      }
    }

    // User unflags by Cancel Report - refresh page
    if ( window.getComputedStyle(ddelement).display === "none" &&
         data.flagName == "abuse_user_meta" &&
         data.flagStatus == "unflagged" ) {
	window.location.reload();
    }

    break;
  } //switch

});

