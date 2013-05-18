/*******************************************************************************
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
 ******************************************************************************/
package edu.berkeley.boinc.adapter;

import java.util.ArrayList;

import edu.berkeley.boinc.client.ClientStatus.ImageWrapper;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.widget.BaseAdapter;
import android.widget.Gallery;
import android.widget.ImageView;

public class GalleryAdapter extends BaseAdapter{

	int mGalleryItemBackground;
    private Context ctx;

    private ArrayList<ImageWrapper> images = new ArrayList<ImageWrapper>();

    public GalleryAdapter(Context ctx, ArrayList<ImageWrapper> images) {
        this.ctx = ctx;
        this.images = images;
    }

    public int getCount() {
        return images.size();
    }

    public ImageWrapper getItem(int position) {
        return images.get(position);
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ImageView imageView = new ImageView(ctx);

        LayoutParams params = new Gallery.LayoutParams(290, 126);
        imageView.setLayoutParams(params);
        imageView.setImageBitmap(images.get(position).image);

        return imageView;
    }
}
