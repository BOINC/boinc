/*
 * This file is part of BOINC.
 * https://boinc.berkeley.edu
 * Copyright (C) 2025 University of California
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
package edu.berkeley.boinc.attach

import android.app.Dialog
import android.content.Context
import android.content.Intent
import android.os.Bundle
import android.text.InputType
import android.text.method.PasswordTransformationMethod
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.Window
import android.widget.CheckBox
import androidx.core.net.toUri
import androidx.fragment.app.DialogFragment
import edu.berkeley.boinc.attach.ProjectAttachService.ProjectAttachWrapper
import edu.berkeley.boinc.databinding.AttachProjectCredentialInputDialogBinding
import edu.berkeley.boinc.utils.Logging.Category.GUI_ACTIVITY
import edu.berkeley.boinc.utils.Logging.Category.USER_ACTION
import edu.berkeley.boinc.utils.Logging.logException
import edu.berkeley.boinc.utils.Logging.logVerbose

class IndividualCredentialInputFragment : DialogFragment() {
    private var projectName: String? = null
    private var errorMessage: String? = null
    private var forgotPwdLink: String? = null
    private var project: ProjectAttachWrapper? = null

    private var binding: AttachProjectCredentialInputDialogBinding? = null

    interface IndividualCredentialInputFragmentListener {
        fun onFinish(
            project: ProjectAttachWrapper,
            login: Boolean,
            email: String,
            name: String,
            pwd: String
        )

        val defaultInput: List<String?>
    }

    private var mListener: IndividualCredentialInputFragmentListener? = null

    @Suppress("RedundantNullableReturnType")
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        binding = AttachProjectCredentialInputDialogBinding.inflate(inflater, container, false)

        binding!!.title.text = projectName
        binding!!.message.text = errorMessage

        val defaultValues = mListener!!.defaultInput
        binding!!.emailInput.setText(defaultValues[0])
        binding!!.nameInput.setText(defaultValues[1])

        binding!!.loginButton.setOnClickListener { _: View? ->
            logVerbose(
                USER_ACTION,
                "IndividualCredentialInputFragment: login clicked"
            )
            val email = binding!!.emailInput.text.toString()
            val name = binding!!.nameInput.text.toString()
            val password = binding!!.pwdInput.text.toString()
            mListener!!.onFinish(project!!, true, email, name, password)
            dismiss()
        }

        binding!!.registerButton.setOnClickListener { _: View? ->
            logVerbose(
                USER_ACTION,
                "IndividualCredentialInputFragment: register clicked, client account creation disabled: " +
                        project!!.config!!.clientAccountCreationDisabled
            )
            if (project!!.config!!.clientAccountCreationDisabled) {
                // cannot register in client, open website
                val i = Intent(Intent.ACTION_VIEW)
                i.setData(project!!.config!!.masterUrl.toUri())
                startActivity(i)
            } else {
                val email = binding!!.emailInput.text.toString()
                val name = binding!!.nameInput.text.toString()
                val password = binding!!.pwdInput.text.toString()
                mListener!!.onFinish(project!!, false, email, name, password)
                dismiss()
            }
        }

        binding!!.forgotPwdButton.setOnClickListener { _: View? ->
            logVerbose(
                USER_ACTION,
                "IndividualCredentialInputFragment: forgot pwd clicked"
            )
            val i = Intent(Intent.ACTION_VIEW)
            i.setData(forgotPwdLink?.toUri())
            startActivity(i)
        }

        binding!!.showPwdCb.setOnClickListener { view: View ->
            if ((view as CheckBox).isChecked) {
                binding!!.pwdInput.inputType =
                    InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
            } else {
                binding!!.pwdInput.inputType =
                    InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
                binding!!.pwdInput.transformationMethod = PasswordTransformationMethod.getInstance()
            }
        }

        return binding!!.root
    }

    override fun onDestroyView() {
        super.onDestroyView()
        binding = null
    }

    override fun onAttach(context: Context) {
        super.onAttach(context)
        try {
            mListener = context as IndividualCredentialInputFragmentListener
        } catch (e: ClassCastException) {
            logException(
                GUI_ACTIVITY,
                "IndividualCredentialInputFragment.onAttach The activity doesn't implement the interface. Error: ",
                e
            )
        }
    }

    override fun onCreateDialog(savedInstanceState: Bundle?): Dialog {
        val dialog = super.onCreateDialog(savedInstanceState)

        // request a window without the title
        dialog.window!!.requestFeature(Window.FEATURE_NO_TITLE)
        return dialog
    }

    companion object {
        fun newInstance(item: ProjectAttachWrapper): IndividualCredentialInputFragment {
            val frag = IndividualCredentialInputFragment()
            frag.projectName = item.config!!.name
            frag.errorMessage = item.resultDescription
            frag.forgotPwdLink = item.config!!.masterUrl + "/get_passwd.php"
            frag.project = item
            return frag
        }
    }
}
