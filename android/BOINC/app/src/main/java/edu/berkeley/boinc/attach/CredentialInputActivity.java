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

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.text.InputType;
import android.text.method.PasswordTransformationMethod;
import android.view.View;
import android.widget.CheckBox;
import androidx.appcompat.app.AppCompatActivity;
import edu.berkeley.boinc.databinding.AttachProjectCredentialInputLayoutBinding;
import edu.berkeley.boinc.utils.Logging;
import java.util.List;

public class CredentialInputActivity extends AppCompatActivity {
    private AttachProjectCredentialInputLayoutBinding binding;

    private ProjectAttachService attachService = null;
    private boolean asIsBound = false;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Logging.logVerbose(Logging.Category.GUI_ACTIVITY, "CredentialInputActivity onCreate");

        doBindService();
        binding = AttachProjectCredentialInputLayoutBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.showPwdCb.setOnClickListener(view -> {
            if(((CheckBox) view).isChecked()) {
                binding.pwdInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
            }
            else {
                binding.pwdInput.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
                binding.pwdInput.setTransformationMethod(PasswordTransformationMethod.getInstance());
            }
        });
    }

    @Override
    protected void onDestroy() {
        doUnbindService();
        super.onDestroy();
    }

    // triggered by continue button
    public void continueClicked(View v) {
        Logging.logVerbose(Logging.Category.USER_ACTION, "CredentialInputActivity.continueClicked.");

        // set credentials in service
        if(asIsBound) {
            // verify input and set credentials if valid.
            final String email = binding.emailInput.getText().toString();
            final String name = binding.nameInput.getText().toString();
            final String password = binding.pwdInput.getText().toString();
            if(attachService.verifyInput(email, name, password)) {
                attachService.setCredentials(email, name, password);
            } else {
                Logging.logWarning(Logging.Category.USER_ACTION, "CredentialInputActivity.continueClicked: empty credentials found");

                return;
            }
        } else {
            Logging.logError(Logging.Category.GUI_ACTIVITY, "CredentialInputActivity.continueClicked: service not bound.");

            return;
        }

        Logging.logVerbose(Logging.Category.USER_ACTION, "CredentialInputActivity.continueClicked: starting BatchProcessingActivity...");

        startActivity(new Intent(this, BatchProcessingActivity.class));
    }

    // triggered by individual button
    public void individualClicked(View v) {
        Logging.logVerbose(Logging.Category.USER_ACTION, "CredentialInputActivity.individualClicked.");

        // set credentials in service, in case user typed before deciding btwn batch and individual attach
        if(asIsBound) {
            final String email = binding.emailInput.getText().toString();
            final String name = binding.nameInput.getText().toString();
            final String password = binding.pwdInput.getText().toString();
            attachService.setCredentials(email, name, password);
        }

        Intent intent = new Intent(this, BatchConflictListActivity.class);
        intent.putExtra("conflicts", false);
        startActivity(new Intent(this, BatchConflictListActivity.class));
    }

    private ServiceConnection mASConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            // This is called when the connection with the service has been established, getService returns
            // the Monitor object that is needed to call functions.
            attachService = ((ProjectAttachService.LocalBinder) service).getService();
            asIsBound = true;

            List<String> values = attachService.getUserDefaultValues();
            binding.emailInput.setText(values.get(0));
            binding.nameInput.setText(values.get(1));
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            // This should not happen
            attachService = null;
            asIsBound = false;
        }
    };

    private void doBindService() {
        // bind to attach service
        bindService(new Intent(this, ProjectAttachService.class), mASConnection, Service.BIND_AUTO_CREATE);
    }

    private void doUnbindService() {
        if(asIsBound) {
            // Detach existing connection.
            unbindService(mASConnection);
            asIsBound = false;
        }
    }
}
