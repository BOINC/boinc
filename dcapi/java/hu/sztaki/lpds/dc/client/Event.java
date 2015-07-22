package hu.sztaki.lpds.dc.client;

import hu.sztaki.lpds.dc.DCException;

public final class Event {

	private final static int DC_CLIENT_CHECKPOINT = 0;
	private final static int DC_CLIENT_FINISH = 1;
	private final static int DC_CLIENT_MESSAGE = 2;

	/* Event type */
	private int type;

	/* Pointer to the C data structure */
	private long ptr;

	private Event(int type, long ptr) {
		this.type = type;
		this.ptr = ptr;
	}

	public boolean isCheckpointRequest() {
		return type == DC_CLIENT_CHECKPOINT;
	}

	public boolean isFinishRequest() {
		return type == DC_CLIENT_FINISH;
	}

	public boolean isMessage() {
		return type == DC_CLIENT_MESSAGE;
	}

	public native String getMessage() throws DCException;

	native public void finalize();
}
