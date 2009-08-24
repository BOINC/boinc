#!/usr/bin/env python

# VMwrpapper.py
# VMwrapper program - lets you use non-BOINC apps with BOINC in volunteers machines or in virtual machines.
#
# Handles:
# - suspend/resume/quit/abort
# - reporting CPU time
# - loss of heartbeat from core client
# - checkpointing (at least at the level of task)
#     * volunteers machine: checkpoint filename of application has to be specified
#     * VM: Takes periodic snapshots of the virtual machine
# - (supposed to also handle trickle messaging in future)
#
# See http://boinc.berkeley.edu/trac/wiki/VmApps for details
# Contributor: Jarno Rantala (jarno.rantala@gmail.com)
#
# The original code was made under the CERN openlab summer student program July-August 2009


import sys, string, xmlrpclib, time, subprocess, signal, os, traceback
from boinc import *
from xml.dom import minidom

JOB_FILENAME = "job.xml"
CHECKPOINT_FILENAME  = "checkpoint.txt"
SERVER_PROXY = 'http://localhost:8080/RPC2'
POLL_PERIOD = 1.0
CHECKPOINT_PERIOD = 15*60
TRICKLE_UP_PERIOD = 120
MAX_WAIT_TIME = 60
TASK_TAGS = ['virtualmachine',
           'image',
           'application',
           'copy_app_to_VM',
           'copy_file_to_VM',
           'stdin_filename',
           'stdout_filename',
           'stderr_filename',
           'copy_file_from_VM',
           'checkpoint_filename',
           'command_line',
           'weight']

