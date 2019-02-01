/*
 * This file is part of BOINC.
 * http://boinc.berkeley.edu
 * Copyright (C) 2016 University of California
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
package edu.berkeley.boinc.adapter;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.utils.Logging;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Iterator;

public class NavDrawerListAdapter extends BaseAdapter {

    //private final String TAG = "NavDrawerListAdapter";
    private Context context;
    private ArrayList<NavDrawerItem> navDrawerItems = new ArrayList<>();

    public int selectedMenuId = 0;

    public NavDrawerListAdapter(Context context) {
        this.context = context;

        // populate items
        navDrawerItems.add(new NavDrawerItem(R.string.tab_tasks, R.drawable.tabtaskb, true));
        navDrawerItems.add(new NavDrawerItem(R.string.tab_notices, R.drawable.mailb, true));
        navDrawerItems.add(new NavDrawerItem(R.string.tab_projects, R.drawable.projectsb));
        navDrawerItems.add(new NavDrawerItem(R.string.projects_add, R.drawable.sqplusb, false, true));
        navDrawerItems.add(new NavDrawerItem(R.string.tab_preferences, R.drawable.cogsb));
        navDrawerItems.add(new NavDrawerItem(R.string.menu_help, R.drawable.helpb));
        navDrawerItems.add(new NavDrawerItem(R.string.menu_report_issue, R.drawable.bugb));
        navDrawerItems.add(new NavDrawerItem(R.string.menu_about, R.drawable.infob));
        navDrawerItems.add(new NavDrawerItem(R.string.menu_eventlog, R.drawable.attentionb));
    }

    @Override
    public int getCount() {
        return navDrawerItems.size();
    }

    @Override
    public NavDrawerItem getItem(int position) {
        return navDrawerItems.get(position);
    }

    @Override
    public long getItemId(int position) {
        return navDrawerItems.get(position).id;
    }

    public NavDrawerItem getItemForId(int id) {
        for(NavDrawerItem item : navDrawerItems) {
            if(item.id == id) {
                return item;
            }
        }
        return null;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "NavDrawerListAdapter.getView() for : " + navDrawerItems.get(position).title +
                               navDrawerItems.get(position).isCounterVisible + navDrawerItems.get(position).isSubItem +
                               navDrawerItems.get(position).isProjectItem);
        }
        if(convertView == null || !(convertView.getTag()).equals(navDrawerItems.get(position).title)) {
            int layoutId = R.layout.navlist_listitem;
            if(navDrawerItems.get(position).isSubItem()) {
                layoutId = R.layout.navlist_listitem_subitem;
            }
            LayoutInflater mInflater = (LayoutInflater) context.getSystemService(Activity.LAYOUT_INFLATER_SERVICE);
            convertView = mInflater.inflate(layoutId, null);
        }

        RelativeLayout wrapper = convertView.findViewById(R.id.listitem);
        ImageView imgIcon = convertView.findViewById(R.id.icon);
        TextView txtTitle = convertView.findViewById(R.id.title);
        TextView txtCount = convertView.findViewById(R.id.counter);

        if(navDrawerItems.get(position).isProjectItem) {
            Bitmap icon = navDrawerItems.get(position).getProjectIcon();
            if(icon == null) {
                navDrawerItems.get(position).updateProjectIcon();
            }
            if(icon != null) {
                imgIcon.setImageBitmap(icon);
            }
        }
        else {
            imgIcon.setImageResource(navDrawerItems.get(position).getIcon());
        }
        txtTitle.setText(navDrawerItems.get(position).getTitle());

        // displaying count
        // check whether it set visible or not
        if(navDrawerItems.get(position).getCounterVisibility()) {
            Integer counter = 0;
            switch(navDrawerItems.get(position).id) {
                case R.string.tab_tasks:
                    try {
                        counter = BOINCActivity.monitor.getTasks().size();
                    }
                    catch(Exception e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "NavDrawerListAdapter.getView error: ", e);
                        }
                    }
                    break;
                case R.string.tab_notices:
                    try {
                        counter = BOINCActivity.monitor.getRssNotices().size();
                    }
                    catch(Exception e) {
                        if(Logging.ERROR) {
                            Log.e(Logging.TAG, "NavDrawerListAdapter.getView error: ", e);
                        }
                    }
                    break;
            }
            txtCount.setText(NumberFormat.getIntegerInstance().format(counter));
        }
        else {
            // hide the counter view
            txtCount.setVisibility(View.GONE);
        }

        // highligt entry of currently activated item
        if(navDrawerItems.get(position).id == selectedMenuId) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "NavDrawerListAdapter.getView() highlighted! ID : " + selectedMenuId);
            }
            wrapper.setBackgroundResource(R.drawable.navlist_selector_pressed);
        }
        else {
            wrapper.setBackgroundResource(R.drawable.navlist_selector);
        }

        convertView.setTag(navDrawerItems.get(position).title);
        return convertView;
    }

    public Bitmap getProjectIconForMasterUrl(String masterUrl) {
        Bitmap bm = null;
        try {
            bm = BOINCActivity.monitor.getProjectIcon(masterUrl);
        }
        catch(Exception e) {
            if(Logging.ERROR) {
                Log.e(Logging.TAG, "NavDrawerListAdapter.getProjectIconForMasterUrl error: ", e);
            }
        }
        return bm;
    }

    /**
     * Compares list of projects to items represented in nav bar.
     *
     * @param projects Project list
     * @return Returns number of project items in nav bar after adding
     */
    public Integer compareAndAddProjects(ArrayList<Project> projects) {
        // delete all old projects from nav items
        Iterator<NavDrawerItem> it = navDrawerItems.iterator();
        while(it.hasNext()) {
            NavDrawerItem item = it.next();
            if(item.isProjectItem) {
                it.remove();
            }
        }

        Integer numberAdded = 0;

        for(Project project : projects) {
            NavDrawerItem newProjectItem =
                    new NavDrawerItem(project.project_name, getProjectIconForMasterUrl(project.master_url), project.master_url);
            navDrawerItems.add(3, newProjectItem);
            numberAdded++;
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG, "NavDrawerListAdapter.compareAndAddProjects() added: " + numberAdded);
        }
        this.notifyDataSetChanged();
        return numberAdded;
    }

    public class NavDrawerItem {

        private int id = 0;
        private String title;
        private int icon;
        private boolean isCounterVisible = false;
        private boolean isSubItem = false;
        private boolean isProjectItem = false;
        private Bitmap projectIcon;
        private String projectMasterUrl;

        public NavDrawerItem() {
        }

        /**
         * Creates default item
         */
        public NavDrawerItem(int id, int icon) {
            this.id = id;
            this.title = context.getString(id);
            this.icon = icon;
        }

        /**
         * Creates sub item under previous element
         */
        public NavDrawerItem(int id, int icon, boolean isCounterVisible, boolean isSubItem) {
            this.id = id;
            this.title = context.getString(id);
            this.icon = icon;
            this.isSubItem = isSubItem;
            this.isCounterVisible = isCounterVisible;
        }

        /**
         * Creates item for project, which is sub item of Projects by default
         */
        public NavDrawerItem(String name, Bitmap icon, String masterUrl) {
            this.id = masterUrl.hashCode();
            this.title = name;
            this.projectIcon = icon;
            this.projectMasterUrl = masterUrl;
            this.isProjectItem = true;
            this.isSubItem = true;
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "NavDrawerItem: created hash code " + id + " for project " + name);
            }
        }

        /**
         * Creates item with number counter on right
         */
        public NavDrawerItem(int id, int icon, boolean isCounterVisible) {
            this.id = id;
            this.title = context.getString(id);
            this.icon = icon;
            this.isCounterVisible = isCounterVisible;
        }

        public int getId() {
            return id;
        }

        public String getTitle() {
            return this.title;
        }

        public int getIcon() {
            return this.icon;
        }

        public String getProjectMasterUrl() {
            return this.projectMasterUrl;
        }

        public boolean getCounterVisibility() {
            return this.isCounterVisible;
        }

        public boolean isSubItem() {
            return this.isSubItem;
        }

        public boolean isProjectItem() {
            return this.isProjectItem;
        }

        public Bitmap getProjectIcon() {
            return this.projectIcon;
        }

        public void updateProjectIcon() {
            this.projectIcon = getProjectIconForMasterUrl(projectMasterUrl);
        }

        public void setTitle(String title) {
            this.title = title;
        }

        public void setIcon(int icon) {
            this.icon = icon;
        }

        public void setCounterVisibility(boolean isCounterVisible) {
            this.isCounterVisible = isCounterVisible;
        }
    }
}
