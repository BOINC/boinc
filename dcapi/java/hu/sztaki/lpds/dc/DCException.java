package hu.sztaki.lpds.dc;

import java.lang.Exception;

/**
 * The <code>DCException</code> class represents the errors returned by DC-API.
 *
 * @author Gábor Gombás
 */

public class DCException extends Exception {

	/* Error codes. The values must match the C code. */
	public static final int DC_ERR_CONFIG = 1;
	public static final int DC_ERR_DATABASE = 2;
	public static final int DC_ERR_NOTIMPL = 3;
	public static final int DC_ERR_UNKNOWN_WU = 4;
	public static final int DC_ERR_TIMEOUT = 5;
	public static final int DC_ERR_BADPARAM = 6;
	public static final int DC_ERR_SYSTEM = 7;
	public static final int DC_ERR_INTERNAL = 8;

	private static String messages[] = {
		"No error",
		"Configuration error",
		"Database error",
		"Function not implemented",
		"Unknown work unit",
		"Timeout occured",
		"Bad parameter",
		"Failed system call",
		"Internal library error"
	};

	private int code;

	private DCException(int code, String message) {
		super(message + ": " + messages[code]);
		this.code = code;
	}

	public int getCode() {
		return code;
	}
}