#----------------------------------------------------------------------
# Definition of TASK class
#----------------------------------------------------------------------
class TASK:
   virtualmachine = ""
   image = ""
   application = ""
   copy_app_to_VM = 0
   copy_files_to_VM = []
   copy_stdin_to_VM = 0
   stdin_filename = ""
   stdout_filename = ""
   stderr_filename = ""
   copy_files_from_VM = []  
   checkpoint_filename = ""
   command_line = ""
   weight = 1.0
   CmdId = ""
   CmdResults = None
   app_process = None  # instance of Popen class of subprocess Module  
   suspended = 0
   starting_cpu = 0.0
   final_cpu_time = 0.0
   time_checkpointed = 0            
   # contribution of this task to overall fraction done
   final_cpu_time = 0;
   starting_cpu = 0;
   ready = 0;
   exitCode = None

   # how much CPU time was used by tasks before this in the job file
   suspended = 0 # zero or nonzero (false or true)
   wall_cpu_time = 0


   def readTag(self, tag, data):
      if tag == "virtualmachine":
         self.virtualmachine = data
   
      elif tag == "image":
         self.image = data
     
      elif tag == "copy_file_to_VM":
         self.copy_files_to_VM.append(data)
     
      elif tag == "application":
         self.application = data
     
      elif tag == "copy_app_to_VM":
         self.copy_app_to_VM = int(data)

      elif tag == "copy_stdin_to_VM":
         self.copy_stdin_to_VM = int(data)
 
      elif tag == "stdin_filename":
         self.stdin_filename = data

      elif tag == "stdout_filename":
         self.stdout_filename = data

      elif tag == "stderr_filename":
         self.stderr_filename = data

      elif tag == "copy_file_from_VM":
         self.copy_files_from_VM.append(data)
     
      elif tag == "command_line":
         self.command_line = data

      elif tag == "checkpoint_filename":
         self.checkpoint_filename = int(data)

      elif tag == "weight":
         self.weight = int(data)
     
      else:
         sys.stderr.write("Unknown tag: " + tag + "\n")
   
   # Replace file names in command line with the physical names
   # resolved by boinc_resolve_filename-method. Every word which
   # starts "./" is recognised as file name.
   def resolve_commandline(self):
      newline = ""
      for word in self.command_line.split():
         if word[0:2] == "./":
            newline = newline + " " + boinc_resolve_filename(word)
         else:
            newline = newline + " " + word
      self.command_line = newline      

   def kill(self, VMmanage):
      if self.virtualmachine != "":
         VMmanage.saveState(self.virtualmachine) # saves and power off the VM
      else:
         if not self.ready:
            self.app_process.kill()      

   def stop(self, VMmanage):
      self.suspended = 1
      if self.virtualmachine != "":
         VMmanage.pause(self.virtualmachine)  
      else:
         self.app_process.send_signal(signal.SIGSTOP)

   def resume(self, VMmanage):
      self.suspended = 0
      if self.virtualmachine != "":
         VMmanage.unpause(self.virtualmachine)
      else:
         self.app_process.send_signal(signal.SIGCONT)

   def has_checkpointed(self, VMmanage, checkpoint_period):
     
      if self.virtualmachine != "":
         if time.time()-self.time_checkpointed > checkpoint_period:
            # time to checkpoint
            VMmanage.saveSnapshot(self.virtualmachine, self.checkpoint_filename)
            sys.stderr.write("snapshot at time: "+str(time.time())+"\n")
            self.time_checkpointed = time.time()  
            return 1
         else:
            return 0
      else:
         changed = 0
         if self.checkpoint_filename == "":
            return 0
         # is the file changed  ??

   def cpu_time(self, VMmanage):
      if self.virtualmachine != "":
         # linux VM assumed!!!!
         if self.suspended: # we cannot send a cmd to VM which is paused
            return 0

         cmdid = VMmanage.runCmd(self.virtualmachine, "cat", ["/proc/uptime"])

         wait = 1
         tic = time.time()
         while wait:
            for cmd in VMmanage.listFinishedCmds():
               if cmd == cmdid:
                  wait = 0
                  break
            # sys.stderr.write("Wait cpu_time cmd: "+str(wait)+"\n")
            time.sleep(1)
            if time.time() - tic > 10:
               sys.stderr.write("It took too long to get cpu time! \n")
               wait = 0

         res = VMmanage.getCmdResults(cmdid)['out']        
         return reduce( lambda x,y: x-y,  map(float, res.split()) )

      else:
         # cpu of subprocess  
         # sys.stderr.write(str(os.times()[2]) +"\n")    
         return os.times()[2]

   def poll(self, VMmanage = ""):
     
      if self.virtualmachine != "":
         for Cmd in VMmanage.listFinishedCmds():
            if Cmd == self.CmdId:
               self.ready = 1
         if self.ready:
            self.CmdResults = VMmanage.getCmdResults(self.CmdId)
            self.exitCode = self.CmdResults['exitCodeOrSignal']
            self.final_cpu_time = self.CmdResults['resources']['ru_stime'] + self.CmdResults['resources']['ru_utime']

      else:
         self.exitCode = self.app_process.poll()
         if self.exitCode != None:
            self.ready = 1
            self.final_cpu_time = self.cpu_time(VMmanage)

   def VMrunning(self, VMmanage):
      running = 0
      for VM in VMmanage.listRunningVMs():
         if VM == self.virtualmachine:
            running = 1
      return running
   
   def runVM(self, VMmanage, commandline = "", max_wait_time = float('inf')):
      app_path = boinc_resolve_filename(self.application)
      image_path = boinc_resolve_filename(self.image)
      vm_path = boinc_resolve_filename(self.virtualmachine)
      input_path = boinc_resolve_filename(self.stdin_filename)

      # Append wrapper's command-line arguments to those in the job file.
      self.command_line = self.command_line + " " + commandline#+ " < "+ self.stdin_filename

      # Check if the virtual machine is already on client
      doCreateVM = 1
     
      for VM in VMmanage.listAvailableVMs():
         if VM == self.virtualmachine:      
            doCreateVM = 0
     
      if doCreateVM:
         # we assume that base directory of createVM is home_of_boinc/.VirtualBox
         # and that VM is in project directory home_of_boinc/projects/URL_of_project
         # in image_path we have the path of image relative to boinc/slot/n/ directory
         # that's why we have to remove first "../" and then it should be OK.
         image_path = image_path[3:len(image_path)]
         try:
            VMmanage.createVM(self.virtualmachine, image_path)
         except Exception as e:
            sys.stderr.write("Creation of VM failed! \n")
            sys.stderr.write(str(e) + "\n")
            raise Exception(3)          

      # restore snapshot if there is one
      if VMmanage.getState(self.virtualmachine) != "Saved":
         try:
            VMmanage.restoreSnapshot(self.virtualmachine)
         except:
            sys.stderr.write("Restoring snapshot failed. \n")          

      # start VM
      self.time_checkpointed = time.time()
      if not self.VMrunning(VMmanage):
         try:
            VMmanage.start(self.virtualmachine)
         except Exception as e:
            sys.stderr.write("Can't start VM: " + self.virtualmachine + "\n")
            sys.stderr.write(str(e) + "\n")
            raise Exception(3)

      # wait until VM is running
      tic = time.time()
      while not self.VMrunning(VMmanage):
         sys.stderr.write("Wait VM '" + self.virtualmachine + "' to run.\n")
         
         if time.time() - tic > max_wait_time:
            sys.stderr.write("It took too long to VM to run. \n")
            raise Exception(5)  

         time.sleep(1)

      # wait until VM has connected to broker successfully
      tic = time.time()
      while 1:
         try:
            VMmanage.ping(self.virtualmachine)
         except Exception as e:
            sys.stderr.write("waiting for VM to connect to the broker \n")
               
            if time.time() - tic > max_wait_time:
               sys.stderr.write(str(e)+"\n")
               sys.stderr.write("It took too long to VM to connect to the broker. \n")
               raise Exception(5)

            time.sleep(10)
            continue
         break    

      # copy app file to VM
      if self.copy_app_to_VM:
         sys.stderr.write("copy file '"+app_path+"' to VM\n")
         
         [out, err, status] = VMmanage.cpFileToVM(self.virtualmachine, app_path, self.application)
         if status:
            sys.stderr.write("Can't copy app to VM \n")
            sys.stderr.write(err)
            raise Exception(4)  
     
      # copy files file to VM
      for fileName in self.copy_files_to_VM:
         file_path = boinc_resolve_filename(fileName)
         sys.stderr.write("copy file '"+file_path+"' to VM\n")
         
         [out, err, status] = VMmanage.cpFileToVM(self.virtualmachine, file_path, fileName)
         if status:
            sys.stderr.write("Can't copy files to VM \n")
            sys.stderr.write(err)
            raise Exception(4)  


      if self.application != "" or self.command_line != "":  # we put " " to command_line couple of lines above    
         sys.stderr.write("run command on VM: "+self.application+" "+self.command_line+"\n")      
         tic = time.time()
         while 1:
            try:
               self.CmdId = VMmanage.runCmd(self.virtualmachine, self.application, [self.command_line], '{}',                                                                   'None', input_path)
            except Exception as e:
               sys.stderr.write("command didn't succeed.. try again.. \n")
 
               if time.time() - tic > max_wait_time:
                  sys.stderr.write(str(e)+"\n")
                  sys.stderr.write("Running command in VM didn't succeed. \n")
                  raise Exception(5)

               time.sleep(10)
               continue
            break    
       

   def run(self, commandline = ""):
      app_path = boinc_resolve_filename(self.application)

      stdout_file = None
      stdin_file = None
      stderr_file = None

      if self.stdout_filename != "":
         output_path = boinc_resolve_filename(self.stdout_filename)
         sys.stderr.write("stdout file: "+output_path+"\n")
         stdout_file = open(output_path, "a")

      if self.stdin_filename != "":      
         input_path = boinc_resolve_filename(self.stdin_filename)
         sys.stderr.write("stdin file: "+input_path+"\n")
         stdin_file = open(input_path, "r")
 
      if self.stderr_filename != "":
         err_path = boinc_resolve_filename(self.stderr_filename)
         sys.stderr.write("stderr file: "+err_path+"\n")
         stderr_file = open(err_path, "a")      
         
      # Append wrapper's command-line arguments to those in the job file.
      self.command_line = self.command_line + " " + commandline

      # resolve file names in command line
      self.resolve_commandline()
 
      sys.stderr.write("wrapper: running "+app_path+" "+"("+self.command_line+") \n")
           
      # runs application on host machine    
      self.app_process = subprocess.Popen((app_path+" "+self.command_line).split(), 0, None,        
                                                     stdin_file, stdout_file, stderr_file)  

