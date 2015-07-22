package hu.sztaki.lpds.dc.client;

import hu.sztaki.lpds.dc.*;

public class DCClient {

	public final static String DC_CHECKPOINT_FILE = new String("__DC_CHECKPOINT_");

	/* ==========================================================
	 * Class initialization
	 */

	static {
		System.loadLibrary("dc-java-client-" + Version.getVersion());
		if (!Version.getVersion().equals(Version.getNativeVersion()))
			throw new RuntimeException("DC-API version mismatch: " +
				"Java side is " + Version.getVersion() + ", " +
				"JNI side is " + Version.getNativeVersion());
	}

	/* ==========================================================
	 * Methods from dc_common.h
	 */
	public final synchronized static native int getMaxMessageSize();
	public final synchronized static native int getMaxSubresults();
	public final synchronized static native void log(int level, String message);

	public final synchronized static native String getCfgStr(String key);
	public final synchronized static native int getCfgInt(String key, int defaultValue);
	public final synchronized static native boolean getCfgBool(String key, boolean DefaultValue);

	/* ==========================================================
	 * Methods from dc_client.h
	 */
	public final synchronized static native void init() throws DCException;
	public final synchronized static native String resolveFileName(FileType type, String logicalName) throws DCException;
	public final synchronized static native void sendResult(String logicalFileName, String path, FileMode mode) throws DCException;
	public final synchronized static native void sendMessage(String message) throws DCException;
	public final synchronized static native void finish(int exitCode);
	public final synchronized static native void checkpointMade(String path);
	public final synchronized static native void fractionDone(double fraction);
	public final synchronized static native Event checkEvent();
}
