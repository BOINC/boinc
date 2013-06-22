package edu.berkeley.boinc.utils;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import edu.berkeley.boinc.R;

public class BOINCUtils {

	public static String translateRPCReason(Activity activity, int reason) {
	    switch (reason) {
		    case BOINCDefs.RPC_REASON_USER_REQ:
		    	return activity.getResources().getString(R.string.rpcreason_userreq);
		    case BOINCDefs.RPC_REASON_NEED_WORK:
		    	return activity.getResources().getString(R.string.rpcreason_needwork);
		    case BOINCDefs.RPC_REASON_RESULTS_DUE:
		    	return activity.getResources().getString(R.string.rpcreason_resultsdue);
		    case BOINCDefs.RPC_REASON_TRICKLE_UP:
		    	return activity.getResources().getString(R.string.rpcreason_trickleup);
		    case BOINCDefs.RPC_REASON_ACCT_MGR_REQ:
		    	return activity.getResources().getString(R.string.rpcreason_acctmgrreq);
		    case BOINCDefs.RPC_REASON_INIT:
		    	return activity.getResources().getString(R.string.rpcreason_init);
		    case BOINCDefs.RPC_REASON_PROJECT_REQ:
		    	return activity.getResources().getString(R.string.rpcreason_projectreq);
		    default:
		    	return activity.getResources().getString(R.string.rpcreason_unknown);
	    }
	}
	
	public static String translateNetworkSuspendReason(Context ctx, int reason) {
		switch(reason) {
		case BOINCDefs.SUSPEND_REASON_USER_REQ:
			return ctx.getString(R.string.suspend_network_user_req);
		case BOINCDefs.SUSPEND_REASON_WIFI_STATE:
			return ctx.getString(R.string.suspend_wifi);
		default:
			return "" + reason;
		}
	}
	
	@SuppressLint("DefaultLocale")
	public static String formatSize(double fBytesSent, double fFileSize) {
		String buf = new String();
	    double xTera = 1099511627776.0;
	    double xGiga = 1073741824.0;
	    double xMega = 1048576.0;
	    double xKilo = 1024.0;

	    if (fFileSize != 0) {
	        if        (fFileSize >= xTera) {
	            buf = String.format("%.2f/%.2f TB", fBytesSent/xTera, fFileSize/xTera);
	        } else if (fFileSize >= xGiga) {
	        	buf = String.format("%.2f/%.2f GB", fBytesSent/xGiga, fFileSize/xGiga);
	        } else if (fFileSize >= xMega) {
	        	buf = String.format("%.2f/%.2f MB", fBytesSent/xMega, fFileSize/xMega);
	        } else if (fFileSize >= xKilo) {
	        	buf = String.format("%.2f/%.2f KB", fBytesSent/xKilo, fFileSize/xKilo);
	        } else {
	        	buf = String.format("%.0f/%.0f bytes", fBytesSent, fFileSize);
	        }
	    } else {
	        if        (fBytesSent >= xTera) {
	        	buf = String.format("%.2f TB", fBytesSent/xTera);
	        } else if (fBytesSent >= xGiga) {
	        	buf = String.format("%.2f GB", fBytesSent/xGiga);
	        } else if (fBytesSent >= xMega) {
	        	buf = String.format("%.2f MB", fBytesSent/xMega);
	        } else if (fBytesSent >= xKilo) {
	        	buf = String.format("%.2f KB", fBytesSent/xKilo);
	        } else {
	        	buf = String.format("%.0f bytes", fBytesSent);
	        }
	    }

	    return buf;
	}
    
}