#------------------------------------------------------------------------------------------
# Definitions of used methods        
#------------------------------------------------------------------------------------------
def read_job_file(filename):
   input_path = boinc_resolve_filename(filename)

   # open the job file  
   try:
       infile = boinc_fopen(input_path, 'r')
   except boinc.error:  
      sys.stderr.write("Can't open job file: " + input_path)   
      raise Exception(1)

   jobxml = minidom.parse(infile)
   infile.close()

   # read the context of job file
   try:  
      xmltasks = jobxml.getElementsByTagName("job_desc")[0]
   except IndexError:
      sys.stderr.write("Can't read job file: no 'job_desc' tag \n")
     
   tasks = []
   # read the attributes of tasks
   for xmltask in xmltasks.getElementsByTagName("task"):
      task = TASK()
     
      for tag in TASK_TAGS:
         try:
            taglist = xmltask.getElementsByTagName(tag)
            for tagxml in taglist:
               data = tagxml.childNodes[0].data
               task.readTag(tag, data)  
         except IndexError:
            sys.stderr.write("Task has no "+tag+" \n")

      tasks.append(task)

   # read attributes of VMmanage tasks
   VMmanageTasks = []
   for xmltask in xmltasks.getElementsByTagName("VMmanage_task"):
      task = TASK()
     
      for tag in TASK_TAGS:
         try:
            taglist = xmltask.getElementsByTagName(tag)
            for tagxml in taglist:
               data = tagxml.childNodes[0].data
               task.readTag(tag, data)  
         except IndexError:
            sys.stderr.write("Task has no "+tag+" \n")    
         
      VMmanageTasks.append(task)

   # read attributes of unzip tasks
   unzipTasks = []
   for xmltask in xmltasks.getElementsByTagName("unzip_task"):
      task = TASK()
     
      for tag in TASK_TAGS:
         try:
            taglist = xmltask.getElementsByTagName(tag)
            for tagxml in taglist:
               data = tagxml.childNodes[0].data
               task.readTag(tag, data)  
         except IndexError:
            sys.stderr.write("Task has no "+tag+" \n")    
         
      unzipTasks.append(task)

   jobxml.unlink()
   return [tasks, VMmanageTasks, unzipTasks]

