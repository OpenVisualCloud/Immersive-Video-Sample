package com.vcd.immersive.omafplayer;

import android.Manifest;
import android.Manifest.permission;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import com.google.vr.ndk.base.DaydreamApi;
import com.vcd.immersive.omafplayer.Rendering.Mesh;
import org.json.JSONException;
import org.json.JSONObject;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;

/**
 * Basic Activity to hold {@link MonoscopicView} and render a 360 video in 2D.
 *
 * Most of this Activity's code is related to Android & VR permission handling. The real work is in
 * MonoscopicView.
 *
 * The default intent for this Activity will load a 360 placeholder panorama. For more options on
 * how to load other media using a custom Intent, see {@link MediaLoader}.
 */
public class VideoActivity extends Activity {
    private static final String TAG = "VideoActivity";
    private static final int READ_EXTERNAL_STORAGE_PERMISSION_ID = 1;
    private static final int WRITE_EXTERNAL_STORAGE_PERMISSION_ID = 2;
    private MonoscopicView videoView;
    private TextView urlText;
    private TextView sourceTypeText;
    private TextView extractorText;
    private TextView cachePathText;
    private TextView maxWidthText;
    private TextView maxHeightText;
    private TextView hasPredictText;
    private TextView predictNameText;
    private TextView predictPathText;
    private TextView hasCatchUpText;
    private TextView catchupTimesText;
    private TextView catchupMaxWidthText;
    private TextView catchupMaxHeightText;
    private boolean isInputConfigFinished = false;
    private boolean isPermissionRequired = false;

