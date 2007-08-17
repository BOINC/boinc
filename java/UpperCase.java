
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import java.util.Date;


public class UpperCase {
    private static BOINCJavaWrapper boinc = new BOINCJavaWrapper();
    private static boolean early_exit;
    private static boolean early_crash;
    private static boolean early_sleep;
    private static boolean run_slow;
    private static int cpu_time;
    private static final String ARG_CPU_TIME = "-cpu_time";
    private static final String INPUT_FILENAME = "in";
    private static final String OUTPUT_FILENAME = "out";
    private static final String CHECKPOINT_FILENAME = "upper_case_state";

    private static int do_checkpoint(FileWriter wout, String fnchk, int chars_read)
        throws IOException {
        System.out.println("Java: checkpointing");
        wout.flush();
        FileWriter wchk = new FileWriter("temp");
        wchk.write(Integer.toString(chars_read));
        wchk.flush();
        wchk.close();
        File f = new File("temp");
        File fchk = new File(fnchk);
        f.renameTo(fchk);
        System.out.println("Java: checkpointing done");
        return 0;
    }

    private static void do_work() {
	System.out.println("Java: entering do_work()");
	int ret=0;
        String input_path = "";
        String output_path = "";
        String chkpt_path = "";
        int chars_read = 0;

        //resolve file names
	System.out.println("Java: calling boinc.resolve_filename() for " + INPUT_FILENAME);
        input_path = boinc.resolve_filename(INPUT_FILENAME);
	System.out.println("Java: returned from boinc.resolve_filename() with " + input_path);
        if (input_path==null) {
            System.err.println("Java: exiting");
            System.exit(-1);
        }
	System.out.println("Java: calling boinc.resolve_filename() for " + OUTPUT_FILENAME);
        output_path = boinc.resolve_filename(OUTPUT_FILENAME);
	System.out.println("Java: returned from boinc.resolve_filename() with " + output_path);
        //
	System.out.println("Java: calling boinc.resolve_filename() for " + CHECKPOINT_FILENAME);
        chkpt_path = boinc.resolve_filename(CHECKPOINT_FILENAME);
	System.out.println("Java: returned from boinc.resolve_filename() with " + chkpt_path);
        //
        BufferedReader rin = null;
        BufferedReader rchk = null;
        FileWriter wout = null;
        try {
		System.out.println("Java: createing file for " + input_path);
            File infile = new File(input_path);
            long fl = infile.length();
		System.out.println("Java: open reader for " + input_path);
            rin = new BufferedReader(new FileReader(input_path));
            //check for checkpoint file, read contents in
            File chkptfile = new File(chkpt_path);
            if (chkptfile.exists()) {
		System.out.println("Java: open reader for " + chkpt_path);
                rchk = new BufferedReader(new FileReader(chkpt_path));
                String line = rchk.readLine();
                chars_read = Integer.valueOf(line).intValue();
                rin.skip(chars_read);
                rchk.close();
            }
            //
		System.out.println("Java: open writer for " + output_path);
            wout = new FileWriter(output_path);
            //
            char[] chbuf = new char[1];
            int num = rin.read(chbuf, 0, 1);
            int count = chars_read;
            while (num >= 0) {
                chars_read++;
                count++;
                String s = new String(chbuf);
                char upper = s.toUpperCase().charAt(0);
                wout.write(upper);
                //
                if (run_slow) {
                    Thread.sleep(1000);
                }
                if (early_exit && (count > 30)) {
                    System.exit(-10);
                }
                if (early_crash && (count > 30)) {
                    //provke a runtime (division by zero) exception
                    double t = 1000 / 0;
                }
                if (early_sleep && (count > 30)) {
                    while (true) {
                        Thread.sleep(1000);
                    }
                }


                // checkpointing
		System.out.println("Java: calling boinc.time_to_checkpoint()");
                if (boinc.time_to_checkpoint()) {
			System.out.println("Java: returned from boinc.time_to_checkpoint() with true");
                    	do_checkpoint(wout, chkpt_path, chars_read);
			System.out.println("Java: calling boinc.checkpoint_completed()");
			ret = boinc.checkpoint_completed();
			System.out.println("Java: returned boinc.checkpoint_completed() with " + ret);
                }
		System.out.println("Java: calling boinc.fraction_done()");
                ret = boinc.fraction_done(chars_read / fl);
		System.out.println("Java: returned from boinc.fraction_done() with " + ret);
                num = rin.read(chbuf, 0, 1);
            }

            //use some cpu time
            if (cpu_time > 0) {
                long start = System.currentTimeMillis();
                while (true) {
                    long e = System.currentTimeMillis() - start;
                    if (e > (cpu_time * 1000)) {
                        break;
                    }
                    boinc.fraction_done(e / (cpu_time * 1000));

                    if (boinc.time_to_checkpoint()) {
                        do_checkpoint(wout, chkpt_path, chars_read);
                        boinc.checkpoint_completed();
                    }
                    use_some_cpu();
                }
            }
        } catch (IOException e) {
            System.err.println("Java: " + e.getMessage());
            e.printStackTrace();
            System.exit(-1);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } finally {
            if (rin != null) {
                try {
                    rin.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
            if (rchk != null) {
                try {
                    rchk.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
            if (wout != null) {
                try {
                    wout.flush();
                    wout.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
	System.out.println("Java: leaving do_work()");
    }

    public static void main(String[] args) {
        //enabling boinc diagnostics
	System.out.println("Java: calling boinc.init_diagnostics()");
	int ret =  boinc.init_diagnostics((int)(BOINCJavaWrapper.BOINC_DIAG_DUMPCALLSTACKENABLED |
							BOINCJavaWrapper.BOINC_DIAG_HEAPCHECKENABLED |
							BOINCJavaWrapper.BOINC_DIAG_MEMORYLEAKCHECKENABLED |
							BOINCJavaWrapper.BOINC_DIAG_TRACETOSTDERR |
							BOINCJavaWrapper.BOINC_DIAG_REDIRECTSTDERR));
	System.out.println("Java: returned form boinc.init_diagnostics() with " + ret);
        //parsing arguments
	System.out.println("Java: parsing arguments");
        for (int i = 0; i < args.length; i++) {
            String arg = args[i];
            if (arg.equals("-early_exit")) {
		System.out.println("Java: setting early_exit");
                early_exit = true;
            } else if (arg.equals("-early_crash")) {
		System.out.println("Java: setting early_crash");
                early_crash = true;
            } else if (arg.equals("-early_sleep")) {
		System.out.println("Java: setting early_sleep");
                early_sleep = true;
            } else if (arg.equals("-run_slow")) {
		System.out.println("Java: setting run_slow");
                run_slow = true;
            } else if (arg.startsWith(ARG_CPU_TIME)) {
                String value = arg.substring(ARG_CPU_TIME.length());
                cpu_time = Integer.valueOf(value);
		System.out.println("Java: setting cpu_time to " + cpu_time + " seconds");
            } else {
                System.err.println("Java: unsupported argument(ignored): " + arg);
            }
        }

        //init boinc
	System.out.println("Java: calling boinc.init()");
        ret = boinc.init();
	System.out.println("Java: returned from boinc.init() with " + ret);
        if (ret > 0) {
		System.out.println("Java: exiting");
    		System.exit(ret);
        }
        //do work
        do_work();
	System.out.println("Java: leaving main()");
        //don't call boinc_finish() here to let invoker app do the job cleaning the jvm
	System.exit(0);
    }

    private static void use_some_cpu() {
        double j = 3.14159;
        int i;
        int n = 0;
        for (i = 0; i < 20000000; i++) {
            n++;
            j *= ((n + j) - 3.14159);
            j /= (float) n;
        }
    }
}
