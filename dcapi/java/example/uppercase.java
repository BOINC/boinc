import hu.sztaki.lpds.dc.*;
import hu.sztaki.lpds.dc.client.*;

import java.io.*;
import java.util.*;

class Uppercase {

static {
    System.loadLibrary("dc-java-client-0.9");
}

  private static FileReader infile = null;
  private static FileWriter outfile = null;
  private static FileReader ckptin = null;
  private static FileWriter ckptout = null;
  
  private static int pos = 0;

  public static void main(String argsp[]) {
    
    DCClient cli = new DCClient();
    
    try {
        cli.init();
    } catch (DCException e)
    {
	System.out.println("APP: Failed to initilaize DC-API");
    }
    
    init_files(cli);
    
    do_work(cli);
    
    cli.finish(0);
  }

  private static void init_files(DCClient dc) {
    String file_name = "";
    String ckpt = "";
    
    // INPUT File
    try {
        file_name = dc.resolveFileName(FileType.IN, "in.txt");
    } catch (DCException e) {
	System.out.println("APP: Could not resolve input file name.");
	dc.finish(1);
    }
    try {
      infile = new FileReader(file_name);
    } catch (IOException e) {
      System.out.println("APP: Could not open input file.");
      dc.finish(1);
    }    

    //CKPT File
    try {
      ckptin = new FileReader("ckpt.txt");
      
      try {
        BufferedReader br = new BufferedReader(ckptin);
        ckpt = br.readLine();
      } catch (IOException ex) {
         System.out.println("APP: Could not read ckpt file.");
         dc.finish(1);
      }
    
      StringTokenizer ckptst = new StringTokenizer(ckpt);
      pos = Integer.parseInt(ckptst.nextToken());
    
      try {
      for (int i=0; i<pos; i++) {
        infile.read();
      }
      }catch (IOException er)
      {
         System.out.println("APP: Could not read input file.");
         dc.finish(1);    
      }

 
    } catch (FileNotFoundException e) {
    }
    
    //OUTPUT File
    try {
      file_name = dc.resolveFileName(FileType.OUT, "out.txt");
    } catch (DCException e) {
      System.out.println("APP: Could not resolve output file name.");
      dc.finish(1);
    }
    try {
      outfile = new FileWriter(file_name, true);
     } catch (IOException e) {
        System.out.println("APP: Could not create/open output file.");
        dc.finish(1);
     }
  }
  
  private static void do_work(DCClient dc) {
    int i;
    char c;
    
    try { 
      while ( (i = infile.read()) > -1) {
        c = (char)i;
	pos++;
        try {    
          outfile.write(Character.toUpperCase(c));
        } catch (IOException e) {
            System.out.println("APP: Could not write output file.");
            dc.finish(1);    
        }
	
	do_checkpoint(dc);
	
	Event ev = dc.checkEvent();
        if (ev == null)
          continue;
      
        if(ev.isCheckpointRequest())
          do_checkpoint(dc);
      }
    }catch (IOException e){
      System.out.println("APP: Could not read input file.");
      dc.finish(1);    
    }
    try {
      outfile.close();
    } catch (IOException e)
    {
      System.out.println("APP: Could not close output file.");
      dc.finish(1);         
    }
  }
  
  private static void do_checkpoint(DCClient dc) {

    try {
      outfile.flush();
    } catch (IOException e) {
        System.out.println("APP: Could not flush output file.");
        dc.finish(1);         
    }
    
    try {
      ckptout = new FileWriter("ckpt.txt");
      ckptout.write(String.valueOf(pos));
      ckptout.close();
    }catch (IOException ec) {
      System.out.println("APP: Could not write ckpt file.");
      dc.finish(1);
    }
    
  }
  //end class
}