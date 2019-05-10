/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2012 University of California
 *
 * BOINC is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * BOINC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
 */

package edu.berkeley.boinc.attach;

import java.util.ArrayList;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.utils.*;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;

import android.app.Activity;
import android.app.Dialog;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.text.InputType;
import android.text.method.PasswordTransformationMethod;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

public class IndividualCredentialInputFragment extends DialogFragment {

    private String projectName;
    private String errorMessage;
    private String forgotPwdLink;
    private ProjectAttachWrapper project;

    private EditText emailET;
    private EditText nameET;
    private EditText pwdET;

    static IndividualCredentialInputFragment newInstance(ProjectAttachWrapper item) {
        IndividualCredentialInputFragment frag = new IndividualCredentialInputFragment();
        frag.projectName = item.config.name;
        frag.errorMessage = item.getResultDescription();
        frag.forgotPwdLink = item.config.masterUrl + "/get_passwd.php";
        frag.project = item;
        return frag;
    }

    public interface IndividualCredentialInputFragmentListener {
        void onFinish(ProjectAttachWrapper project, Boolean login, String email, String name, String pwd);

        ArrayList<String> getDefaultInput();
    }

    IndividualCredentialInputFragmentListener mListener;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.attach_project_credential_input_dialog, container, false);

        TextView title = v.findViewById(R.id.title);
        title.setText(projectName);
        TextView message = v.findViewById(R.id.message);
        message.setText(errorMessage);

        ArrayList<String> defaultValues = mListener.getDefaultInput();
        emailET = v.findViewById(R.id.email_input);
        emailET.setText(defaultValues.get(0));
        nameET = v.findViewById(R.id.name_input);
        nameET.setText(defaultValues.get(1));
        pwdET = v.findViewById(R.id.pwd_input);

        Button loginButton = v.findViewById(R.id.login_button);
        loginButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "IndividualCredentialInputFragment: login clicked");
                }
                mListener.onFinish(project, true, emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString());
                dismiss();
            }
        });

        Button registerButton = v.findViewById(R.id.register_button);
        registerButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG,
                          "IndividualCredentialInputFragment: register clicked, client account creation disabled: " +
                          project.config.clientAccountCreationDisabled);
                }
                if(project.config.clientAccountCreationDisabled) {
                    // cannot register in client, open website
                    Intent i = new Intent(Intent.ACTION_VIEW);
                    i.setData(Uri.parse(project.config.masterUrl));
                    startActivity(i);
                }
                else {
                    mListener.onFinish(project, false, emailET.getText().toString(), nameET.getText().toString(), pwdET.getText().toString());
                    dismiss();
                }
            }
        });

        TextView forgotPwdButton = v.findViewById(R.id.forgotpwd_text);
        forgotPwdButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "IndividualCredentialInputFragment: forgot pwd clicked");
                }
                Intent i = new Intent(Intent.ACTION_VIEW);
                i.setData(Uri.parse(forgotPwdLink));
                startActivity(i);
            }
        });

        CheckBox showPwdCb = v.findViewById(R.id.show_pwd_cb);
        showPwdCb.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if(((CheckBox) v).isChecked()) {
                    pwdET.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
                }
                else {
                    pwdET.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
                    pwdET.setTransformationMethod(PasswordTransformationMethod.getInstance());
                }
            }
        });

        return v;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        try {
            mListener = (IndividualCredentialInputFragmentListener) activity;
        }
        catch(ClassCastException e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "IndividualCredentialInputFragment.onAttach The activity doesn't implement the interface. Error: ", e);
            }
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Dialog dialog = super.onCreateDialog(savedInstanceState);

        // request a window without the title
        dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        return dialog;
    }
}
