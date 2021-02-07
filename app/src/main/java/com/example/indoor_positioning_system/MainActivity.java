package com.example.indoor_positioning_system;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
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
import java.util.Arrays;
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

    // Request code for creating a text document.
    private static final int CREATE_FILE = 1;

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
        //createFile(Uri.parse(Environment.DIRECTORY_DOWNLOADS));
        initialise_scan("/storage/emulated/0/Download/IPS_test.txt");

    }

    private void createFile(Uri pickerInitialUri) {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("text/plain");
        intent.putExtra(Intent.EXTRA_TITLE, "test");

        // Optionally, specify a URI for the directory that should be opened in
        // the system file picker when your app creates the document.
        intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, pickerInitialUri);

        startActivityForResult(intent, CREATE_FILE);
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
        Uri uri = null;
        if(requestCode==CREATE_FILE && resultCode== Activity.RESULT_OK){
            if(result != null){
                uri = result.getData();
                Log.i("INFO",uri.toString());
                Log.i("Decoded Path",uri.getPath());
                //String path = getPath(getApplicationContext(),uri);
                //Path hardcoded for now. Would be changed later.
                //String path = "/storage/self/primary/Download/IPS/test0.txt";
                //String path = "/storage/emulated/0/Download/IPS/invoice.txt";
                //Log.i("getPath",path);
                initialise_scan("/storage/emulated/0/Download/IPS/test.txt");
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

    /**
     * Get a file path from a Uri. This will get the the path for Storage Access
     * Framework Documents, as well as the _data field for the MediaStore and
     * other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri The Uri to query.
     * @author paulburke
     */
    public static String getPath(final Context context, final Uri uri) {

        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;

        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }

                // TODO handle non-primary volumes
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {

                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(
                        Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));

                return getDataColumn(context, contentUri, null, null);
            }
            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                final String selection = "_id=?";
                final String[] selectionArgs = new String[] {
                        split[1]
                };

                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }
        // MediaStore (and general)
        else if ("content".equalsIgnoreCase(uri.getScheme())) {
            return getDataColumn(context, uri, null, null);
        }
        // File
        else if ("file".equalsIgnoreCase(uri.getScheme())) {
            return uri.getPath();
        }

        return null;
    }

    /**
     * Get the value of the data column for this Uri. This is useful for
     * MediaStore Uris, and other file-based ContentProviders.
     *
     * @param context The context.
     * @param uri The Uri to query.
     * @param selection (Optional) Filter used in the query.
     * @param selectionArgs (Optional) Selection arguments used in the query.
     * @return The value of the _data column, which is typically a file path.
     */
    public static String getDataColumn(Context context, Uri uri, String selection,
                                       String[] selectionArgs) {

        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {
                column
        };

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(column_index);
            }
        } finally {
            if (cursor != null)
                cursor.close();
        }
        return null;
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }
}