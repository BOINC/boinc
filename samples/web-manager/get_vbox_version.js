function GetVboxVersion()
         {
            if ("WebSocket" in window)
            {
               // Let us open a web socket
               var ws = new WebSocket("wss://localhost:31416", ['binary']);

               ws.onopen = function()
               {

                   ws.send("<boinc_gui_rpc_request>\n" +
                              "<get_host_info/>\n</boinc_gui_rpc_request>\n\003");
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

                       var result = text.match(/<boinc_gui_rpc_reply>([\s\S]*?)<\/boinc_gui_rpc_reply>/g).map(function(val){
		          
			  var xmlDoc = jQuery.parseXML(val.replace(/<\/?boinc_gui_rpc_reply>/g,''));
                        
			  var version = $(xmlDoc).find("virtualbox_version").text(); 

                          draw_vbox_result(version);
/*
                          if (version != "") 
		                alert("The version of VirtualBox detected is: " + version);

			  else  alert("VirtualBox is not installed.");*/

                       });
                   //}

                   //reader.readAsText(evt.data);
                }

               };

            }

            else
            {
               // The browser doesn't support WebSocket
               alert("WebSocket NOT supported by your Browser!");
            }
         }
