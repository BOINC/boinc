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
package edu.berkeley.boinc;

import android.app.Activity;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Point;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.text.SpannableString;
import android.text.style.UnderlineSpan;
import android.util.Log;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;

import java.util.ArrayList;
import java.util.List;

import edu.berkeley.boinc.rpc.ImageWrapper;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.rpc.ProjectInfo;
import edu.berkeley.boinc.rpc.RpcClient;
import edu.berkeley.boinc.utils.Logging;

public class ProjectDetailsFragment extends Fragment {
    private String url;
    private ProjectInfo projectInfo; // might be null for projects added via manual URL attach
    private Project project;
    private List<ImageWrapper> slideshowImages = new ArrayList<>();

    private LayoutInflater li;
    private View root;
    private HorizontalScrollView slideshowWrapper;
    private LinearLayout slideshowHook;
    private ProgressBar slideshowLoading;

    // display dimensions
    private int width;
    private int height;

    private boolean retryLayout = true;

    // BroadcastReceiver event is used to update the UI with updated information from
    // the client.  This is generally called once a second.
    //
    private IntentFilter ifcsc = new IntentFilter("edu.berkeley.boinc.clientstatuschange");
    private BroadcastReceiver mClientStatusChangeRec = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            getCurrentProjectData();
            if(retryLayout) {
                populateLayout();
            }
            else {
                updateChangingItems(root);
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        // get data
        url = getArguments().getString("url");
        getCurrentProjectData();

        setHasOptionsMenu(true); // enables fragment specific menu
        super.onCreate(savedInstanceState);
    }

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        if(Logging.VERBOSE) {
            Log.v(Logging.TAG, "ProjectDetailsFragment onCreateView");
        }
        // Inflate the layout for this fragment
        this.li = inflater;
        View layout = inflater.inflate(R.layout.project_details_layout, container, false);
        root = layout;
        return layout;
    }

    @Override
    public void onAttach(@NonNull Context context) {
        if (context instanceof Activity) {
            final Activity activity = (Activity) context;
            final Display display = activity.getWindowManager().getDefaultDisplay();

            Point size = new Point();
            display.getSize(size);

            width = size.x;
            height = size.y;
        }

        super.onAttach(context);
    }

    @Override
    public void onPause() {
        getActivity().unregisterReceiver(mClientStatusChangeRec);
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        getActivity().registerReceiver(mClientStatusChangeRec, ifcsc);
    }

    @Override
    public void onCreateOptionsMenu(@NonNull Menu menu, MenuInflater inflater) {
        // appends the project specific menu to the main menu.
        inflater.inflate(R.menu.project_details_menu, menu);
        super.onCreateOptionsMenu(menu, inflater);
    }

    @Override
    public void onPrepareOptionsMenu(@NonNull Menu menu) {

        super.onPrepareOptionsMenu(menu);
        if(project == null) {
            return;
        }

        // no new tasks, adapt based on status
        MenuItem nnt = menu.findItem(R.id.projects_control_nonewtasks);
        if(project.getDoNotRequestMoreWork()) {
            nnt.setTitle(R.string.projects_control_allownewtasks);
        }
        else {
            nnt.setTitle(R.string.projects_control_nonewtasks);
        }

        // project suspension, adapt based on status
        MenuItem suspend = menu.findItem(R.id.projects_control_suspend);
        if(project.getSuspendedViaGUI()) {
            suspend.setTitle(R.string.projects_control_resume);
        }
        else {
            suspend.setTitle(R.string.projects_control_suspend);
        }

        // detach, only show when project not managed
        MenuItem remove = menu.findItem(R.id.projects_control_remove);
        if(project.getAttachedViaAcctMgr()) {
            remove.setVisible(false);
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch(item.getItemId()) {
            case R.id.projects_control_update:
                new ProjectOperationAsync().execute(RpcClient.PROJECT_UPDATE);
                break;
            case R.id.projects_control_suspend:
                if(project.getSuspendedViaGUI()) {
                    new ProjectOperationAsync().execute(RpcClient.PROJECT_RESUME);
                }
                else {
                    new ProjectOperationAsync().execute(RpcClient.PROJECT_SUSPEND);
                }
                break;
            case R.id.projects_control_nonewtasks:
                if(project.getDoNotRequestMoreWork()) {
                    new ProjectOperationAsync().execute(RpcClient.PROJECT_ANW);
                }
                else {
                    new ProjectOperationAsync().execute(RpcClient.PROJECT_NNW);
                }
                break;
            case R.id.projects_control_reset:
                showConfirmationDialog(RpcClient.PROJECT_RESET);
                break;
            case R.id.projects_control_remove:
                showConfirmationDialog(RpcClient.PROJECT_DETACH);
                break;
            default:
                if(Logging.WARNING) {
                    Log.w(Logging.TAG, "ProjectDetailsFragment onOptionsItemSelected: could not match ID");
                }
        }

        return super.onOptionsItemSelected(item);
    }

    private void showConfirmationDialog(final int operation) {
        final Dialog dialog = new Dialog(getActivity());
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setContentView(R.layout.dialog_confirm);
        Button confirm = dialog.findViewById(R.id.confirm);
        TextView tvTitle = dialog.findViewById(R.id.title);
        TextView tvMessage = dialog.findViewById(R.id.message);

        // operation-dependent texts
        if(operation == RpcClient.PROJECT_DETACH) {
            final String removeStr = getString(R.string.projects_confirm_detach_confirm);

            tvTitle.setText(getString(R.string.projects_confirm_title, removeStr));
            tvMessage.setText(getString(R.string.projects_confirm_message,
                                        removeStr.toLowerCase(),
                                        project.getProjectName() + " "
                                        + getString(R.string.projects_confirm_detach_message)));
            confirm.setText(removeStr);
        }
        else if(operation == RpcClient.PROJECT_RESET) {
            final String resetStr = getString(R.string.projects_confirm_reset_confirm);

            tvTitle.setText(getString(R.string.projects_confirm_title, resetStr));
            tvMessage.setText(getString(R.string.projects_confirm_message, resetStr.toLowerCase(),
                                        project.getProjectName()));
            confirm.setText(resetStr);
        }

        confirm.setOnClickListener(view -> {
            new ProjectOperationAsync().execute(operation);
            dialog.dismiss();
        });
        Button cancel = dialog.findViewById(R.id.cancel);
        cancel.setOnClickListener(view -> dialog.dismiss());
        dialog.show();
    }

    private void populateLayout() {
        if(project == null) {
            retryLayout = true;
            return; // if data not available yet, return. frequently retrys with onReceive
        }

        retryLayout = false;
        View v = root;

        updateChangingItems(v);

        slideshowWrapper = v.findViewById(R.id.slideshow_wrapper);
        slideshowHook = v.findViewById(R.id.slideshow_hook);
        slideshowLoading = v.findViewById(R.id.slideshow_loading);

        // set website
        TextView website = v.findViewById(R.id.project_url);
        SpannableString content = new SpannableString(project.getMasterURL());
        content.setSpan(new UnderlineSpan(), 0, content.length(), 0);
        website.setText(content);
        website.setOnClickListener(view -> {
            Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(project.getMasterURL()));
            startActivity(i);
        });

        // set general area
        if(projectInfo != null && projectInfo.getGeneralArea() != null) {
            TextView generalArea = v.findViewById(R.id.general_area);
            generalArea.setText(projectInfo.getGeneralArea());
        }
        else {
            LinearLayout wrapper = v.findViewById(R.id.general_area_wrapper);
            wrapper.setVisibility(View.GONE);
        }

        // set specific area
        if(projectInfo != null && projectInfo.getSpecificArea() != null) {
            TextView specificArea = v.findViewById(R.id.specific_area);
            specificArea.setText(projectInfo.getSpecificArea());
        }
        else {
            LinearLayout wrapper = v.findViewById(R.id.specific_area_wrapper);
            wrapper.setVisibility(View.GONE);
        }

        // set description
        if(projectInfo != null && projectInfo.getDescription() != null) {
            TextView description = v.findViewById(R.id.description);
            description.setText(projectInfo.getDescription());
        }
        else {
            LinearLayout wrapper = v.findViewById(R.id.description_wrapper);
            wrapper.setVisibility(View.GONE);
        }

        // set home
        if(projectInfo != null && projectInfo.getHome() != null) {
            TextView home = v.findViewById(R.id.based_at);
            home.setText(projectInfo.getHome());
        }
        else {
            LinearLayout wrapper = v.findViewById(R.id.based_at_wrapper);
            wrapper.setVisibility(View.GONE);
        }

        // load slideshow
        new UpdateSlideshowImagesAsync().execute();
    }

    private void getCurrentProjectData() {
        try {
            List<Project> allProjects = (ArrayList<Project>) BOINCActivity.monitor.getProjects();
            for(Project tmpP : allProjects) {
                if(tmpP.getMasterURL().equals(url)) {
                    this.project = tmpP;
                }
            }
            this.projectInfo = BOINCActivity.monitor.getProjectInfo(url);
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectDetailsFragment getCurrentProjectData could not retrieve project list");
            }
        }
        if(this.project == null) {
            if(Logging.WARNING) {
                Log.w(Logging.TAG,
                      "ProjectDetailsFragment getCurrentProjectData could not find project for URL: " + url);
            }
        }
        if(this.projectInfo == null) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG,
                      "ProjectDetailsFragment getCurrentProjectData could not find project attach list for URL: " +
                      url);
            }
        }
    }

    private void updateChangingItems(View v) {
        try {
            // status
            String newStatus = BOINCActivity.monitor.getProjectStatus(project.getMasterURL());
            LinearLayout wrapper = v.findViewById(R.id.status_wrapper);
            if(!newStatus.isEmpty()) {
                wrapper.setVisibility(View.VISIBLE);
                TextView statusT = v.findViewById(R.id.status_text);
                statusT.setText(newStatus);
            }
            else {
                wrapper.setVisibility(View.GONE);
            }

        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "ProjectDetailsFragment.updateChangingItems error: ", e);
            }
        }
    }

    // executes project operations in new thread
    private final class ProjectOperationAsync extends AsyncTask<Object, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Object... params) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "ProjectOperationAsync doInBackground");
            }
            try {
                Integer operation = (Integer) params[0];
                return BOINCActivity.monitor.projectOp(operation, project.getMasterURL());
            }
            catch(Exception e) {
                if(Logging.WARNING) {
                    Log.w(Logging.TAG, "ProjectOperationAsync error in do in background", e);
                }
            }
            return false;
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if(success) {
                try {
                    BOINCActivity.monitor.forceRefresh();
                }
                catch(RemoteException e) {
                    if(Logging.ERROR) {
                        Log.e(Logging.TAG, "ProjectDetailsFragment.ProjectOperationAsync.onPostExecute() error: ", e);
                    }
                }
            }
            else if(Logging.WARNING) {
                Log.w(Logging.TAG, "ProjectOperationAsync failed.");
            }
        }
    }

    private final class UpdateSlideshowImagesAsync extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG,
                      "UpdateSlideshowImagesAsync updating images in new thread. project: " +
                      project.getMasterURL());
            }
            try {
                slideshowImages =
                        BOINCActivity.monitor.getSlideshowForProject(project.getMasterURL());
            }
            catch(Exception e) {
                if(Logging.WARNING) {
                    Log.w(Logging.TAG, "UpdateSlideshowImagesAsync: Could not load data, clientStatus not initialized.");
                }
                return false;
            }
            return !slideshowImages.isEmpty();
        }

        @Override
        protected void onPostExecute(Boolean success) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG,
                      "UpdateSlideshowImagesAsync success: " + success + " images: " + slideshowImages.size());
            }

            if(success && !slideshowImages.isEmpty()) {
                slideshowLoading.setVisibility(View.GONE);
                for(ImageWrapper image : slideshowImages) {
                    ImageView iv = (ImageView) li.inflate(R.layout.project_details_slideshow_image_layout, null);
                    Bitmap bitmap = image.getImage();
                    assert bitmap != null;
                    if(scaleImages(bitmap.getHeight(), bitmap.getWidth())) {
                        bitmap = Bitmap.createScaledBitmap(image.getImage(),
                                                           image.getImage().getWidth() * 2,
                                                           image.getImage().getHeight() * 2, false);
                    }
                    iv.setImageBitmap(bitmap);
                    slideshowHook.addView(iv);
                }
            }
            else {
                slideshowWrapper.setVisibility(View.GONE);
            }
        }

        private boolean scaleImages(int imageHeight, int imageWidth) {
            return (height >= imageHeight * 2 && width >= imageWidth * 2);
        }
    }
}
