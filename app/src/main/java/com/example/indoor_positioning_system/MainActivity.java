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

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    TextView cppv;
    Button scanButton;
    ProgressBar progressBar;
    boolean buttonState; //0 means no scanning in progress. 1 means scanning in progress.
    private static final String API_KEY = "zWsIXUklUXaQZfxVnQsraHqWllLvPETs";//grp0 API key

    private static final int REQUEST_PERMISSIONS = 100; //This is in context with the request code
                                                        //asked during onRequestPermissionResult().
                                                        //Choice of 100 is arbitrary but should not be
                                                        //changed without rationale.

    private static final int REQUEST_ENABLE_BT = 101;  //Similar to REQUEST_PERMISSIONS but for BLE.

    public boolean permission_status;                   //To keep track of permission status

    private ProximityManager proximityManager;

    public static final String FILE_NAME = "Example.txt";

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

        //Initialise the ProgressBar:
        progressBar = findViewById(R.id.progress_circular);

        // Example of a call to a native method
        cppv = findViewById(R.id.cpptextview);
        cppv.setText(stringFromJNI());

        //Initialise and configure the proximity Manager
        setupProximityManager();

        //Setup the scan button
        scanButton = findViewById(R.id.scanButton);
        setupButtons();

        //create a file to log the data into
        create_file();
        //This would handle initialising the filters in native code.
        initialise_scan();

    }

    public void create_file(){
        FileOutputStream fos = null;
        try {
            fos = openFileOutput(FILE_NAME, MODE_PRIVATE);
            fos.write('a');
            Toast.makeText(this, "Saved to " + getFilesDir() + "/" + FILE_NAME,
                    Toast.LENGTH_LONG).show();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fos != null) {
                try {
                    fos.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
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
            permission_status =true;
            //For testing only:
            Log.i("INFO","Permission already granted");

            start_scanning();
        }
        else{
            //For testing only:
            Log.i("INFO","Requesting for permissions");
            //The following will result in a callback to onRequestPermissionResult(). The results can be
            //handled there.
            requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION},REQUEST_PERMISSIONS);
        }

        //Now check whether location is enabled.

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
                //OnDeviceUpdate callback will be received with 1 second interval
                .deviceUpdateCallbackInterval(1000);

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
                }
                IBeaconsUpdated(majors, minors, rssis);
            }

            @Override
            public void onIBeaconLost(IBeaconDevice iBeacon, IBeaconRegion region) {
                Log.i("IBeaconLost",iBeacon.toString());
                IBeaconLost(iBeacon.getMajor(),iBeacon.getMinor(),iBeacon.getRssi());
            }
        };
    }

    /**
     *
     */
    public boolean bluetoothEnabled(){
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if(bluetoothAdapter.isEnabled()){
            return true;
        }
        else{
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            //!Possible alternative: If startActivityForResult() is not completed by the time we check for BLE again.
            //The application would almost never reach here.
            if(bluetoothAdapter.isEnabled()){
                return true;
            }
            else{
                return false;
            }
        }
    }

    //The following method is required for testing only as of now.
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent result) {
        super.onActivityResult(requestCode, resultCode, result);
        if (requestCode == REQUEST_ENABLE_BT) {  // Match the request code
            if (resultCode == RESULT_OK) {

            } else {   // RESULT_CANCELED
                Log.i("INFO","Bluetooth request denied");
            }
        }
    }

    /**
     * Start scanning the Beacons.
     */
    public void start_scanning(){
        //TODO: Implement scanning in a new thread. stop_scanning() would change accordingly.

        //check if Bluetooth is on.
        if(!bluetoothEnabled()){
            //User hasn't enabled bluetooth even after asking.
            Toast.makeText(MainActivity.this,"Unable to scan for beacons",Toast.LENGTH_LONG).show();
            return;
        }

        scanButton.setText(R.string.stop_scanning);
        //For testing only:
        Log.i("INFO","Starting Scanning");

        buttonState = true;
        //Connect to scanning service and start scanning when ready.
        //Anonymous listener.
        proximityManager.connect(() -> {
            //Check if proximity manager is already scanning. This check is superfluous.
//            if (proximityManager.isScanning()) {
//                Toast.makeText(MainActivity.this, "Already scanning", Toast.LENGTH_SHORT).show();
//                return;
//            }
            proximityManager.startScanning();
            Toast.makeText(MainActivity.this, "Scanning started", Toast.LENGTH_SHORT).show();
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

        buttonState = false;
    }

    @Override
    protected void onStop() {
        if(buttonState) stop_scanning();
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    /**
     * To initialise the filters and anything that needs to be run before actually starting filtering.
     */
    public native void initialise_scan();

    /**
     * To repeatedly estimate position of the user. This would handle all the filtering calls.
     */
    public native void start_filtering();

    /**
     * To safely exit from filtering.
     */
    public native void stop_filtering();

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
    public native void IBeaconLost(int major, int minor, float rssi);
}