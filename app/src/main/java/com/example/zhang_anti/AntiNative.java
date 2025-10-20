package com.example.zhang_anti;

import android.content.Context;

public class AntiNative {
    static String zhang_TAG = "zhang_tag";
    // checkXposed
    public static native boolean n14();
    // checkFrida
    public static native boolean n13();
    // checkMagisk
    public static native boolean n12();
    // checkEmu
    public static native boolean n10();
    // checkBox
    public static native boolean n9();
    // checkRoot
    public static native boolean n8();
    // getDeviceInfo
    public static native String n6();
    // checkDebugger
    public static native boolean n5();
    // checkUnicorm
    public static native boolean n3();

    public static native boolean n0(Context context);

    static {
        System.loadLibrary("anti");
    }
}
