package hu.sztaki.lpds.dc;

/**
 * An <code>RuntimeDCException</code> is thrown when the JNI glue code detects
 * an error within itself.
 *
 * @author Gábor Gombás
 */

public class RuntimeDCException extends RuntimeException {

	/**
	 * Creates a new <code>RuntimeDCException</code> with the
	 * given message.
	 *
	 * @param msg		the message of the exception.
	 */
	public RuntimeDCException(String msg) {
		super(msg);
	}

	/**
	 * Creates a new <code>RuntimeDCException</code> with the
	 * given message and cause.
	 *
	 * @param msg		the message of the exception.
	 * @param cause		the cause of the exception.
	 */
	public RuntimeDCException(String msg, Throwable cause) {
		super(msg, cause);
	}

	/**
	 * Creates a new <code>RuntimeDCException</code> with the
	 * given cause.
	 *
	 * @param cause		the cause of the exception.
	 */
	public RuntimeDCException(Throwable cause) {
		super(cause);
	}
}
