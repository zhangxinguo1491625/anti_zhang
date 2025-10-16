package com.example.zhang_anti;

import static android.provider.Settings.Secure.ACCESSIBILITY_ENABLED;

import android.accessibilityservice.AccessibilityServiceInfo;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.ServiceInfo;
import android.os.Bundle;
import android.provider.Settings;
import android.view.accessibility.AccessibilityManager;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.orhanobut.logger.Logger;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.json.JSONObject;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;

import android.os.Build;


public class MainActivity extends AppCompatActivity {
    static String zhang_TAG = "zhang_tag";
    HashMap<String,String> smap = new HashMap<>();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        AntiNative antinative = new AntiNative();
        AntiNative.n0(getApplicationContext());
        smap.put("checkXposed",String.valueOf(AntiNative.n14()));
        smap.put("checkFrida",String.valueOf(AntiNative.n13()));
        smap.put("checkMagisk",String.valueOf(AntiNative.n12()));
        smap.put("checkEmu",String.valueOf(AntiNative.n10()));
        smap.put("checkBox",String.valueOf(AntiNative.n9()));
        smap.put("checkRoot",String.valueOf(AntiNative.n8()));
        smap.put("checkdeviceinfo",String.valueOf(AntiNative.n6()));


        TextView textView = findViewById(R.id.text_view);
        JSONObject jsonObject = new JSONObject(smap);
        textView.setText(jsonObject.toString());
    }
    private String getprop(String prop) throws ClassNotFoundException {
        ClassLoader.getSystemClassLoader().loadClass("");

    }
}