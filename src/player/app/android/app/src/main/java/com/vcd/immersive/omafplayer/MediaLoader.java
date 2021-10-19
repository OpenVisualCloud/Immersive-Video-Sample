package com.vcd.immersive.omafplayer;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.os.AsyncTask;
import android.support.annotation.AnyThread;
import android.support.annotation.MainThread;
import android.util.Log;
import android.util.Pair;
import android.view.Surface;

import com.vcd.immersive.omafplayer.MediaPlayer.NativeMediaPlayer;
import com.vcd.immersive.omafplayer.Rendering.CubeMapMesh;
import com.vcd.immersive.omafplayer.Rendering.ERPMesh;
import com.vcd.immersive.omafplayer.Rendering.Mesh;
import com.vcd.immersive.omafplayer.Rendering.SceneRenderer;
import com.vcd.immersive.omafplayer.Rendering.Utils;

import static com.vcd.immersive.omafplayer.Rendering.Utils.checkGlError;

/**
 * MediaLoader takes an Intent from the user and loads the specified media file.
 *
 * <p>The process to load media requires multiple threads since the media is read from disk on a
 * background thread, but it needs to be loaded into the GL scene only after GL initialization is
 * complete.
 *
 * <p>To keep the sample simple, this class doesn't have any support for handling multiple Intents
 * within a single Activity lifecycle.
 *
 * <p>The Intent used to launch {@link VideoActivity} or {@link VrVideoActivity} is parsed by this
 * class and the extra & data fields are extracted. The data field should have a URI useable by
 * {@link MediaPlayer} or {@link BitmapFactory}. There should also be an integer extra matching one
 * of the MEDIA_* types in {@link Mesh}.
 *
 * <p>Example intents compatible with adb are:
 *   <ul>
 *     <li>
 *       A top-bottom stereo image in the VR Activity.
 *       <b>adb shell am start -a android.intent.action.VIEW  \
 *          -n com.google.vr.sdk.samples.video360/.VrVideoActivity \
 *          -d "file:///sdcard/IMAGE.JPG" \
 *          --ei stereoFormat 2
 *       </b>
 *     </li>
 *     <li>
 *       A monoscopic video in the 2D Activity.
 *       <b>adb shell am start -a android.intent.action.VIEW  \
 *          -n com.google.vr.sdk.samples.video360/.VideoActivity \
 *          -d "file:///sdcard/VIDEO.MP4" \
 *          --ei stereoFormat 0
 *       </b>
 *     </li>
 *   </ul>
 *
 * <p>This sample does not validiate that a given file is readable by the Android media decoders.
 * You should validate that the file plays on your target devices via
 * <b>adb shell am start -a android.intent.action.VIEW -t video/mpeg -d "file:///VIDEO.MP4"</b>
 */
public class MediaLoader {
    private static final String TAG = "MediaLoader";

    public static final String MEDIA_FORMAT_KEY = "stereoFormat";
    private static final int MAX_SURFACE_NUM = 5;
    private static final int MAX_CATCHUP_SURFACE_NUM = 1;

    /** A spherical mesh for video should be large enough that there are no stereo artifacts. */
    private static final int SPHERE_RADIUS_METERS = 50;

    /** These should be configured based on the video type. But this sample assumes 360 video. */
    private static final int DEFAULT_SPHERE_VERTICAL_DEGREES = 180;
    private static final int DEFAULT_SPHERE_HORIZONTAL_DEGREES = 360;

    /** The 360 x 180 sphere has 15 degree quads. Increase these if lines in your video look wavy. */
    private static final int DEFAULT_SPHERE_ROWS = 12;
    private static final int DEFAULT_SPHERE_COLUMNS = 24;

    private static final int PF_ERP = 0;
    private static final int PF_CUBEMAP = 1;

    private final Context context;
    // This can be replaced by any media player that renders to a Surface. In a real app, this
    // media player would be separated from the rendering code. It is left in this class for
    // simplicity.
    // This should be set or cleared in a synchronized manner.
    NativeMediaPlayer mediaPlayer;

    // Due to the slow loading media times, it's possible to tear down the app before mediaPlayer is
    // ready. In that case, abandon all the pending work.
    // This should be set or cleared in a synchronized manner.
    private boolean isDestroyed = false;

    // The type of mesh created depends on the type of media.
    Mesh mesh;
    // The sceneRenderer is set after GL initialization is complete.
    private SceneRenderer sceneRenderer;
    // The displaySurface is configured after both GL initialization and media loading.
    private Surface[] decodeSurface = new Surface[MAX_SURFACE_NUM + MAX_CATCHUP_SURFACE_NUM];
    private Pair<Integer, Surface> decoder_surface = null;
    private Pair<Integer, Surface> decoder_surface_cu = null;
    private Pair<Integer, Surface> display_surface = null;
    private Surface displaySurface;

    // The actual work of loading media happens on a background thread.
    private MediaLoaderTask mediaLoaderTask;

    public MediaLoader(Context context) {
        this.context = context;
    }

