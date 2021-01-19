package com.vcd.immersive.omafplayer.Rendering;

import android.content.Context;

import com.google.vr.sdk.base.Eye;
import java.nio.FloatBuffer;

/**
 * Utility class to generate & render spherical meshes for video or images. Use the static creation
 * methods to construct the Mesh's data. Then call the Mesh constructor on the GL thread when ready.
 * Use glDraw method to render it.
 */
public class Mesh {
    /** Standard media where a single camera frame takes up the entire media frame. */
    public static final int MEDIA_MONOSCOPIC = 0;
    /**
     * Stereo media where the left & right halves of the frame are rendered for the left & right eyes,
     * respectively. If the stereo media is rendered in a non-VR display, only the left half is used.
     */
    public static final int MEDIA_STEREO_LEFT_RIGHT = 1;
    /**
     * Stereo media where the top & bottom halves of the frame are rendered for the left & right eyes,
     * respectively. If the stereo media is rendered in a non-VR display, only the top half is used.
     */
    public static final int MEDIA_STEREO_TOP_BOTTOM = 2;

    // Vertices for the mesh with 3D position + left 2D texture UV + right 2D texture UV.
    public float[] vertices;
    public float[] transVertices;
    public FloatBuffer vertexBuffer;
    public FloatBuffer transBuffer;

    // Program related GL items. These are only valid if program != 0.
    public int program;
    public int mvpMatrixHandle;
    public int positionHandle;
    public int transpositionHandle;
    public int texCoordsHandle;
    public int textureHandle;
    public int textureId;

    public Context mContext;

    public static class MeshParams {
        // For ERP
        public float radius;
        public int latitudes;
        public int longitudes;
        public float vFOV;
        public float hFOV;
        public int mediaFormat;
        // For CMP
        // ...
    }
    /**
     * Generates a 3D UV sphere for rendering monoscopic or stereoscopic video.
     *
     * <p>This can be called on any thread. The returned {@link Mesh} isn't valid until
     * {@link #glInit(int)} is called.
     *
     * @param Mesh.MeshParams parameters for creating mesh.
     * @return Unintialized Mesh.
     */
    public static Mesh Create(MeshParams params) { return new Mesh(null); };

    public Mesh() {}
    /** Used by static constructors. */
    private Mesh(float[] vertexData) {
        vertices = vertexData;
        vertexBuffer = Utils.createBuffer(vertices);
    }

    /**
     * Finishes initialization of the GL components.
     *
     * @param textureId GL_TEXTURE_EXTERNAL_OES used for this mesh.
     */
    /* package */ void glInit(int textureId) {}

    /**
     * Renders the mesh. This must be called on the GL thread.
     *
     * @param mvpMatrix The Model View Projection matrix.
     * @param eyeType An {@link Eye.Type} value.
     */
    /* package */ void glDraw(float[] mvpMatrix, int eyeType, int[] transformType, int width, int height) {}

    /** Cleans up the GL resources. */
    /* package */ void glShutdown() {}
}