    /**
     * Checks that the appropriate permissions have been granted. Otherwise, the sample will wait
     * for the user to grant the permission.
     *
     * @param savedInstanceState unused in this sample but it could be used to track video position
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getApplicationContext().getExternalFilesDir("");

        setContentView(R.layout.video_activity);

        // Configure the MonoscopicView which will render the video and UI.
        videoView = (MonoscopicView) findViewById(R.id.video_view);
        VideoUiView videoUi = (VideoUiView) findViewById(R.id.video_ui_view);
        videoUi.setVrIconClickListener(
                new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        // Convert the Intent used to launch the 2D Activity into one that can launch the VR
                        // Activity. This flow preserves the extras and data in the Intent.
                        DaydreamApi api =  DaydreamApi.create(VideoActivity.this);
                        if (api != null){
                            // Launch the VR Activity with the proper intent.
                            Intent intent = DaydreamApi.createVrIntent(
                                    new ComponentName(VideoActivity.this, VrVideoActivity.class));
                            intent.setData(getIntent().getData());
                            intent.putExtra(
                                    MediaLoader.MEDIA_FORMAT_KEY,
                                    getIntent().getIntExtra(MediaLoader.MEDIA_FORMAT_KEY, Mesh.MEDIA_MONOSCOPIC));
                            api.launchInVr(intent);
                            api.close();
                        } else {
                            // Fall back for devices that don't have Google VR Services. This flow should only
                            // be used for older Cardboard devices.
                            Intent intent =
                                    new Intent(getIntent()).setClass(VideoActivity.this, VrVideoActivity.class);
                            intent.removeCategory(Intent.CATEGORY_LAUNCHER);
                            intent.setFlags(0);  // Clear any flags from the previous intent.
                            startActivity(intent);
                        }

                        // See VrVideoActivity's launch2dActivity() for more info about why this finish() call
                        // may be required.
                        finish();
                    }
                });
        videoView.initialize(videoUi);

        // Boilerplate for checking runtime permissions in Android.
        if (ContextCompat.checkSelfPermission(this, permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            View button = findViewById(R.id.permission_button);
            button.setOnClickListener(
                    new OnClickListener() {
                        @Override
                        public void onClick(View v) {
                            ActivityCompat.requestPermissions(
                                    VideoActivity.this,
                                    new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE},
                                    WRITE_EXTERNAL_STORAGE_PERMISSION_ID);
                        }
                    });
            // The user can click the button to request permission but we will also click on their behalf
            // when the Activity is created.
            button.callOnClick();
        } else {
            // Permission has already been granted.
            isPermissionRequired = true;
            if (isInputConfigFinished) {
                initializeActivity();
            }
        }
        QueryInputParams();
    }

    /** Handles the user accepting the permission. */
    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        if (requestCode == READ_EXTERNAL_STORAGE_PERMISSION_ID) {
            if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) {
                isPermissionRequired = true;
                if (isInputConfigFinished) {
                    initializeActivity();
                }
            }
        }
        if (requestCode == WRITE_EXTERNAL_STORAGE_PERMISSION_ID) {
            if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) {
                isPermissionRequired = true;
                if (isInputConfigFinished) {
                    initializeActivity();
                }
            }
        }
    }

    /**
     * Normal apps don't need this. However, since we use adb to interact with this sample, we
     * want any new adb Intents to be routed to the existing Activity rather than launching a new
     * Activity.
     */
    @Override
    protected void onNewIntent(Intent intent) {
        // Save the new Intent which may contain a new Uri. Then tear down & recreate this Activity to
        // load that Uri.
        setIntent(intent);
        recreate();
    }

    /** Initializes the Activity only if the permission has been granted and input parameters configured. */
    private void initializeActivity() {
        ViewGroup root = (ViewGroup) findViewById(R.id.activity_root);
        for (int i = 0; i < root.getChildCount(); ++i) {
            root.getChildAt(i).setVisibility(View.VISIBLE);
        }
        findViewById(R.id.permission_button).setVisibility(View.GONE);
        findViewById(R.id.topTitle).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout1).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout2).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout3).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout4).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout5).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout6).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout7).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout8).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout9).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout10).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout11).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout12).setVisibility(View.GONE);
        findViewById(R.id.relativeLayout13).setVisibility(View.GONE);
        findViewById(R.id.loginButton).setVisibility(View.GONE);
        videoView.loadMedia(getIntent());
    }

    private void QueryInputParams() {
        urlText        = findViewById(R.id.text_url);
        sourceTypeText = findViewById(R.id.text_sourceType);
        extractorText  = findViewById(R.id.text_extractor);
        cachePathText  = findViewById(R.id.text_cachePath);
        maxWidthText   = findViewById(R.id.text_maxWidth);
        maxHeightText  = findViewById(R.id.text_maxHeight);
        hasPredictText = findViewById(R.id.text_hasPredict);
        predictNameText= findViewById(R.id.text_predictName);
        predictPathText= findViewById(R.id.text_predictPath);
        hasCatchUpText = findViewById(R.id.text_hasCatchup);
        catchupTimesText     = findViewById(R.id.text_catchupTimes);
        catchupMaxWidthText  = findViewById(R.id.text_maxCatchupWidth);
        catchupMaxHeightText = findViewById(R.id.text_maxCatchupHeight);

        Button btn     = findViewById(R.id.loginButton);
        SetDefaultValueFromCfgFile();
        btn.setOnClickListener(new InputOnClickListener());
    }

    private void SetDaultValues() {
        // default value for the first time
        urlText.setText("http://x.x.x.x:x/xxx/Test.mpd");
        sourceTypeText.setText("0");
        extractorText.setText("false");
        cachePathText.setText("sdcard/Android/data/tmp/");
        maxWidthText.setText("4096");
        maxHeightText.setText("3200");
        hasPredictText.setText("false");
        predictNameText.setText("");
        predictPathText.setText("");
        hasCatchUpText.setText("false");
        catchupTimesText.setText("1");
        catchupMaxWidthText.setText("3200");
        catchupMaxHeightText.setText("3200");
    }

    //read from cfg file on create
    private void SetDefaultValueFromCfgFile() {

        StringBuilder stringBuilder = new StringBuilder();

        try {
            //1. load cfg file
            String path = getApplicationContext().getExternalFilesDir("").getPath();
            File inputfile = new File(path + "/cfg.json");
            if (!inputfile.exists()) {
                Log.i(TAG, "First time to run the activity! Use default values!");
                SetDaultValues();
                return;
            }

            FileInputStream inputStream = new FileInputStream(inputfile);
            InputStreamReader isr = new InputStreamReader(inputStream, "UTF-8");
            BufferedReader bufferedReader = new BufferedReader(isr);

            String jsonLines;
            while ((jsonLines = bufferedReader.readLine()) != null) {
                stringBuilder.append(jsonLines);
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        //2. set text according to json file
        String cfgJsonStr = stringBuilder.toString();
        JSONObject cfgJsonObject = null;
        try {
            cfgJsonObject = new JSONObject(cfgJsonStr);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        try {
            urlText.setText(cfgJsonObject.getString("url"));
            sourceTypeText.setText(cfgJsonObject.getInt("sourceType") + "");
            extractorText.setText(cfgJsonObject.getBoolean("enableExtractor") + "");
            cachePathText.setText(cfgJsonObject.getString("cachePath"));
            maxWidthText.setText(cfgJsonObject.getInt("maxVideoDecodeWidth") + "");
            maxHeightText.setText(cfgJsonObject.getInt("maxVideoDecodeHeight") + "");
            hasPredictText.setText(cfgJsonObject.getBoolean("hasPredict") + "");
            predictNameText.setText(cfgJsonObject.getString("predictName"));
            predictPathText.setText(cfgJsonObject.getString("predictPath"));
            hasCatchUpText.setText(cfgJsonObject.getBoolean("hasCatchup") + "");
            catchupTimesText.setText(cfgJsonObject.getInt("catchupTimes") + "");
            catchupMaxWidthText.setText(cfgJsonObject.getInt("maxCatchupWidth") + "");
            catchupMaxHeightText.setText(cfgJsonObject.getInt("maxCatchupHeight") + "");
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    // write to cfg file when on click
    private void SetCfgInFileFromInputParams() {
        try {
            //1. open/create cfg file
            String path = getApplicationContext().getExternalFilesDir("").getPath();
            File inputfile = new File(path + "/cfg.json");
            if (!inputfile.exists()) {
                Log.i(TAG, "The first time to use activity! create a cfg file to store history settings!");
                inputfile.createNewFile();
            }
            FileOutputStream outputStream = new FileOutputStream(inputfile);
            OutputStreamWriter osw = new OutputStreamWriter(outputStream, "UTF-8");
            //2. fill input params to file
            JSONObject inputObj = new JSONObject();

            inputObj.put("url", urlText.getText().toString());
            inputObj.put("sourceType", sourceTypeText.getText().toString());
            inputObj.put("enableExtractor", extractorText.getText().toString());
            inputObj.put("cachePath", cachePathText.getText().toString());
            inputObj.put("maxVideoDecodeWidth", maxWidthText.getText().toString());
            inputObj.put("maxVideoDecodeHeight", maxHeightText.getText().toString());
            inputObj.put("hasPredict", hasPredictText.getText().toString());
            inputObj.put("predictName", predictNameText.getText().toString());
            inputObj.put("predictPath", predictPathText.getText().toString());
            inputObj.put("hasCatchup", hasCatchUpText.getText().toString());
            inputObj.put("catchupTimes", catchupTimesText.getText().toString());
            inputObj.put("maxCatchupWidth", catchupMaxWidthText.getText().toString());
            inputObj.put("maxCatchupHeight", catchupMaxHeightText.getText().toString());

            // fill in default values at the first time
            if (!inputObj.has("windowWidth")) inputObj.put("windowWidth", 960);
            if (!inputObj.has("windowHeight")) inputObj.put("windowHeight", 960);
            if (!inputObj.has("viewportHFOV")) inputObj.put("viewportHFOV", 80);
            if (!inputObj.has("viewportVFOV")) inputObj.put("viewportVFOV", 80);
            if (!inputObj.has("viewportWidth")) inputObj.put("viewportWidth", 960);
            if (!inputObj.has("viewportHeight")) inputObj.put("viewportHeight", 960);

            System.out.println("input " + inputObj.toString());
            osw.write(inputObj.toString());
            osw.flush();
            osw.close();

        } catch (IOException e) {
            e.printStackTrace();
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        videoView.onResume();
    }

    @Override
    protected void onPause() {
        // MonoscopicView is a GLSurfaceView so it needs to pause & resume rendering. It's also
        // important to pause MonoscopicView's sensors & the video player.
        videoView.onPause();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        videoView.destroy();
        super.onDestroy();
    }

    private class InputOnClickListener implements OnClickListener {
        @Override
        public void onClick(View v) {
            SetCfgInFileFromInputParams();
            isInputConfigFinished = true;
            if (isPermissionRequired) {
                initializeActivity();
            }
        }
    }
}

