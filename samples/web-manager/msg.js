
var msg_ws;

function GetMessages() {

    if ("WebSocket" in window) {

        // Let us open a web socket
        msg_ws = new WebSocket("wss://localhost:31416", ['binary']);
        var y = 0;
        var intervalID;

        /*var btn = document.createElement('input');
        btn.type = "button"
        btn.value = "Clear Messages";
        btn.className = "btn btn-primary";
        btn.id = "clear";
        btn.addEventListener('click', function(){
                if (intervalID != 0) clearInterval(intervalID);
                $("#logholder").empty();
                $("#logholder").css('background-color', '#ecf3f6'); 
                document.getElementById("gt").disabled = false;
                clear.parentElement.removeChild(document.getElementById("clear"));
                msg_ws.close();
                });
        document.getElementById("buttons").appendChild(btn);
        $("#logholder").empty();
        document.getElementById("gt").disabled = true;*/

        function send(y) {

            msg_ws.send("<boinc_gui_rpc_request>\n" +
                    "<get_messages>\n<seqno>" + y + "</seqno>\n</get_messages>\n " +
                    "</boinc_gui_rpc_request>\n\003");

        };

        msg_ws.onopen = function() {

            // Web Socket is connected, send data using send()
            send(y);
            intervalID = setInterval(function () { send(y); } , 1000); 

        };

        msg_ws.onmessage = function (evt) {

            //if (evt.data instanceof Blob) {
            if (typeof evt.data == "string") 
            {

                //var reader = new FileReader()
                //reader.onload = function () {

                        //var text = reader.result;
                        var text = evt.data;
                        var xmlDoc;

                        var result = text.match(/<msgs>([\s\S]*?)<\/msgs>/g).map(function(val){

                                xmlDoc = jQuery.parseXML(val);
                                });

                        var elem = document.getElementById("logholder");

                        if (xmlDoc != null) {

                            var msgs = xmlDoc.getElementsByTagName("msg");

                            for (var i = 0; i < msgs.length; i++) {

                                var result = xmlDoc.getElementsByTagName("body")[i].childNodes[0].nodeValue;
                                var time = xmlDoc.getElementsByTagName("time")[i].childNodes[0].nodeValue;

                                var project = xmlDoc.getElementsByTagName("project");
                                if (project[i].childNodes[0] != null) project = "[ " +
                                    project[i].childNodes[0].nodeValue + " ]";
                                else project = "[---]";

                                var para = document.createElement("p");
                                
                                var date = new Date(time * 1000).toString();
                                date = date.substring(0, date.indexOf('GMT'));


                                var node = document.createTextNode(date + project +
                                        "  " + result);

                                para.appendChild(node);

                                elem.appendChild(para);
                                y++;
                                elem.scrollTop = y * 100;
                            }

                            elem.style.backgroundColor = '#9CC';
                        }

                    //}

                    //reader.readAsText(evt.data);
            }
        };

       msg_ws.onclose = function(event) {
  
           /*if (event.code == 1006) {
             reset();
             alert("Client is unresponsive");
           
           }*/
           if (intervalID != 0) clearInterval(intervalID);
 
       };


    } 

    else {
        // The browser doesn't support WebSocket
        alert("WebSocket NOT supported by your Browser!");
    }
}