    /**
     * Loads custom videos based on the Intent or load the default video. See the Javadoc for this
     * class for information on generating a custom intent via adb.
     */
    public void handleIntent(Intent intent, VideoUiView uiView) {
        // Load the bitmap in a background thread to avoid blocking the UI thread. This operation can
        // take 100s of milliseconds.
        // Note that this sample doesn't cancel any pending mediaLoaderTasks since it assumes only one
        // Intent will ever be fired for a single Activity lifecycle.
        mediaLoaderTask = new MediaLoaderTask(uiView);
        mediaLoaderTask.execute(intent);
    }

    /** Notifies MediaLoader that GL components have initialized. */
    public void onGlSceneReady(SceneRenderer sceneRenderer) {
        this.sceneRenderer = sceneRenderer;
        Log.i(TAG, "Scene ready!");
        if (mediaPlayer != null)
            this.sceneRenderer.setMediaPlayer(mediaPlayer);
        displayWhenReady();//real operate
    }

    /**
     * Helper class to media loading. This accesses the disk and decodes images so it needs to run in
     * the background.
     */
    private class MediaLoaderTask extends AsyncTask<Intent, Void, Void> {
        private final VideoUiView uiView;

        public MediaLoaderTask(VideoUiView uiView) {
            this.uiView = uiView;
        }

        @Override
        protected Void doInBackground(Intent... intent) {

            mediaPlayer = new NativeMediaPlayer(context);
            Log.i(TAG, "Create native media player!");
            int ret = mediaPlayer.Initialize();
            if (ret != 0)
            {
                Log.e(TAG, "native media player init failed!");
                return null;
            }

            displayWhenReady();
            return null;
        }

        @Override
        public void onPostExecute(Void unused) {
            // Set or clear the UI's mediaPlayer on the UI thread.
            if (uiView != null) {
                uiView.setMediaPlayer(mediaPlayer);
            }
            if (sceneRenderer != null)
                sceneRenderer.setMediaPlayer(mediaPlayer);
        }
    }

    /**
     * Creates the 3D scene and load the media after sceneRenderer & mediaPlayer are ready. This can
     * run on the GL Thread or a background thread.
     */
    @AnyThread
    private synchronized void displayWhenReady() {
        if (isDestroyed) {
            // This only happens when the Activity is destroyed immediately after creation.
            if (mediaPlayer != null) {
                mediaPlayer.Close();
                mediaPlayer = null;
            }
            return;
        }

        if (displaySurface != null || decodeSurface[0] != null) {
            // Avoid double initialization caused by sceneRenderer & mediaPlayer being initialized before
            // displayWhenReady is executed.
            return;
        }
        if (mediaPlayer == null) Log.i(TAG, "media player is null");
        if (sceneRenderer == null) Log.i(TAG, "scene renderer is null");
        if (mediaPlayer == null || sceneRenderer == null) {
            // Wait for everything to be initialized.
            Log.i(TAG, "wait to init!");
            return;
        }
        // The important methods here are the setSurface & lockCanvas calls. These will have to happen
        // after the GLView is created.
        if (mediaPlayer.mConfig != null && sceneRenderer.decode_surface_ready) {
            synchronized (this) {
                // 1. create decode surfaces and set them to native player.
                for (int i = 0; i < MAX_SURFACE_NUM; i++) {
                    decoder_surface = sceneRenderer.createDecodeSurface(
                            mediaPlayer.mConfig.maxVideoDecodeWidth, mediaPlayer.mConfig.maxVideoDecodeHeight, i);
                    Log.i(TAG, "Complete to create one decode surface! surface id is " + i);
                    mediaPlayer.SetDecodeSurface(decoder_surface.second, decoder_surface.first, i);//set surface
                    Log.i(TAG, "ready to set decode surface!");
                    decodeSurface[i] = decoder_surface.second;
                    Log.i(TAG, "decode id in java " + decoder_surface.first);
                }
                for (int i = MAX_SURFACE_NUM; i < MAX_CATCHUP_SURFACE_NUM + MAX_SURFACE_NUM; i++) {
                    decoder_surface_cu = sceneRenderer.createDecodeSurface(
                            mediaPlayer.mConfig.maxCatchupWidth, mediaPlayer.mConfig.maxCatchupHeight, i);
                    Log.i(TAG, "Complete to create one catch-up decode surface! surface id is " + i);
                    mediaPlayer.SetDecodeSurface(decoder_surface_cu.second, decoder_surface_cu.first, i);//set surface
                    Log.i(TAG, "ready to set decode surface!");
                    decodeSurface[i] = decoder_surface_cu.second;
                    Log.i(TAG, "decode id in java " + decoder_surface_cu.first);
                }
                // 2. create native player and get display width and height and projection format
                int ret = mediaPlayer.Create("./config.xml");
                if (ret != 0) {
                    Log.e(TAG, "native media player create failed!");
                    return;
                }
                // 3. create mesh according to PF
                int stereoFormat = Mesh.MEDIA_MONOSCOPIC;
                Mesh.MeshParams params = new Mesh.MeshParams();
                int projFormat = mediaPlayer.GetProjectionFormat();
                Log.i(TAG, "pf is " + projFormat);
                if (projFormat == PF_CUBEMAP) {
                    mesh = CubeMapMesh.Create(params, context);
                    Log.i(TAG, "Create cubemap mesh!");
                } else {
                    params.radius = SPHERE_RADIUS_METERS;
                    params.latitudes = DEFAULT_SPHERE_ROWS;
                    params.longitudes = DEFAULT_SPHERE_COLUMNS;
                    params.vFOV = DEFAULT_SPHERE_VERTICAL_DEGREES;
                    params.hFOV = DEFAULT_SPHERE_HORIZONTAL_DEGREES;
                    params.mediaFormat = stereoFormat;
                    mesh = ERPMesh.Create(params);
                    Log.i(TAG, "Create ERP mesh!");
                    if (projFormat != PF_ERP) {
                        Log.e(TAG, "Projection format is invalid! Default is ERP format!");
                    }
                }
                // 4. get width / height and create display surface and set it to native player
                int displayWidth = mediaPlayer.GetWidth();
                int displayHeight = mediaPlayer.GetHeight();
                if (projFormat == PF_ERP) {
                    sceneRenderer.displayTexId = Utils.glCreateTextureFor2D(mediaPlayer.GetWidth(), mediaPlayer.GetHeight());
                    Log.i(TAG, "ERP Display texture id is " + sceneRenderer.displayTexId);
                } else if (projFormat == PF_CUBEMAP) {
                    sceneRenderer.displayTexId = Utils.glCreateTextureForCube(mediaPlayer.GetWidth(), mediaPlayer.GetHeight());
                    Log.i(TAG, "Cubemap Display texture id is " + sceneRenderer.displayTexId);
                } else {
                    sceneRenderer.displayTexId = 0;
                    Log.e(TAG, "Projection format is invalid! displayer texture id is set to zero!");
                }
                sceneRenderer.displayTexture = new SurfaceTexture(sceneRenderer.displayTexId);
                checkGlError();
                Log.i(TAG, "display width is " + displayWidth + " display height is " + displayHeight);
                display_surface = sceneRenderer.createDisplaySurface(
                        displayWidth, displayHeight, mesh);
                Log.i(TAG, "ready to create display surface");
                mediaPlayer.SetDisplaySurface(display_surface.first);

                displaySurface = display_surface.second;
                // 4. start native player thread
                Log.i(TAG, "start to start!");
                mediaPlayer.Start();
            }
        }else
        {
            Log.e(TAG, "media player is invalid!");
        }
    }

