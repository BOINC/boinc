package edu.berkeley.boinc.utils;

import android.content.Context;

import java.io.IOException;
import java.io.Reader;

import edu.berkeley.boinc.R;

public class BOINCUtils {
    private BOINCUtils() {}

    public static String readLineLimit(Reader reader, int limit) throws IOException {
        StringBuilder sb = new StringBuilder();

        for(int i = 0; i < limit; i++) {
            int c = reader.read(); //Read in single character
            if(c == -1) {
                return ((sb.length() > 0) ? sb.toString() : null);
            }

            if(((char) c == '\n') || ((char) c == '\r')) { //Found end of line, break loop.
                break;
            }

            sb.append((char) c); // String is not over and end line not found
        }

        return sb.toString(); //end of line was found.
    }

    public static String translateRPCReason(Context ctx, int reason) {
        switch(reason) {
            case BOINCDefs.RPC_REASON_USER_REQ:
                return ctx.getResources().getString(R.string.rpcreason_userreq);
            case BOINCDefs.RPC_REASON_NEED_WORK:
                return ctx.getResources().getString(R.string.rpcreason_needwork);
            case BOINCDefs.RPC_REASON_RESULTS_DUE:
                return ctx.getResources().getString(R.string.rpcreason_resultsdue);
            case BOINCDefs.RPC_REASON_TRICKLE_UP:
                return ctx.getResources().getString(R.string.rpcreason_trickleup);
            case BOINCDefs.RPC_REASON_ACCT_MGR_REQ:
                return ctx.getResources().getString(R.string.rpcreason_acctmgrreq);
            case BOINCDefs.RPC_REASON_INIT:
                return ctx.getResources().getString(R.string.rpcreason_init);
            case BOINCDefs.RPC_REASON_PROJECT_REQ:
                return ctx.getResources().getString(R.string.rpcreason_projectreq);
            default:
                return ctx.getResources().getString(R.string.rpcreason_unknown);
        }
    }
}
