package com.example.indoor_positioning_system;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.kontakt.sdk.android.ble.configuration.ScanMode;
import com.kontakt.sdk.android.ble.configuration.ScanPeriod;
import com.kontakt.sdk.android.ble.manager.ProximityManager;
import com.kontakt.sdk.android.ble.manager.ProximityManagerFactory;
import com.kontakt.sdk.android.ble.manager.listeners.IBeaconListener;
import com.kontakt.sdk.android.common.KontaktSDK;
import com.kontakt.sdk.android.common.profile.IBeaconDevice;
import com.kontakt.sdk.android.common.profile.IBeaconRegion;

import java.util.Arrays;
import java.util.List;

public class TrilaterationActivity extends AppCompatActivity {


    /**
     * Used to Start and Stop the positioning algorithm.
     */
    Button scanButton;

    /**
     * A Loading icon. Active when positioning is in progress so that
     * the user knows that positioning is going on.
     */
    ProgressBar progressBar;

    /**
     * To keep track whether positioning is active or not. 1 means active and 0 means not.
     */
    boolean isPositioningActive;

    /**
     * kontact.io panel grp0 API key. Used to initialise the SDK.
     * Also this app is so tailored such that only the beacons registered at the kontact.io panel
     * with this particular key are scanned.
     */
    private static final String API_KEY = "zWsIXUklUXaQZfxVnQsraHqWllLvPETs";

    /**
     * Bluetooth enable request code. Used in onActivityResult() callback. The value 100 is
     * arbitrary but should not be changed without rationale.
     */
    private static final int REQUEST_ENABLE_BT = 100;

    /**
     * Code for Location and Storage permission request callback. Used with onRequestPermissionResult().
     * The value 101 is arbitrary but should not be changed without some rationale.
     */
    private static final int REQUEST_PERMISSIONS = 101;


    /**
     * This is a handle for managing the beacons via kontact.io SDK.
     */
    private ProximityManager proximityManager;

    /**
     * Log file. Created and Stored in "Downloads/". If the file is already present then it is appended.
     */
    public static final String FILE_NAME = "IPS_test.txt";

    // Used to load the 'native-lib' library on application startup. All other C++ libraries are
    // imported via native-lib.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_trilateration);

        //First initialise the SDK with the API key.
        KontaktSDK.initialize(API_KEY);

        //Initially, no scanning in progress
        isPositioningActive = false;


        //Initialise the ProgressBar aka loading icon view.
        progressBar = findViewById(R.id.progress_circular);

        //Ask for permissions.
        checkPermissions();

        //Initialise and configure the proximity Manager
        setupProximityManager();

        //Setup the scan button
        scanButton = findViewById(R.id.scan_button);
        setupButtons();

