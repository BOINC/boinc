function reset(draw_ping_but) {

     $("#buttons").empty(); 
     $("#buttons2").empty();
     $("#drop_proj").empty();
     $("#footer").empty();
     $("#ping_vbox").empty();
     $("#log_exp").hide();
   
     //$("select").empty();
     document.getElementById('top-con').style.borderBottom = 'none';
     //document.getElementById('bottom-con').style.borderTop = 'none';
 
     if (draw_ping_but != false) { 
        $(".ping-but").remove(); 
      
        var btn = document.createElement('input');
        btn.type = "button"
        btn.value = "Ping the BOINC client";
        btn.className = "btn btn-primary ping-but";
        btn.id = "ping";
        btn.addEventListener('click', WebSocketTest);
        document.getElementById("buttons").appendChild(btn);
     }

     if (msg_ws != null) msg_ws.close();
     
     if (task_ws != null) task_ws.close();
    
     if (proj_ws != null) proj_ws.close();
 
     if (document.getElementById("logholder") != null) {
          $("#logholder").empty(); 
          $("#logholder").css('background-color', '#ecf3f6');
     }

     if (document.getElementById("prog") != null) {           
          $("#prog").empty();
          $("#link").empty();
     }
}

/////////////////////////////////////////////////////////////////////
//Tasks Related Functions
/////////////////////////////////////////////////////////////////////

function task_state(state) {
            switch(state) {
                case '0':
                    return "Uninitialized";
                case '1':
                    return "Running";
                case '2':
                    return "Exited";
                case '3':
                    return "Exited";
                case '4':
                    return "Exited";
                case '5':
                    return "Waiting to exit (exceeded limits)";
                case '6':
                    return "Aborted";
                case '7':
                    return "Couldn't start";
                case '8':
                    return "Waiting to exit";
                case '9':
                    return "Suspended";
                case '10':
                    return "Waiting for files to be copied";
                default :
                    return "Unknown state";
            }
}

String.prototype.toHHMMSS = function () {
            var sec_num = parseInt(this, 10); // don't forget the second param
            var hours   = Math.floor(sec_num / 3600);
            var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
            var seconds = sec_num - (hours * 3600) - (minutes * 60);

            if (hours   < 10) {hours   = "0"+hours;}
            if (minutes < 10) {minutes = "0"+minutes;}
            if (seconds < 10) {seconds = "0"+seconds;}
            return hours+':'+minutes+':'+seconds;
}

