package edu.berkeley.boinc;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.content.DialogInterface.OnClickListener;

public class ConfirmationDialog extends android.support.v4.app.DialogFragment {
    private OnClickListener mConfirmClickListener = null;

    public static ConfirmationDialog newInstance(String title, String message, String confirm) {
        ConfirmationDialog frag = new ConfirmationDialog();
        Bundle args = new Bundle();
        args.putString("title", title);
        args.putString("message", message);
        args.putString("confirm", confirm);
        frag.setArguments(args);
        return frag;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        String title = getArguments().getString("title");
        String message = getArguments().getString("message");
        String confirm = getArguments().getString("confirm");

        return new AlertDialog.Builder(getActivity())
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setTitle(title)
                .setMessage(message)
                .setPositiveButton(confirm, mConfirmClickListener)
                .setNegativeButton("Cancel", new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        dismiss();
                    }
                })
                .create();
    }

    public void setConfirmationClicklistener(OnClickListener cl) {
        mConfirmClickListener = cl;
    }

    public interface ConfirmationDialogListener {
        void onDialogRead();

        void onDialogChecked(boolean isChecked);
    }
}