def read_checkpoint(filename):
   ntasks = 0
   cpu = 0

   try:
      f = open(filename, "r");
   except:
      return [ntasks, cpu]

   data = f.readline().split()
   f.close()

   try:
      ntasks = int(data[0])
      cpu = float(data[1])
   except:
      sys.stderr.write("Can't read checkpoint file \n")
      return [0, 0]

   return [ntasks, cpu]      

def poll_boinc_messages(task, VMmanage):
   status = boinc_get_status()
   exit = 0 # if nonzero then VMwrapper should exit
   #sys.stderr.write("status suspended: "+str(status['suspended'])+" \n")
   #sys.stderr.write("status no heartbeat: "+str(status['no_heartbeat'])+" \n")
   #sys.stderr.write("status quit request: "+str(status['quit_request'])+" \n")
   #sys.stderr.write("status abort request: "+str(status['abort_request'])+" \n")

   if status['no_heartbeat'] or status['quit_request'] or status['abort_request']:
      task.kill(VMmanage)
      exit = 1

   if status['suspended']:
      if not task.suspended:
         task.stop(VMmanage)
   else:
      if task.suspended:
         task.resume(VMmanage)

   return exit

def send_status_message(task, VMmanage, frac_done, checkpoint_cpu_time):
   current_cpu_time = task.starting_cpu + task.cpu_time(VMmanage)
   boinc_report_app_status(current_cpu_time, checkpoint_cpu_time, frac_done)

# Support for multiple tasks.
# We keep a checkpoint file that says how many tasks we've completed
# and how much CPU time has been used so far
def write_checkpoint(filename, ntasks, cpu):
   try:
      f = open(filename, "w")
   except IOError:
      sys.stderr.write("Writing checkpoint file failed. \n")

   f.write(str(ntasks)+" "+str(cpu)+"\n")
   f.close()

#def wait(n):
#   tic = time.time()
#   toc = time.time()
#   while toc - tic < n:
#      time.sleep(1)
#      toc = time.time()  


#---------------------------------------------------------------------------------
# MAIN
#---------------------------------------------------------------------------------