///////////////////////////////////////////////////////////
//After the progress bar is clicked an alert displays details
//about the task represented by the bar. Another alert
//gives the option of displaying the task's web graphics.
//
//TODO: Avoid alerts.
//
function display_task(xmlDoc, i) {

            var result = xmlDoc.getElementsByTagName("result");
            var name = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue;
            var received_time = new Date(xmlDoc.getElementsByTagName("received_time")[i]
                                          .childNodes[0].nodeValue * 1000).toString();
            received_time = received_time.substring(0, received_time.indexOf('GMT'));
            var report_deadline = new Date(xmlDoc.getElementsByTagName("report_deadline")[i]
                                           .childNodes[0].nodeValue * 1000).toString();
            report_deadline = report_deadline.substring(0, report_deadline.indexOf('GMT'));
            var state = xmlDoc.getElementsByTagName("active_task_state")[i].childNodes[0].nodeValue;
            var cpu_time = xmlDoc.getElementsByTagName("current_cpu_time")[i].childNodes[0].nodeValue;
            var time_since_chkpt = parseInt(cpu_time) - parseInt(xmlDoc.getElementsByTagName("checkpoint_cpu_time")[i]
                                                             .childNodes[0].nodeValue);
            var elapsed_time = xmlDoc.getElementsByTagName("elapsed_time")[i].childNodes[0].nodeValue.toHHMMSS();
            var estimated_remaining_time = xmlDoc.getElementsByTagName("estimated_cpu_time_remaining")[i]
                                                 .childNodes[0].nodeValue.toHHMMSS();
            var fraction_done = (xmlDoc.getElementsByTagName("fraction_done")[i].childNodes[0].nodeValue * 100).toFixed(2);
            var virtual_mem = (xmlDoc.getElementsByTagName("swap_size")[i].childNodes[0].nodeValue / Math.pow(1024,3)).toFixed(2);
            var working_set = (xmlDoc.getElementsByTagName("working_set_size_smoothed")[i].
                                      childNodes[0].nodeValue / Math.pow(1024,2)).toFixed(2);
            var slot = xmlDoc.getElementsByTagName("slot")[i].childNodes[0].nodeValue;
            var pid = xmlDoc.getElementsByTagName("pid")[i].childNodes[0].nodeValue;
            var prog_rate = xmlDoc.getElementsByTagName("progress_rate")[i].childNodes[0].nodeValue * 100 * 3600;
            
            alert("Name: " + name + "\n\nState:  " + task_state(state) + "\n\nReceived Time: " + received_time +
                  "\n\nReport Deadline: " + report_deadline + "\n\nCPU Time: " + cpu_time.toHHMMSS() +
                  "\n\nTime Since Last Checkpoint: " + time_since_chkpt.toString().toHHMMSS() +
                  "\n\nElapsed Time: " + elapsed_time + "\n\nEstimated Remaining Time: " + estimated_remaining_time +
                  "\n\nFraction Done: " + fraction_done + "%" + "\n\nVirtual Memory Size: " + virtual_mem + " GB" +
                  "\n\nWorking Set Size: " + working_set + " MB" + "\n\nDirectory: /slots/" + slot +
                  "\n\nProcess ID: " + pid + "\n\nProgress Rate: " + prog_rate + "%");

            if (task_state(state) == "Running") {
                var retVal = confirm("Do you want to visit the WebGraphics page for this task ?");
                if( retVal == true )
                {
                    x = xmlDoc.getElementsByTagName("active_task")[i];
                    xlen = x.childNodes.length;
                    y = x.firstChild;

                    txt = "";
                    for (i = 0; i <xlen; i++) {
                       if (y.nodeType == 1) {
                          txt = y.nodeName;
                          if (txt == "web_graphics_url") window.open(y.childNodes[0].nodeValue, '_blank');
                       }
                       y = y.nextSibling;
                    }
                 }
            }
};

///////////////////////////////////////////////////////////
//Draw the progress bar representing a task.
//It is clickable.
///////////////////////////////////////////////////////////

