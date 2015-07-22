package hu.sztaki.lpds.dc;

/**
 * The <code>FileMode</code> class contains the valid file mode constants.
 *
 * @author Gábor Gombás
 */

public final class FileMode {
	/* The codes must match their C equivalend */
	private static final int DC_FILE_REGULAR = 0;
	private static final int DC_FILE_PERSISTENT = 1;
	private static final int DC_FILE_VOLATILE = 2;

	/* Public instances */
	public static final FileMode REGULAR = new FileMode(DC_FILE_REGULAR);
	public static final FileMode PERSISTENT = new FileMode(DC_FILE_PERSISTENT);
	public static final FileMode VOLATILE = new FileMode(DC_FILE_VOLATILE);

	/* Class implementation */
	private int code;
	private FileMode(int code) {
		this.code = code;
	}

	int getCode() {
		return code;
	}
};
