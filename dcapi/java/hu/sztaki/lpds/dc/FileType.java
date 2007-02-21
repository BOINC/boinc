package hu.sztaki.lpds.dc;

/**
 * The <code>FileType</code> class contains the valid file type constants.
 *
 * @author Gábor Gombás
 */

public final class FileType {
	/* The codes must match their C equivalend */
	private static final int DC_FILE_IN = 0;
	private static final int DC_FILE_OUT = 1;
	private static final int DC_FILE_TMP = 2;

	/* Public instances */
	public static final FileType IN = new FileType(DC_FILE_IN);
	public static final FileType OUT = new FileType(DC_FILE_OUT);
	public static final FileType TMP = new FileType(DC_FILE_TMP);

	/* Class implementation */
	private int code;
	private FileType(int code) {
		this.code = code;
	}

	int getCode() {
		return code;
	}
};
