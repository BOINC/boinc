function WebSocketTest()
         {
            if ("WebSocket" in window)
            {
               // Let us open a web socket
               var ws = new WebSocket("wss://localhost:31416", ['binary']);

               ws.onopen = function()
               {
                  // Web Socket is connected, send data using send()
                       ws.send("<boinc_gui_rpc_request>\n" +
                               " <exchange_versions>\n"+
                               "<major>BOINC_MAJOR_VERSION</major>\n"+
                               "<minor>BOINC_MINOR_VERSION</minor>\n"+
                               " <release>BOINC_RELEASE</release>\n"+
                               " </exchange_versions>\n\n</boinc_gui_rpc_request>\n\003");
 
                       $(".ping-but").remove();
               };
                                
               ws.onmessage = function (evt)  
               { 
                //if (evt.data instanceof Blob)
                if (typeof evt.data == "string")
                {
                   //var reader = new FileReader()
                   //reader.onload = function () {

                       //var text = reader.result;
                       var text = evt.data;
                     
                       alert(text);                    

                       var xmlDoc;

                       var result = text.match(/<boinc_gui_rpc_reply>([\s\S]*?)<\/boinc_gui_rpc_reply>/g).map(function(val){

                              xmlDoc = jQuery.parseXML(val);
                       });

                       var major = xmlDoc.getElementsByTagName("major")[0].childNodes[0].nodeValue; 
                       var minor = xmlDoc.getElementsByTagName("minor")[0].childNodes[0].nodeValue;
                       var release = xmlDoc.getElementsByTagName("release")[0].childNodes[0].nodeValue;

                       if (major != null && minor != null && release != null) {
                             
                             var result = major + "." + minor + "." + release;
                             //alert ("The version of the detected client is: " + result); 
                            // $("#ping").remove();
                             $(".ping-but").remove();
                             GetVboxVersion();
                             document.getElementById('top-con').style.borderBottom = '3px solid black';
                             //document.getElementById('bottom-con').style.borderTop = '1px solid black';
                             draw_proj_cntrl_button();
                             //draw_msg_button();     
                             //draw_vbox_button();
                             //draw_task_button();
                             draw_add_proj_button();
                             projectStatus();
                             draw_ping_ok_result(result);
                             Tasks();
                             GetMessages();
                             $("#log_exp").show();

                             //alert ("The version of the detected client is: " + result);
                       }
                   //}

                   //reader.readAsText(evt.data);
               }
                
            };

             ws.onclose = function(event) {

                 if (event.code == 1006) {
                     //alert("Client is unresponsive");
                     reset(false);
                     draw_ping_fail_result();
                     //reset();
                 }
                   
             };

                                
            }
            
            else
            {
               // The browser doesn't support WebSocket
               alert("WebSocket NOT supported by your Browser!");
            }
         }

