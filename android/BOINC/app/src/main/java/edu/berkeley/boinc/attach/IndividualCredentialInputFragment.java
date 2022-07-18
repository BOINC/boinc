/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2020 University of California
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

import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.text.InputType;
import android.text.method.PasswordTransformationMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.CheckBox;
import androidx.annotation.NonNull;
import androidx.fragment.app.DialogFragment;
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper;
import edu.berkeley.boinc.databinding.AttachProjectCredentialInputDialogBinding;
import edu.berkeley.boinc.utils.Logging;
import java.util.List;

public class IndividualCredentialInputFragment extends DialogFragment {
    private String projectName;
    private String errorMessage;
    private String forgotPwdLink;
    private ProjectAttachWrapper project;

    private AttachProjectCredentialInputDialogBinding binding;

    static IndividualCredentialInputFragment newInstance(ProjectAttachWrapper item) {
        IndividualCredentialInputFragment frag = new IndividualCredentialInputFragment();
        frag.projectName = item.getConfig().getName();
        frag.errorMessage = item.getResultDescription();
        frag.forgotPwdLink = item.getConfig().getMasterUrl() + "/get_passwd.php";
        frag.project = item;
        return frag;
    }

    public interface IndividualCredentialInputFragmentListener {
        void onFinish(ProjectAttachWrapper project, Boolean login, String email, String name, String pwd);

        List<String> getDefaultInput();
    }

    private IndividualCredentialInputFragmentListener mListener;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        binding = AttachProjectCredentialInputDialogBinding.inflate(inflater, container, false);

        binding.title.setText(projectName);
        binding.message.setText(errorMessage);

        List<String> defaultValues = mListener.getDefaultInput();
        binding.emailInput.setText(defaultValues.get(0));
        binding.nameInput.setText(defaultValues.get(1));

        binding.loginButton.setOnClickListener(view -> {
            Logging.logVerbose(Logging.Category.USER_ACTION, "IndividualCredentialInputFragment: login clicked");

            final String email = binding.emailInput.getText().toString();
            final String name = binding.nameInput.getText().toString();
            final String password = binding.pwdInput.getText().toString();
            mListener.onFinish(project, true, email, name, password);
            dismiss();
        });

        binding.registerButton.setOnClickListener(view -> {
            Logging.logVerbose(Logging.Category.USER_ACTION,
                    "IndividualCredentialInputFragment: register clicked, client account creation disabled: " +
                    project.getConfig().getClientAccountCreationDisabled());

            if(project.getConfig().getClientAccountCreationDisabled()) {
                // cannot register in client, open website
                Intent i = new Intent(Intent.ACTION_VIEW);
                i.setData(Uri.parse(project.getConfig().getMasterUrl()));
                startActivity(i);
            }
            else {
                final String email = binding.emailInput.getText().toString();
                final String name = binding.nameInput.getText().toString();
                final String password = binding.pwdInput.getText().toString();
                mListener.onFinish(project, false, email, name, password);
                dismiss();
            }
        });

        binding.forgotPwdButton.setOnClickListener(view -> {
            Logging.logVerbose(Logging.Category.USER_ACTION, "IndividualCredentialInputFragment: forgot pwd clicked");

            Intent i = new Intent(Intent.ACTION_VIEW);
            i.setData(Uri.parse(forgotPwdLink));
            startActivity(i);
        });

        binding.showPwdCb.setOnClickListener(view -> {
            if(((CheckBox) view).isChecked()) {
                binding.pwdInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
            }
            else {
                binding.pwdInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
                binding.pwdInput.setTransformationMethod(PasswordTransformationMethod.getInstance());
            }
        });

        return binding.getRoot();
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        binding = null;
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);
        try {
            mListener = (IndividualCredentialInputFragmentListener) context;
        }
        catch (ClassCastException e) {
            Logging.logException(Logging.Category.GUI_ACTIVITY,
                             "IndividualCredentialInputFragment.onAttach The activity doesn't implement the interface. Error: ", e);
        }
    }

    @NonNull
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Dialog dialog = super.onCreateDialog(savedInstanceState);

        // request a window without the title
        dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        return dialog;
    }
}
