
class BOINCJavaWrapper {
	public static final long BOINC_DIAG_DUMPCALLSTACKENABLED = 0x00000001L;
	public static final long BOINC_DIAG_HEAPCHECKENABLED = 0x00000002L;
	public static final long BOINC_DIAG_MEMORYLEAKCHECKENABLED = 0x00000004L;
	public static final long BOINC_DIAG_ARCHIVESTDERR = 0x00000008L;
	public static final long BOINC_DIAG_ARCHIVESTDOUT = 0x00000010L;
	public static final long BOINC_DIAG_REDIRECTSTDERR = 0x00000020L;
	public static final long BOINC_DIAG_REDIRECTSTDOUT = 0x00000040L;
	public static final long BOINC_DIAG_REDIRECTSTDERROVERWRITE = 0x00000080L;
	public static final long BOINC_DIAG_REDIRECTSTDOUTOVERWRITE = 0x00000100L;
	public static final long BOINC_DIAG_TRACETOSTDERR = 0x00000200L;
	public static final long BOINC_DIAG_TRACETOSTDOUT = 0x00000400L;
	public static final long BOINC_DIAG_HEAPCHECKEVERYALLOC = 0x00000800L;
	public static final long BOINC_DIAG_BOINCAPPLICATION = 0x00001000L;

	static {
		System.loadLibrary("boincjava");
	}

	native synchronized String resolve_filename(String symName);
	native synchronized int init();
	native synchronized int finish(int status);
	native synchronized int fraction_done(double fraction);
	native synchronized int init_diagnostics(int flags);
	native synchronized boolean time_to_checkpoint();
	native synchronized int checkpoint_completed();
}
