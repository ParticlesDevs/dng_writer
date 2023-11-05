package com.freedcam.dng_writer;

public class NativeLib {

    // Used to load the 'dng_writer' library on application startup.
    static {
        System.loadLibrary("dng_writer");
    }

    /**
     * A native method that is implemented by the 'dng_writer' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}