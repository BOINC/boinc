var proj_ws;

function projectStatus()
{
      if ("WebSocket" in window)
      {
            // Let us open a web socket
            proj_ws = new WebSocket("wss://localhost:31416", ['binary']);
               
            var intervalID;

            function send() {
  
                     proj_ws.send("<boinc_gui_rpc_request>\n" +
                             " <get_project_status/>\n</boinc_gui_rpc_request>\n\003");
            }

            function proj(name, url, susp, nomas, detach) {  
                         
                  //List all projects
                  //
                  if (document.getElementById(name) == null) { 

                      var x = document.getElementById("select");
                      if (x != null) {
                         
                         document.getElementById("select").style.fontSize = "large";
                         var option = document.createElement("option");
                         option.id = name;
                         option.style.fontSize = "large";
                         option.text = name;
                         option.setAttribute("suspended", susp);
                         option.setAttribute("nomas", nomas);
                         option.setAttribute("detach", detach);
                         option.value = url;
                         x.add(option);
                      }
                      
                  }
              
                  //Detect project states and if changes are detected pass them to the 
                  //element (projects dropdown menu)
                  //
                  var elem = document.getElementById(name);
                  if (elem != null) {
                      var suspen = elem.getAttribute("suspended");   
                      var change = ((suspen == "false") == susp);
                      if (change == true) elem.setAttribute("suspended", susp);
                  
                      var noma = elem.getAttribute("nomas");   
                      var change2 = ((noma == "false") == nomas);
                      if (change2 == true) elem.setAttribute("nomas", nomas);
                  
                      var deta = elem.getAttribute("detach");   
                      var change3 = (( deta == "false") == detach);
                      if (change3 == true) elem.setAttribute("detach", detach);
                  }
             }

             proj_ws.onopen = function()
             {
                  // Web Socket is connected, send data using send()
                  send();
                  intervalID = setInterval(function () { send(); } , 1000);                       
             };
                               
             proj_ws.onmessage = function (evt)  
             { 
                //if (evt.data instanceof Blob)
                if (typeof evt.data == "string")
                {
                   //var reader = new FileReader()
                   //reader.onload = function () {

                       //var text = reader.result;
                       var text = evt.data;

                       var xmlDoc;
          
                       var name = [];
                       var url = [];
                       var suspended = [];
                       var nomas = [];
                       var detach = [];

                       var result = text.match(/<boinc_gui_rpc_reply>([\s\S]*?)<\/boinc_gui_rpc_reply>/g).map(function(val){

                              xmlDoc = jQuery.parseXML(val);
                       });
                       
                       if (xmlDoc != null) {
  
                             var projects = xmlDoc.getElementsByTagName("project"); 

                             for (var i = 0; i < projects.length; i++) {
                               
                                 if (xmlDoc.getElementsByTagName("project_name")[i].childNodes[0] != null) {
                                    name[i] = xmlDoc.getElementsByTagName("project_name")[i].childNodes[0].nodeValue;
                                    url[i] = xmlDoc.getElementsByTagName("master_url")[i].childNodes[0].nodeValue;
                                    suspended[i] = false;
                                    nomas[i] = false;
                                    detach[i] = false;

                                    //Here we detect what options are set for every project
                                    //
                                    x = xmlDoc.getElementsByTagName("project")[i];
                                    xlen = x.childNodes.length;
                                    y = x.firstChild;

                                    txt = "";
                                    for (var z = 0; z < xlen; z++) {
                                       if (y.nodeType == 1) {
                                          txt = y.nodeName;
                                          if (txt == "suspended_via_gui") suspended[i] = true;
                                          if (txt == "dont_request_more_work") nomas[i] = true;
                                          if (txt == "detach_when_done") detach[i] = true;
                                       }
                                       y = y.nextSibling;
                                    }
 
                                    proj(name[i], url[i], suspended[i], nomas[i], detach[i]);
                                  } 
                              }
                       }
                   //}
                   
                   //reader.readAsText(evt.data);
               }
             };

             proj_ws.onclose = function(event) {

                 /*if (event.code == 1006) {
                     //reset();
                     alert("Client is unresponsive");
                     reset();
                 }*/
                 if (intervalID != 0) clearInterval(intervalID);
             };
      }
      else
      {
           // The browser doesn't support WebSocket
           alert("WebSocket NOT supported by your Browser!");
      }
}