function draw_progress_bar(i, xmlDoc) {

            var progress_bar_exists = document.getElementById("progr" + i);
            var fraction = xmlDoc.getElementsByTagName("fraction_done")[i].childNodes[0].nodeValue;
            var name     = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue;
            var url      = xmlDoc.getElementsByTagName("project_url")[i].childNodes[0].nodeValue;

            var handler = function () {
                  display_task(xmlDoc, i);
            }

                if (progress_bar_exists == null) {

                    var t_name = document.createElement('p');
                    t_name.className = "name";
                    t_name.id = 'p' + i;
                    t_name.innerHTML = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue ;
                    document.getElementById('prog').appendChild(t_name);

                    var progress = document.createElement('div');
                    progress.id = 'progr' + i;
                    progress.value = name;
                    progress.className = 'progress';
                    progress.setAttribute('role', 'button');
                    document.getElementById('prog').appendChild(progress);
                    progress.addEventListener('click',handler);

                    var pro = document.createElement('div');
                    var id = 'pro' + i;
                    pro.id = id;
                    pro.className = 'progress progress-bar progress-bar-info';
                    pro.setAttribute('role', 'button');
                    pro.setAttribute('aria-valuenow', fraction);
                    pro.setAttribute('aria-valuemin', '0');
                    pro.setAttribute("aria-valuemax", "100");
                    pro.setAttribute("style","width:" + fraction * 100 + '%');

                    if (fraction > 0.03) {
                        progress.innerHTML = "<span id=progres" + i + " style='color: black'></span>";
                        pro.innerHTML = "<span style='color: black;'>" + (fraction * 100).toFixed(2) +
                                        '%' + "</span>";
                    }
                    else
                        progress.innerHTML = "<span id=progres" + i + " style='color: black'>" +
                                             (fraction * 100).toFixed(2) + '%' + "</span>";

                    document.getElementById('progr' + i).appendChild(pro);
                }
                else {
                    var pro = document.getElementById('pro' + i);
                    var name = document.getElementById('p' + i);
                    var progress = document.getElementById('progres' + i);

                    var element = document.getElementById('progr' + i);
                    var clone = element.cloneNode();
                    while (element.firstChild) {
                        clone.appendChild(element.lastChild);
                    }
                    element.parentNode.replaceChild(clone, element);
                    clone.addEventListener('click', handler);

                    name.innerHTML = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue ;
                    pro.setAttribute("style","width:" + fraction * 100 + '%');
                    if (fraction > 0.03) {
                        pro.innerHTML = "<span style='color: black;'>" + (fraction * 100).toFixed(2) + '%' + "</span>";
                        progress.innerHTML = "";
                    }
                    else {
                        progress.innerHTML =  (fraction * 100).toFixed(2) + '%';
                        pro.innerHTML = "<span style='color: black;'>" + "" + "</span>";
                    }
              }
}

//////////////////////////////////////////////////////////////////////////
//Clear task view for specific task
//////////////////////////////////////////////////////////////////////////

function remove_task(i) {

    var id = "progr" + i;
    var task = document.getElementById(id);
    if (task != null) {
        document.getElementById("prog").removeChild(task);
        document.getElementById("prog").removeChild(document.getElementById("p" + i));
        document.getElementById("div" + i).removeChild(document.getElementById("abrt" + i));
        document.getElementById("div" + i).removeChild(document.getElementById("res" + i));
        document.getElementById("div" + i).removeChild(document.getElementById("susp" + i));
    }
}



//////////////////////////////////////////////////////////////////////////
//Control a task (the actual RPC call)
//////////////////////////////////////////////////////////////////////////

