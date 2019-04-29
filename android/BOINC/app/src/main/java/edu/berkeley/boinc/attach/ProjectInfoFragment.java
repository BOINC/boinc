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

import java.io.InputStream;
import java.net.URL;

import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.utils.*;

import android.app.Dialog;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

public class ProjectInfoFragment extends DialogFragment {

    ProjectInfo info;
    LinearLayout logoWrapper;
    ProgressBar logoPb;
    ImageView logoIv;

    public static ProjectInfoFragment newInstance(ProjectInfo info) {
        ProjectInfoFragment f = new ProjectInfoFragment();
        Bundle args = new Bundle();
        args.putParcelable("info", info);
        f.setArguments(args);
        return f;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment onCreateView");
        }
        View v = inflater.inflate(R.layout.attach_project_info_layout, container, false);

        // get data
        info = this.getArguments().getParcelable("info");
        if(info == null) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectInfoFragment info is null, return.");
            }
            dismiss();
            return v;
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment project: " + info.name);
        }

        // set texts
        ((TextView) v.findViewById(R.id.project_name)).setText(info.name);
        ((TextView) v.findViewById(R.id.project_summary)).setText(info.summary);
        ((TextView) v.findViewById(R.id.project_area)).setText(info.generalArea + ": " + info.specificArea);
        ((TextView) v.findViewById(R.id.project_desc)).setText(info.description);
        ((TextView) v.findViewById(R.id.project_home)).setText(
                getResources().getString(R.string.attachproject_login_header_home) + " " + info.home);

        // find view elements for later use in image download
        logoWrapper = v.findViewById(R.id.project_logo_wrapper);
        logoPb = v.findViewById(R.id.project_logo_loading_pb);
        logoIv = v.findViewById(R.id.project_logo);

        // setup return button
        Button continueB = v.findViewById(R.id.continue_button);
        continueB.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "ProjectInfoFragment continue clicked");
                }
                dismiss();
            }
        });

        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "ProjectInfoFragment image url: " + info.imageUrl);
        }
        new DownloadLogoAsync().execute(info.imageUrl);

        return v;
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        Dialog dialog = super.onCreateDialog(savedInstanceState);

        // request a window without the title
        dialog.getWindow().requestFeature(Window.FEATURE_NO_TITLE);
        return dialog;
    }

    private class DownloadLogoAsync extends AsyncTask<String, Void, Bitmap> {

        @Override
        protected Bitmap doInBackground(String... params) {
            String url = params[0];
            if(url == null || url.isEmpty()) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync url is empty, return.");
                }
                return null;
            }

            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync for url: " + url);
            }
            Bitmap logo;

            try {
                InputStream in = new URL(url).openStream();
                logo = BitmapFactory.decodeStream(in);
                // scale
                logo = Bitmap.createScaledBitmap(logo, logo.getWidth() * 2, logo.getHeight() * 2, false);
            }
            catch(Exception e) {
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync image download failed");
                }
                return null;
            }

            return logo;
        }

        protected void onPostExecute(Bitmap logo) {
            if(logo == null) {
                // failed.
                if(Logging.ERROR) {
                    Log.e(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync failed.");
                }
                logoWrapper.setVisibility(View.GONE);
            }
            else {
                // success.
                if(Logging.DEBUG) {
                    Log.d(Logging.TAG, "ProjectInfoFragment DownloadLogoAsync successful.");
                }
                logoPb.setVisibility(View.GONE);
                logoIv.setVisibility(View.VISIBLE);
                logoIv.setImageBitmap(logo);
            }
        }
    }
}