        initialise_scan("/storage/emulated/0/Download/IPS_test.txt");
    }

    /**
     * If scanning in progress then quit. Otherwise,
     * First check whether the required permissions have been granted.
     * If the required permissions are granted, start a new thread and carry on scanning there.
     */
    public void setupButtons(){
        //Lambda is used here because the listener is anonymous
        scanButton.setOnClickListener(v -> {
            if(!isPositioningActive){
                    checkServices();
            }
            else{
                //stop the scanning thread and set buttonState to false.
                stop_scanning();
            }
        });
    }

    public void checkServices(){
        if(BluetoothAdapter.getDefaultAdapter().isEnabled()){
            start_scanning();
        }
        else{
            enableBluetooth();
        }
    }

    public void enableBluetooth(){
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if(!bluetoothAdapter.isEnabled()){
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
    }

    //The following method is required for testing only as of now.
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent result) {
        super.onActivityResult(requestCode, resultCode, result);
        if (requestCode == REQUEST_ENABLE_BT) {  // Match the request code
            if (resultCode == RESULT_OK) {
                start_scanning();
            } else {   // RESULT_CANCELED
                disableButton("Please turn on ");
            }
        }
    }


    /**
     * Called at each button press to check whether the required services(Location and BLE)
     * are running and also the permissions(FINE_LOCATION) are accepted.
     * Do not call it from anywhere else!!
     * This function checks whether the required permissions have been granted.
     * If the permissions are not granted, ask for them.
     * If the permissions are granted then return true otherwise false.
     */
    public void checkPermissions(){

        //For testing only:
        Log.i("INFO","Checking for permissions");

        //Check for FINE_LOCATION Permission.
        int checkSelfPermissionResult = checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION);
        if(PackageManager.PERMISSION_GRANTED==checkSelfPermissionResult){
            //For testing only:
            Log.i("INFO","FINE_LOCATION Permission already granted");
        }
        else{
            //For testing only:
            Log.i("INFO","Requesting for permissions");
            //The following will result in a callback to onRequestPermissionResult(). The results can be
            //handled there.
            requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION,Manifest.permission.WRITE_EXTERNAL_STORAGE},REQUEST_PERMISSIONS);
        }

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {


        if(grantResults.length>0 && grantResults[0]==PackageManager.PERMISSION_GRANTED && grantResults[1]==PackageManager.PERMISSION_GRANTED){
            if(requestCode==REQUEST_PERMISSIONS){
                //For testing purposes only:
                Log.i("INFO","LOCATION Permission Granted");
            }
        }

        else{
            Toast.makeText(this, "Critical Permission Denied",Toast.LENGTH_LONG).show();
            disableButton("The button has been disabled. Please allow Location and Storage Permission");
        }
    }

    private void disableButton(String s) {
        scanButton.setEnabled(false);
        TextView error_text_view = findViewById(R.id.error_text_view);
        error_text_view.setText(s);
    }

    /**
     * This function initialises and configures the IBeacon Listener.
     */
    public void setupProximityManager(){
        proximityManager = ProximityManagerFactory.create(this);

        //Configure proximity manager basic options
        proximityManager.configuration()
                //Using ranging for continuous scanning or MONITORING for scanning with intervals
                .scanPeriod(ScanPeriod.RANGING)
                //Using BALANCED for best performance/battery ratio
                .scanMode(ScanMode.BALANCED)
                //OnDeviceUpdate callback will be received with 10 second interval
                .deviceUpdateCallbackInterval(10000);

        //Setting up iBeacon listener
        proximityManager.setIBeaconListener(createIBeaconListener());
    }

    private IBeaconListener createIBeaconListener() {
        return new IBeaconListener() {
            @Override
            public void onIBeaconDiscovered(IBeaconDevice iBeacon, IBeaconRegion region) {
                Log.i("IBeaconDiscovered",iBeacon.toString());
                IBeaconDiscovered(iBeacon.getMajor(),iBeacon.getMinor(),iBeacon.getRssi());
            }

            @Override
            public void onIBeaconsUpdated(List<IBeaconDevice> iBeacons, IBeaconRegion region) {
                Log.i("IBeaconUpdated", String.valueOf(iBeacons.size()));
                int i=0;
                int[] majors = new int[iBeacons.size()];
                int[] minors = new int[iBeacons.size()];
                float[] rssis = new float[iBeacons.size()];
                for (IBeaconDevice iBeacon :iBeacons) {
                    majors[i] = iBeacon.getMajor();
                    minors[i] = iBeacon.getMinor();
                    rssis[i] = iBeacon.getRssi();
                    i++;
                }
                Log.i("Devices Updated", Arrays.toString(majors));
                IBeaconsUpdated(majors, minors, rssis);
            }

            @Override
            public void onIBeaconLost(IBeaconDevice iBeacon, IBeaconRegion region) {
                Log.i("IBeaconLost",iBeacon.toString());
                IBeaconLost(iBeacon.getMajor(),iBeacon.getMinor());
            }
        };
    }


    /**
     * Start scanning the Beacons.
     */
    public void start_scanning(){
        //TODO: Implement scanning in a new thread. stop_scanning() would change accordingly.

        scanButton.setText(R.string.stop_scanning);
        //For testing only:
        Log.i("INFO","Starting Scanning");

        isPositioningActive = true;
        //Connect to scanning service and start scanning when ready.
        //Anonymous listener.
        proximityManager.connect(() -> {
            //Check if proximity manager is already scanning. This check is superfluous.
//            if (proximityManager.isScanning()) {
//                Toast.makeText(MainActivity.this, "Already scanning", Toast.LENGTH_SHORT).show();
//                return;
//            }
            proximityManager.startScanning();
            Toast.makeText(TrilaterationActivity.this, "Scanning started", Toast.LENGTH_SHORT).show();
            progressBar.setVisibility(View.VISIBLE);
        });
        //This should be called periodically at regular intervals.
        //Java handler would handle the call at periodic intervals.
        //TODO: add the thread handler here.
        start_filtering();
    }

    /**
     * Stop scanning the beacons.
     */
    public void stop_scanning(){
        scanButton.setText(R.string.start_scanning);
        //For testing only:
        Log.i("INFO","Stopping Scan");

        progressBar.setVisibility(View.GONE);
        //TODO: Handled via a thread handler.
        stop_filtering();

        //Stop scanning if scanning is in progress. This check is superfluous.
        if (proximityManager.isScanning()) {
            proximityManager.stopScanning();
            Toast.makeText(this, "Scanning stopped", Toast.LENGTH_SHORT).show();
        }
        proximityManager.disconnect();

        isPositioningActive = false;
    }

    @Override
    protected void onStop() {
        if(isPositioningActive) stop_scanning();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        exit();
    }


    /**
     * To initialise the filters and anything that needs to be run before actually starting filtering.
     */
    public native void initialise_scan(String path);

    /**
     * To repeatedly estimate position of the user. This would handle all the filtering calls.
     */
    public native void start_filtering();

    /**
     * To safely exit from filtering.
     */
    public native void stop_filtering();

    /**
     * To exit the program.
     */
    public native void exit();

    /**
     * Called on new IBeacon discovery.
     */
    public native void IBeaconDiscovered(int major, int minor, float rssi);

    /**
     * Called on IBeacon update.
     */
    public native void IBeaconsUpdated(int[] majors, int[] minors, float[] rssis);

    /**
     * Called when an IBeacon is lost.
     */
    public native void IBeaconLost(int major, int minor);
}