function task_cntrl (name, url, action) {
    
 
     if ("WebSocket" in window)
     {
          var ws = new WebSocket("wss://localhost:31416", ['binary']);

          ws.onopen = function() {
               ws.send("<boinc_gui_rpc_request>\n" +
                    "<" + action + "_result>\n<project_url>" + url + "</project_url>\n" + 
                    "<name>" + name + "</name>\n\n</" + action + "_result>\n " +
                    "</boinc_gui_rpc_request>\n\003");
          }

          ws.onmessage = function(evt)
          {
              //if (evt.data instanceof Blob)
              if (typeof evt.data == "string")
              {
                //var reader = new FileReader()
                //reader.onload = function () {

                   //var text = reader.result;
                   var text = evt.data;
                   var result = text.match(/<boinc_gui_rpc_reply>([\s\S]*?)<\/boinc_gui_rpc_reply>/g).map(function(val){

                          xmlDoc = jQuery.parseXML(val);
                   });

                   if (xmlDoc != null) {
                       var sucs = xmlDoc.getElementsByTagName("success");
                       //if (sucs != null)
                            //alert("Task " + name + " suspended");

                   }
                   ws.close();
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

///////////////////////////////////////////////////////////////////////////////
//Control a project (the actual RPC call)
//////////////////////////////////////////////////////////////////////////////

function proj_cntrl(action) {

       var x = document.getElementById("select").selectedIndex;
       var y = document.getElementById("select").options;
       var id = y[x].id;

       if ("WebSocket" in window)
       {
               // Let us open a web socket
               var ws = new WebSocket("wss://localhost:31416", ['binary']);

               ws.onopen = function()
               {

                    ws.send("<boinc_gui_rpc_request>\n" +
                            "<project_" + action + ">\n" +
                            "<project_url>" + y[x].value + "</project_url>\n" +
                            "</project_" + action + ">\n</boinc_gui_rpc_request>\n\003");
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

                              xmlDoc = jQuery.parseXML(val);
                      }); 
                      ws.close();
       
                      if (action == "detach") {
            
                         document.getElementById("select").remove(x);
                      }

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


///////////////////////////////////////////////////////////////////////////////
//Add a Project. This is the actual RPC call.
//////////////////////////////////////////////////////////////////////////////

function add_project(url, auth) {
     //alert(url + " " + auth);
     
     if ("WebSocket" in window)
     {
          // Let us open a web socket
          var ws = new WebSocket("wss://localhost:31416", ['binary']);

          ws.onopen = function()
          {

              ws.send("<boinc_gui_rpc_request>\n" +
                      "<project_attach>\n" +
                      "<project_url>" + url + "</project_url>\n" +
                      "<authenticator>" + auth + "</authenticator>\n" +
                      "<project_name></project_name>\n" +
                      "</project_attach>\n</boinc_gui_rpc_request>\n\003");
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
                    //alert(text);
                    var result = text.match(/<boinc_gui_rpc_reply>([\s\S]*?)<\/boinc_gui_rpc_reply>/g).map(function(val){

                           xmlDoc = jQuery.parseXML(val);
                           var suc = xmlDoc.getElementsByTagName("success")[0];
                           var text = (new XMLSerializer( )).serializeToString(xmlDoc);
                           if (suc != null) 
                           alert("Successfully added project");
                           else alert(text); 
                           $('#myModal').modal('hide');
                    }); 
                    ws.close();
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

///////////////////////////////////////////////////////////////////////////////
//Draw Buttons
//////////////////////////////////////////////////////////////////////////////

function draw_msg_button() {

     if (document.getElementById("gt") == null) {
                var btn = document.createElement("input");
                btn.type = "button";
                btn.value = "View Event Log";
                btn.className = "btn";
                btn.id = "gt";
                btn.addEventListener('click', GetMessages);
                document.getElementById('buttons').appendChild(btn);
     }
}

function draw_task_button() {
 
    if (document.getElementById("task") == null) {
                var btn = document.createElement("input");
                btn.type = "button";
                btn.value = "Active Tasks";
                btn.className = "btn";
                btn.id = "task";
                btn.addEventListener('click', Tasks);
                document.getElementById('buttons').appendChild(btn);
     } 
}

function draw_vbox_button() {
 
    if (document.getElementById("vbox") == null) {
                var btn = document.createElement("input");
                btn.type = "button";
                btn.value = "VirtualBox Version";
                btn.className = "btn";
                btn.id = "vbox";
                btn.addEventListener('click', GetVboxVersion);
                document.getElementById('buttons').appendChild(btn);
     } 
}

///////////////////////////////////////////////////////////
//Adjust project control options according to project state
///////////////////////////////////////////////////////////

function adjust_proj(){
     
     var xi = document.getElementById("select").selectedIndex;
     var yi = document.getElementById("select").options;
     var id = yi[xi].id;

     ///////////////////////////////////////////////////
     //Give suspend option only if not already suspended
     ///////////////////////////////////////////////////

     if ( document.getElementById(id).getAttribute("suspended") == "false" && (document.getElementById("suspe2") == null)) {

           if (document.getElementById("resum") != null) document.getElementById("resum").remove();

           var sus = document.createElement("li");
           sus.id = "sus";
           var pend = document.createElement("a");
           pend.id = "suspe2";
           pend.setAttribute("href", "#");
           pend.text = "Suspend";
           pend.addEventListener('click', function() {

                proj_cntrl("suspend");
           });
           sus.appendChild(pend);
           document.getElementById("list").appendChild(sus);
     }


     if ( document.getElementById(id).getAttribute("suspended") == "true" && (document.getElementById("suspe2") != null)) {

           if (document.getElementById("sus") != null) document.getElementById("sus").remove();

           var res = document.createElement("li");
           res.id = "resum";
           var ume = document.createElement("a");
           ume.id = "upd2";
           ume.setAttribute("href", "#");
           ume.text = "Resume";
           ume.addEventListener('click', function() {

                  proj_cntrl("resume");
           });
           res.appendChild(ume);
           document.getElementById("list").appendChild(res);
    } 

     ///////////////////////////////////////////////////
     //Give no_more_work option only if not already selected
     ///////////////////////////////////////////////////

     if ( document.getElementById(id).getAttribute("nomas") == "false" && (document.getElementById("nom2") == null)) 
     { 
          if ( document.getElementById("trabajo")) document.getElementById("trabajo").remove(); 

          var no = document.createElement("li");
          no.id = "nom2";
          var mas = document.createElement("a");
          mas.id = "upd2";
          mas.setAttribute("href", "#");
          //ach.value = "Update";
          mas.text = "No More Work";
          mas.addEventListener('click', function() {

                proj_cntrl("nomorework");
          }); 
          no.appendChild(mas);
          document.getElementById("list").appendChild(no);
     }

     if ( document.getElementById(id).getAttribute("nomas") == "true" && (document.getElementById("nom2") != null)) 
     {
      
          if (document.getElementById("nom2")) document.getElementById("nom2").remove();

 
          var si = document.createElement("li");
          si.id = "trabajo";
          var work = document.createElement("a");
          work.id = "upd2";
          work.setAttribute("href", "#");
          //ach.value = "Update";
          work.text = "Allow More Work";
          work.addEventListener('click', function() {

                proj_cntrl("allowmorework");
          }); 
          si.appendChild(work);
          document.getElementById("list").appendChild(si);
     }

     //////////////////////////////////////////////////
     //Give detach_when_done option only if not already set
     //////////////////////////////////////////////////

     if ( document.getElementById(id).getAttribute("detach") == "false" && (document.getElementById("deta") == null)) 
     {
     
          if ( document.getElementById("not")) document.getElementById("not").remove(); 

          var deta = document.createElement("li");
          deta.id = "deta";
          var when = document.createElement("a");
          when.id = "upd2";
          when.setAttribute("href", "#");
          //ach.value = "Update";
          when.text = "Detach When Done";
          when.addEventListener('click', function() {

                proj_cntrl("detach_when_done");
          }); 
          deta.appendChild(when);
          document.getElementById("list").appendChild(deta);
     }

     if ( document.getElementById(id).getAttribute("detach") == "true" && (document.getElementById("deta") != null)) 
     {

          if ( document.getElementById("deta")) document.getElementById("deta").remove(); 

          var not = document.createElement("li");
          not.id = "not";
          var detac = document.createElement("a");
          detac.id = "upd2";
          detac.setAttribute("href", "#");
          //ach.value = "Update";
          detac.text = "Don't Detach When Done";
          detac.addEventListener('click', function() {

                proj_cntrl("dont_detach_when_done");
          }); 
          not.appendChild(detac);
          document.getElementById("list").appendChild(not);
     }
}

///////////////////////////////////////////////////////////
//Options that stay the same no matter what the project state is
///////////////////////////////////////////////////////////

function draw_proj_cntrl_button() {

    if (document.getElementById("select") == null) {

          var y = document.createElement("select");
          y.className = "proj btn-block";
          y.id = "select";
          document.getElementById("drop_proj").appendChild(y);
    }


    if (document.getElementById("select2") == null) {

          var y = document.createElement("div");
          y.className = "dropdown";
          y.id = "select2";
          document.getElementById("buttons2").appendChild(y);
          var x = document.createElement("input");
          x.className = "btn dropdown-toggle proj";
          x.setAttribute("type", "button");
          x.setAttribute("data-toggle", "dropdown");
          //x.text = "Project Commands";
          x.value = "Project Commands";
          x.addEventListener('click', adjust_proj);
          y.appendChild(x);
          var z = document.createElement("span");
          z.className = "caret";
          x.appendChild(z);
          var w = document.createElement("ul");
          w.id = "list";
          w.className = "dropdown-menu";
          w.setAttribute("style", "top:0; bottom: auto; right: 0; left: auto;");
          y.appendChild(w);

          var upd = document.createElement("li");
          var ate = document.createElement("a");
          ate.id = "upd2";
          ate.setAttribute("href", "#");
          ate.text = "Update";
          ate.addEventListener('click', function() {

                proj_cntrl("update");
          }); 
          upd.appendChild(ate);
          w.appendChild(upd);

           var res = document.createElement("li");
           res.id = "resum";
           var ume = document.createElement("a");
           ume.id = "upd2";
           ume.setAttribute("href", "#");
           ume.text = "Resume";
           ume.addEventListener('click', function() {

                  proj_cntrl("resume");
           });
           res.appendChild(ume);
           document.getElementById("list").appendChild(res);

          var re = document.createElement("li");
          var set = document.createElement("a");
          set.id = "upd2";
          set.setAttribute("href", "#");
          //set.value = "Update";
          set.text = "Reset";
          set.addEventListener('click', function() {

                proj_cntrl("reset");
          }); 
          re.appendChild(set);
          w.appendChild(re);

          var det = document.createElement("li");
          var ach = document.createElement("a");
          ach.id = "upd2";
          ach.setAttribute("href", "#");
          //ach.value = "Update";
          ach.text = "Detach";
          ach.addEventListener('click', function() {

                proj_cntrl("detach");
          }); 
          det.appendChild(ach);
          w.appendChild(det);
    }
}

////////////////////////////////////////////////////////////////////
//Add project 
////////////////////////////////////////////////////////////////////

function draw_add_proj_button() {

    if (document.getElementById("add_btn") == null) {
           var btn = document.createElement("input");
           btn.type = "button";
           btn.value = "Add Project";
           btn.className = "btn proj ";
           btn.id = "add_btn";
           btn.addEventListener('click',function(){
                 $("#myModal").modal();
           }); 
           document.getElementById('buttons2').appendChild(btn);
    }
}

function form_submit() {

    //var url_loc = document.getElementById("proj_url").value;
    //var auth = document.getElementById("proj_auth").value;
    var url =  window.location.protocol+ "//" + window.location.hostname + "/" + window.location.pathname.split('/')[1] +
                "/";
    var n_url =  url + "lookup_account_post.php";
  
    var email = document.getElementById("email").value;
    var psw = document.getElementById("psw").value;

 
    var psw_hash = md5(psw+email);
    //alert (psw_hash);
    //alert(url + " " + email + " " + psw);

    //var n_url = "https://lhcathomedev.cern.ch/lhcathome-dev/lookup_account_post.php";
    $.ajax({
        url: n_url,
        type: 'POST',
        dataType: 'xml',
        data: { //'url_loc': url_loc,
                'email_addr': email,
                'passwd_hash':  psw_hash
        },
        success: function(response){

                    var text = (new XMLSerializer( )).serializeToString(response);
                    var err = response.getElementsByTagName("error")[0];
                    if ( err == null) {
                       var auth = response.getElementsByTagName("authenticator")[0].childNodes[0].nodeValue; 
                       //alert(auth);
                       add_project(url, text);
                    } 
                    else alert(text);
                 }
    });
}

////////////////////////////////////////////////////////////////////
//Task Abort button
///////////////////////////////////////////////////////////////////

function draw_abort_btn (i, xmlDoc) {
 
    var btn_exists = document.getElementById("abrt" + i);
    var name     = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue;
    var url      = xmlDoc.getElementsByTagName("project_url")[i].childNodes[0].nodeValue;

    if (btn_exists == null) {
 
        var contain = document.getElementById('div' + i);
        
        var btn = document.createElement('button');
        btn.id = "abrt" + i;
        btn.style.color = "#ff0000";
        btn.addEventListener('click', function() {
  
              task_cntrl(name,url, "abort"); 
        });
 
        contain.appendChild(btn);

        var spn = document.createElement("span");
        spn.className = "glyphicon glyphicon-remove-sign";
        spn.setAttribute('aria-hidden', true);

        document.getElementById("abrt" + i).appendChild(spn);
    }
    else 
    {
        var element = document.getElementById("abrt" + i);
        var clone = element.cloneNode();
        while (element.firstChild) {
             clone.appendChild(element.lastChild);
        }
        element.parentNode.replaceChild(clone, element);
        clone.addEventListener('click', function() {
             
            var retVal = confirm("Do you really want to abort this task? \n" +
                                  "Once aborted your work until now will be lost.");
            if( retVal == true )
            {
               task_cntrl(name,url, "abort");
            }

        });

    }
}

///////////////////////////////////////////////////////////////////
//Task Resume button
///////////////////////////////////////////////////////////////////

function draw_resume_btn (i, xmlDoc) {
 
    var btn_exists = document.getElementById("res" + i);
    var name     = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue;
    var url      = xmlDoc.getElementsByTagName("project_url")[i].childNodes[0].nodeValue;
    var state    = xmlDoc.getElementsByTagName("active_task_state")[i].childNodes[0].nodeValue;

    if (btn_exists == null) {
 
        var contain = document.getElementById('div' + i);
        
        var btn = document.createElement('button');
        btn.id = "res" + i;

        btn.addEventListener('click', function() {
  
              task_cntrl(name,url, "resume"); 
              //$("#res" + i).removeClass("flash");
              $("#susp" + i).removeClass("flash");
              //document.getElementById("res" + i).disabled = true;
        });
 
        contain.appendChild(btn);
        
        if (task_state(state) == "Uninitialized") document.getElementById("res" + i).disabled = false;
        else document.getElementById("res" + i).disabled = true;

        var spn = document.createElement("span");
        spn.className = "glyphicon glyphicon-play";
        spn.setAttribute('aria-hidden', true);

        document.getElementById("res" + i).appendChild(spn);
    }
    else 
    {
        var element = document.getElementById("res" + i);
        var clone = element.cloneNode();
        while (element.firstChild) {
             clone.appendChild(element.lastChild);
        }
        element.parentNode.replaceChild(clone, element);
        clone.addEventListener('click', function() {
             
               task_cntrl(name,url, "resume");
               //$("#res" + i).removeClass("flash");
               $("#susp" + i).removeClass("flash");
               document.getElementById("res" + i).disabled = true;
        });

    }
}

//////////////////////////////////////////////////////////////////////
//Task Suspend button
//////////////////////////////////////////////////////////////////////

function draw_suspend_btn (i, xmlDoc) {
 
    var btn_exists = document.getElementById("susp" + i);
    var name       = xmlDoc.getElementsByTagName("name")[i].childNodes[0].nodeValue;
    var url        = xmlDoc.getElementsByTagName("project_url")[i].childNodes[0].nodeValue;
    var state      = xmlDoc.getElementsByTagName("active_task_state")[i].childNodes[0].nodeValue;

    if (btn_exists == null) {
 
        var contain = document.createElement('div');
        contain.id = "div" + i;
        contain.setAttribute("style","margin-bottom: 54px");
        document.getElementById("link").appendChild(contain);
        
        var btn = document.createElement('button');
        if (task_state(state) == "Uninitialized") $("#susp" + i).addClass("flash");
        //btn.type = "button";
        btn.id = "susp" + i;
        btn.addEventListener('click', function() {
  
              task_cntrl(name,url, "suspend"); 
              //$("#susp" + i).addClass("flash");
              //document.getElementById("res" + i).disabled = false;
        });
 
        contain.appendChild(btn);

        var spn = document.createElement("span");
        spn.className = "glyphicon glyphicon-pause";
        spn.id = "spn" + i;
        spn.setAttribute('aria-hidden', true);

        document.getElementById("susp" + i).appendChild(spn);
    }
    else 
    {
        var element = document.getElementById("susp" + i);
        var clone = element.cloneNode();
        while (element.firstChild) {
             clone.appendChild(element.lastChild);
        }
        element.parentNode.replaceChild(clone, element);
        clone.addEventListener('click', function() {
             
               task_cntrl(name,url, "suspend");
               $("#susp" + i).addClass("flash");
               document.getElementById("res" + i).disabled = false;
        });

    }
}

/////////////////////////////////////////////////////////////////////
//Ping Result
/////////////////////////////////////////////////////////////////////


function draw_ping_ok_result(result) {
   
      var p = document.createElement('div');
      p.className = "col-sm-12";
      p.style.borderRadius = "20px";
      p.style.backgroundColor = "#e7eaec";
      document.getElementById("ping_vbox").appendChild(p);
     
      var tab = document.createElement('table'); 
      tab.id = "tab";
      p.appendChild(tab);
      var tr =  document.createElement('tr');
      //tr.style.backgroundColor = "#e7eaec";
      tab.appendChild(tr);
      var td1 = document.createElement('td');
      td1.style.paddingRight = "15px";
      td1.style.paddingBottom = "10px";
      td1.innerHTML = '<span>Client Status: </span>';
      tr.appendChild(td1);
      var td2 = document.createElement('td');
      td2.style.paddingRight = "1px";
      td2.style.paddingBottom = "10px";
      td2.style.fontSize = "large"; 
      td2.innerHTML = '<span class="label label-success">BOINC v' + result + '</span>'; 
      tr.appendChild(td2);
}

function draw_ping_fail_result() {
  
      if ( document.getElementById("fail_client") == null) { 
         var p = document.createElement('div');
         p.className = "col-sm-2";
         document.getElementById("buttons").appendChild(p);
      
         var pi = document.createElement('h4');
         pi.id = "fail_client";
         pi.className = "col-sm-1";
         pi.innerHTML = '<span class="label label-danger">No running BOINC client detected.</span>';
         p.appendChild(pi);
      }
}
/////////////////////////////////////////////////////////////////////
//Vbox check Result
/////////////////////////////////////////////////////////////////////


function draw_vbox_result(version) {

     
      /*var p = document.createElement('div');
      p.className = "col-sm-2";
      document.getElementById("ping_vbox").appendChild(p);
     */
 
      var tab = document.getElementById('tab'); 
      if (tab != null) {
         var tr =  document.createElement('tr');
         //tr.style.backgroundColor = "#e7eaec";
         tab.appendChild(tr);
         var td1 = document.createElement('td');
         td1.style.paddingRight = "15px";
         td1.style.paddingBottom = "10px";
         td1.innerHTML = '<span>VBox Status: </span>';
         tr.appendChild(td1);
         var td2 = document.createElement('td');
         td2.style.fontSize = "large"; 
 
         if (version != null) {
            td2.innerHTML = '<span class="label label-success">BOINC v' + version + '</span>'; 
            td2.style.paddingRight = "15px";
            td2.style.paddingBottom = "10px";
            tr.appendChild(td2);
         }
         else {
            td2.innerHTML = '<span class="label label-danger">No VirtualBox Detected</span>'; 
            td2.style.paddingRight = "15px";
            td2.style.paddingBottom = "10px";
            tr.appendChild(td2);
         }
      }
}

