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

import org.eclipse.collections.api.list.MutableList;

import java.text.NumberFormat;
import java.util.List;

import edu.berkeley.boinc.BOINCActivity;
import edu.berkeley.boinc.R;
import edu.berkeley.boinc.rpc.Project;
import edu.berkeley.boinc.utils.ECLists;
import edu.berkeley.boinc.utils.Logging;

public class NavDrawerListAdapter extends BaseAdapter {
    private Context context;
    private MutableList<NavDrawerItem> navDrawerItems;

    public int selectedMenuId = 0;

    public NavDrawerListAdapter(Context context) {
        this.context = context;

        // populate items
        navDrawerItems = ECLists.mutable.of(
                new NavDrawerItem(this, R.string.tab_tasks, R.drawable.tabtaskb, true),
                new NavDrawerItem(this, R.string.tab_notices, R.drawable.mailb, true),
                new NavDrawerItem(this, R.string.tab_projects, R.drawable.projectsb),
                new NavDrawerItem(this, R.string.projects_add, R.drawable.sqplusb, false, true),
                new NavDrawerItem(this, R.string.tab_preferences, R.drawable.cogsb),
                new NavDrawerItem(this, R.string.menu_help, R.drawable.helpb),
                new NavDrawerItem(this, R.string.menu_report_issue, R.drawable.bugb),
                new NavDrawerItem(this, R.string.menu_about, R.drawable.infob),
                new NavDrawerItem(this, R.string.menu_eventlog, R.drawable.attentionb)
        );
    }

    public Context getContext() {
        return context;
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
        return navDrawerItems.get(position).getId();
    }

    public NavDrawerItem getItemForId(int id) {
        return navDrawerItems.detect(item -> item.getId() == id);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if(Logging.VERBOSE) {
            Log.d(Logging.TAG, "NavDrawerListAdapter.getView() for : " +
                               navDrawerItems.get(position).getTitle() +
                               navDrawerItems.get(position).getCounterVisibility() +
                               navDrawerItems.get(position).isSubItem() +
                               navDrawerItems.get(position).isProjectItem());
        }
        if(convertView == null ||
           !(convertView.getTag()).equals(navDrawerItems.get(position).getTitle())) {
            int layoutId = R.layout.navlist_listitem;
            if(navDrawerItems.get(position).isSubItem()) {
                layoutId = R.layout.navlist_listitem_subitem;
            }
            LayoutInflater mInflater =
                    (LayoutInflater) context.getSystemService(Activity.LAYOUT_INFLATER_SERVICE);
            convertView = mInflater.inflate(layoutId, null);
        }

        RelativeLayout wrapper = convertView.findViewById(R.id.listitem);
        ImageView imgIcon = convertView.findViewById(R.id.icon);
        TextView txtTitle = convertView.findViewById(R.id.title);
        TextView txtCount = convertView.findViewById(R.id.counter);

        if(navDrawerItems.get(position).isProjectItem()) {
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
            int counter = 0;
            switch(navDrawerItems.get(position).getId()) {
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

        // highlight entry of currently activated item
        if(navDrawerItems.get(position).getId() == selectedMenuId) {
            if(Logging.DEBUG) {
                Log.d(Logging.TAG, "NavDrawerListAdapter.getView() highlighted! ID : " +
                                   selectedMenuId);
            }
            wrapper.setBackgroundResource(R.drawable.navlist_selector_pressed);
        }
        else {
            wrapper.setBackgroundResource(R.drawable.navlist_selector);
        }

        convertView.setTag(navDrawerItems.get(position).getTitle());
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
    public int compareAndAddProjects(List<Project> projects) {
        // delete all old projects from nav items
        navDrawerItems.removeIf(NavDrawerItem::isProjectItem);

        int numberAdded = 0;
        for(Project project : projects) {
            NavDrawerItem newProjectItem =
                    new NavDrawerItem(this, project.getProjectName(),
                                      getProjectIconForMasterUrl(project.getMasterURL()),
                                      project.getMasterURL());
            navDrawerItems.add(3, newProjectItem);
            numberAdded++;
        }

        if(Logging.DEBUG) {
            Log.d(Logging.TAG,
                  "NavDrawerListAdapter.compareAndAddProjects() added: " + numberAdded);
        }
        this.notifyDataSetChanged();
        return numberAdded;
    }
}
