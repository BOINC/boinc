package com.example.projectapp;

public class BoincStatus {
	public static final int INITIALIZING = 0; //default

	public static final int CHECKING_BOINC_AVAILABLILITY = 1;
	public static final int BOINC_AVAILABLE = 11;
	public static final int BOINC_NOT_AVAILABLE = 12;

	public static final int INSTALLING_BOINC = 2;
	public static final int BOINC_INSTALLED = 21;

	public static final int BINDING_BOINC_CLIENT_REMOTE_SERVICE = 3;
	public static final int CRS_BOUND = 31;
	public static final int CRS_BINDING_FAILED = 32;

	public static final int TRYING_CRS_INTERFACE = 4;
	public static final int CRS_INTERFACE_READY = 41;
	public static final int CRS_INTERFACE_TIMEOUT = 42;

	public static final int ATTACHING_PROJECT = 5;
	public static final int CHECKING_PROJECT_ATTACHED = 50;
	public static final int PROJECT_ATTACHED = 51;
	public static final int PROJECT_NOT_ATTACHED = 52;
}