# BOINC OPTIONS (bools) :
main_program = 1
check_heartbeat = 1
handle_process_control = 1
send_status_msgs = 1
handle_trickle_ups = 0
handle_trickle_downs = 0

retval = boinc_init_options(main_program, check_heartbeat, handle_process_control,
                            send_status_msgs, handle_trickle_ups, handle_trickle_downs)

if (retval):
        sys.exit(retval)

# read command line
commandline = ""
for arg in sys.argv[1:len(sys.argv)]:
   commandline = commandline + arg + " "

# read job file
[tasks, VMmanageTasks, unzip_tasks] = read_job_file(JOB_FILENAME)

# read checkpoint file
ntasks = 0
[ntasks, checkpoint_cpu_time] = read_checkpoint(CHECKPOINT_FILENAME)
if ntasks > len(tasks):
   sys.stderr.write("Checkpoint file: ntasks "+str(ntasks)+" too large\n")
   boinc_finish(1)
if ntasks == len(tasks):
   sys.stderr.write("Workunit is already computed.\n")
   boinc_finish(0)


# calculate the total weight
total_weight = 0
for task in tasks:
   total_weight = total_weight + task.weight    

sys.stderr.write('ntasks: '+str(ntasks)+" len tasks: "+str(len(tasks))+"\n")  

# check if there is a task which uses VM
isVM = 0
for i in range(ntasks, len(tasks)):
   if tasks[i].virtualmachine != "":
      isVM = 1
      break

# unzip the archives first
for task in unzip_tasks:
   task.run()

for task in unzip_tasks:
   task.poll()
   tic = time.time()
   while not task.ready:
      if time.time() - tic > MAX_WAIT_TIME:
         sys.stderr.write("Unzip task "+task.application + " "+task.command_line+" take too long. \n")
         task.kill()
         boinc_finish(5)

      sys.stderr.write("Wait for unzip tasks. \n")
      time.sleep(2)
      task.poll()
   
