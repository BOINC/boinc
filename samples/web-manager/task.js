var task_ws;

function Tasks()
{
    if ("WebSocket" in window)
    {
        // Let us open a web socket
        task_ws = new WebSocket("wss://localhost:31416", ['binary']);
        var intervalID;
        
        /*var btn = document.createElement('input');
        btn.type = "button"
        btn.value = "Clear Tasks View";
        btn.className = "btn btn-primary";
        btn.id = "clear_task";
        btn.addEventListener('click', function(){
                if (intervalID != 0) clearInterval(intervalID); 
                $("#prog").empty();
                $("#link").empty();
                document.getElementById("task").disabled = false;
                clear_task.parentElement.removeChild(document.getElementById("clear_task"));
                task_ws.close();
        });

        document.getElementById("buttons").appendChild(btn);
        
        document.getElementById("task").disabled = true;*/

        function send() {
            task_ws.send("<boinc_gui_rpc_request>\n" +
                    "<get_results><active_only>1</active_only>\n" + 
                    "<get_results/>" +
                    "\n</boinc_gui_rpc_request>\n\003");         
        }

        task_ws.onopen = function()
        {
            send();
            intervalID = setInterval(send, 1000);

        };

        task_ws.onmessage = function (evt)  
        { 

            //if (evt.data instanceof Blob)
            if (typeof evt.data == "string")
            {
                //var reader = new FileReader()
                //reader.onload = function () {

                    //var text = reader.result;
                    var text = evt.data;

                    //alert(text);
                    var result = text.match(/<boinc_gui_rpc_reply>([\s\S]*?)<\/boinc_gui_rpc_reply>/g).map(function(val){

                          var xmlDoc = jQuery.parseXML(val.replace(/<\/?boinc_gui_rpc_reply>/g,''));
                          var task = xmlDoc.getElementsByTagName("active_task");
                          var y = 0;

                          var xi = document.getElementById("select").selectedIndex;
                          var yi = document.getElementById("select").options;
                          var id = yi[xi].id;
                          sel_url = document.getElementById(id).value;
                          //alert(com_url);

                          $("#prog").empty();
                          $("#link").empty();

                          for (var i = 0; i < task.length; i++) {
    
                           
                               var url = xmlDoc.getElementsByTagName("project_url")[i].childNodes[0].nodeValue;
                               if (url == sel_url) {
                                
                                   draw_progress_bar(i, xmlDoc);
                                   draw_suspend_btn(i, xmlDoc);
                                   draw_resume_btn(i, xmlDoc);
                                   draw_abort_btn(i, xmlDoc);
                               }
                               //else remove_task(i); 
                            
                          }
                    });
                //}

                //reader.readAsText(evt.data);
            }
        };

       task_ws.onclose = function(event)
       { 
          /*if (event.code == 1006) {
               reset();
               alert("Client is unresponsive");
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