    /**
     * Renders a placeholder grid with optional error text.
     */
    private static void renderEquirectangularGrid(Canvas canvas, String message) {
        // Configure the grid. Each square will be 15 x 15 degrees.
        final int width = canvas.getWidth();
        final int height = canvas.getHeight();
        // This assumes a 4k resolution.
        final int majorWidth = width / 256;
        final int minorWidth = width / 1024;
        final Paint paint = new Paint();

        // Draw a black ground & gray sky background
        paint.setColor(Color.BLACK);
        canvas.drawRect(0, height / 2, width, height, paint);
        paint.setColor(Color.GRAY);
        canvas.drawRect(0, 0, width, height / 2, paint);

        // Render the grid lines.
        paint.setColor(Color.WHITE);

        for (int i = 0; i < DEFAULT_SPHERE_COLUMNS; ++i) {
            int x = width * i / DEFAULT_SPHERE_COLUMNS;
            paint.setStrokeWidth((i % 3 == 0) ? majorWidth : minorWidth);
            canvas.drawLine(x, 0, x, height, paint);
        }

        for (int i = 0; i < DEFAULT_SPHERE_ROWS; ++i) {
            int y = height * i / DEFAULT_SPHERE_ROWS;
            paint.setStrokeWidth((i % 3 == 0) ? majorWidth : minorWidth);
            canvas.drawLine(0, y, width, y, paint);
        }

        // Render optional text.
        if (message != null) {
            paint.setTextSize(height / 64);
            paint.setColor(Color.RED);
            float textWidth = paint.measureText(message);

            canvas.drawText(
                    message,
                    width / 2 - textWidth / 2, // Horizontally center the text.
                    9 * height / 16, // Place it slightly below the horizon for better contrast.
                    paint);
        }
    }

    @MainThread
    public synchronized void pause() {
        if (mediaPlayer != null) {
            mediaPlayer.Pause();
        }
    }

    @MainThread
    public synchronized void resume() {
        if (mediaPlayer != null) {
            mediaPlayer.Resume();
        }
    }

    /** Tears down MediaLoader and prevents further work from happening. */
    @MainThread
    public synchronized void destroy() {
        if (mediaPlayer != null) {
            mediaPlayer.Stop();
        }
        isDestroyed = true;
    }
}
