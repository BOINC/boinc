package edu.berkeley.boinc.rpc;

import android.graphics.Bitmap;
import android.os.Parcel;
import android.os.Parcelable;

public class ImageWrapper implements Parcelable {
    public Bitmap image;
    public String projectName;
    public String path;

    public ImageWrapper(Bitmap image, String projectName, String path) {
        this.image = image;
        this.projectName = projectName;
        this.path = path;
    }

    @Override
    public int describeContents() {
        // TODO Auto-generated method stub
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int arg1) {
        dest.writeValue(image);
        dest.writeString(projectName);
        dest.writeString(path);
    }

    public static final Parcelable.Creator<ImageWrapper> CREATOR = new Parcelable.Creator<ImageWrapper>() {
        public ImageWrapper createFromParcel(Parcel in) {
            return new ImageWrapper((Bitmap) in.readValue(Bitmap.class.getClassLoader()), in.readString(), in.readString());
        }

        public ImageWrapper[] newArray(int size) {
            return new ImageWrapper[size];
        }
    };

}