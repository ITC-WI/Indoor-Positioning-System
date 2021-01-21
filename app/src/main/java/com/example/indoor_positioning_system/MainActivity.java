package com.example.indoor_positioning_system;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.kontakt.sdk.android.common.KontaktSDK;

public class MainActivity extends AppCompatActivity {
    TextView cppv;
    Button scanButton;
    boolean buttonState; //0 means no scanning in progress. 1 means scanning in progress.
    private static final String API_KEY = "zWsIXUklUXaQZfxVnQsraHqWllLvPETs";//grp0 API key

    private static final int REQUEST_PERMISSIONS = 100; //This is in context with the request code
                                                        //asked during onRequestPermissionResult().
                                                        //Choice of 100 is arbitrary but should not be
                                                        //changed without rationale.

    public boolean permission_status;                   //To keep track of permission status

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //First initialise the SDK with the API key.
        KontaktSDK.initialize(API_KEY);

        //Initially, no scanning in progress
        buttonState = false;

        //Assume, no permissions were granted every time the app is started:
        permission_status = false;

        // Example of a call to a native method
        cppv = findViewById(R.id.cpptextview);
        cppv.setText(stringFromJNI());

        //Setup the scan button
        scanButton = findViewById(R.id.scanButton);
        setupButtons();
    }

    /**
     * If scanning in progress then quit. Otherwise,
     * First check whether the required permissions have been granted.
     * If the required permissions are granted, start a new thread and carry on scanning there.
     */
    public void setupButtons(){
        //Lambda is used here because the listener is anonymous
        scanButton.setOnClickListener(v -> {
            if(!buttonState){
                if(!permission_status){
                    checkPermissions();
                }
                else{
                    start_scanning();
                }
            }
            else{
                //stop the scanning thread and set buttonState to false.
                stop_scanning();
            }
        });
    }

    /**
     * Called at each button press if permission_status is false.
     * Do not call it from anywhere else!!
     * This function checks whether the required permissions have been granted.
     * If the permissions are not granted, ask for them.
     * If the permissions are granted then return true otherwise false.
     */
    public void checkPermissions(){
        //For testing only:
        Log.i("INFO","Checking for permissions");
        int checkSelfPermissionResult = checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION);
        if(PackageManager.PERMISSION_GRANTED==checkSelfPermissionResult){
            permission_status =true;
            //For testing only:
            Log.i("INFO","Permission already granted");
        }
        else{
            //For testing only:
            Log.i("INFO","Requesting for permissions");
            //The following will result in a callback to onRequestPermissionResult(). The results can be
            //handled there.
            requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION},REQUEST_PERMISSIONS);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if(grantResults.length>0 && grantResults[0]==PackageManager.PERMISSION_GRANTED){
            if(requestCode==REQUEST_PERMISSIONS){
                //For testing purposes only:
                Log.i("INFO","Permissions Granted");
                permission_status = true;
                start_scanning();
            }
        }

        else{
            //TODO: User can be shown more rationale here.
            Toast.makeText(this, "Critical Permission Denied",Toast.LENGTH_LONG).show();
        }
    }

    /**
     *
     */
    public void start_scanning(){
        scanButton.setText(R.string.stop_scanning);
        //For testing only:
        Log.i("INFO","Starting Scanning");
        //TODO: start the thread.
        buttonState = true;
    }

    /**
     *
     */
    public void stop_scanning(){
        scanButton.setText(R.string.start_scanning);
        //For testing only:
        Log.i("INFO","Stopping Scan");
        //TODO: end the thread first.
        buttonState = false;
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}