# try-finally structure makes sure that we kill the VMmanage tasks also when
# there is an error.
exitStatus = 0
try:

   # If there is a task which uses VM we start VMmanageTasks
   if isVM:
      for task in VMmanageTasks:
         task.run()
     
      VM_MANAGE = xmlrpclib.ServerProxy(SERVER_PROXY)

      # wait until VMmanageTasks started succesfully
      tic = time.time()
      time.sleep(5)
      VMmanageRunning = 0
      while 1:
         
         # check that tasks are still running
         for task in VMmanageTasks:
            task.poll()
            if task.ready:
               sys.stderr.write("VM manage tasks "+task.application+" not running. Exit status: "+str(task.exitCode)+"\n")
               raise Exception(5)

         try:
            VM_MANAGE.listAvailableVMs()
         except Exception as e:
            sys.stderr.write("Wait VM manage tasks to start. \n")
            traceback.print_exc()
            if time.time() - tic > MAX_WAIT_TIME:
               sys.stderr.write(str(e)+"\n")
               sys.stderr.write("It took too long to VM manage tasks to start. \n")
               raise Exception(5)
            time.sleep(5)
            continue

         sys.stderr.write("VM manage tasks started successfully \n")
         VMmanageRunning = 1
         break
   else:
      VM_MANAGE = []      

   weight_done = 0
   for i in range(ntasks, len(tasks)):
       weight_done = weight_done + tasks[i].weight
       frac_done = weight_done / total_weight
       tasks[i].starting_time = checkpoint_cpu_time
   
       sys.stderr.write('task number: '+str(i)+"\n")

       if tasks[i].virtualmachine != "":
         # check VMmanageTasks process
         #for task in VMmanageTasks:
         #   [ready, status] = task.poll()  
         #   if ready:
         #      sys.stderr.write("App '"+task.application+"' is not running. Exit status: "+str(status)+"\n")
         #      boinc_finish(195); # EXIT_CHILD_FAILED

         # run app in VM (JOB_FILENAME used as a connection test file)
         sys.stderr.write('run virtual machine: "'+tasks[i].virtualmachine+'"\n')      
         tasks[i].runVM(VM_MANAGE, commandline, MAX_WAIT_TIME)
       else:
         # run app in host
         sys.stderr.write('run application on Host: "'+tasks[i].application+'"\n')
         tasks[i].run(commandline)

       # wait for the task to accomplish    
       while 1:
          tasks[i].poll(VM_MANAGE)
         
          if tasks[i].ready:
             if tasks[i].exitCode:
                sys.stderr.write("App exit code "+str(task.exitCode)+" \n")
                sys.stderr.write("App stderr: \n")
                sys.stderr.write(tasks[i].CmdResults['err']+"\n")
                raise Exception(195) # EXIT_CHILD_FAILED  
             break

          if poll_boinc_messages(tasks[i], VM_MANAGE):
             # VMwrapper should exit cleanly
             raise Exception(0)
     
          if task.has_checkpointed(VM_MANAGE, CHECKPOINT_PERIOD):
             checkpoint_cpu_time = tasks[i].starting_cpu + tasks[i].cpu_time(VM_MANAGE)
             write_checkpoint(CHECKPOINT_FILENAME, i, checkpoint_cpu_time)
       
          send_status_message(tasks[i], VM_MANAGE, frac_done, checkpoint_cpu_time)
          time.sleep(POLL_PERIOD)
 
       if tasks[i].virtualmachine != "":
 
          # write stdout and stderr
          if tasks[i].stdout_filename != "":
             fpath = boinc_resolve_filename(tasks[i].stdout_filename)
             fout = open(fpath, "w")
             fout.write(tasks[i].CmdResults['out'])

          if tasks[i].stderr_filename != "":
             fpath = boinc_resolve_filename(tasks[i].stderr_filename)
             ferr = open(fpath, "w")
             ferr.write(tasks[i].CmdResults['err'])
          else:
             sys.stderr.write(tasks[i].CmdResults['err'])
 
          # if we are going to copy files to VM we need to try that VM has connected to
          # broker successfully
          if len(tasks[i].copy_files_from_VM) > 0:
             tic = time.time()
             while 1:
                try:
                   VM_MANAGE.ping(tasks[i].virtualmachine)
                except Exception as e:
                   sys.stderr.write("waiting for VM to connect to the broker \n")
                   sys.stderr.write(str(e)+"\n")
                   if time.time() - tic > MAX_WAIT_TIME:
                      sys.stderr.write(str(e)+"\n")
                      sys.stderr.write("It took too long to VM to connect to the broker. \n")
                      raise Exception(5)

                   time.sleep(10)
                   continue
                break    

          # copy files from VM
          for fileName in tasks[i].copy_files_from_VM:
             file_path = fileName # is it always true?
             sys.stderr.write("Copy file from VM: "+file_path+"\n")
             [out, err, status] = VM_MANAGE.cpFileFromVM(tasks[i].virtualmachine, file_path, fileName)
             if status:
               sys.stderr.write("Can't copy file from VM \n")
               sys.stderr.write(err)
               raise Exception(4)

          # save state of VM and kill it (VMMain.py is running on VM!)
          VM_MANAGE.saveState(tasks[i].virtualmachine) # or do we want to power off??
       
       checkpoint_cpu_time = tasks[i].starting_cpu + tasks[i].final_cpu_time
       write_checkpoint(CHECKPOINT_FILENAME, i+1, checkpoint_cpu_time)

# read the exit status from raised exception
except Exception as e:
   sys.stderr.write(str(e)+"\n")
   traceback.print_exc()
   if type(e.args[0]) == int:
      exitStatus = e.args[0]

finally:
   if isVM:
      # save state of running VMs
      if VMmanageRunning: # otherwise listRunningVMs() never exits
         for task in tasks:
            if task.virtualmachine != "":
               try:
                  for VM in VM_MANAGE.listRunningVMs():
                     if VM == task.virtualmachine:
                        VM_MANAGE.saveState(task.virtualmachine)
               except:
                  sys.stderr.write("Couldn't save state of "+task.virtualmachine+". \n")
                  traceback.print_exc()
 
      # kill VM manage tasks        
      for task in VMmanageTasks:
         sys.stderr.write("Kill the VM manage task: "+task.application+" \n")
         task.kill(VM_MANAGE)
     
   boinc_finish(exitStatus)
