package com.vcd.immersive.omafplayer.Rendering;

import static android.support.constraint.Constraints.TAG;

import static com.vcd.immersive.omafplayer.Rendering.Utils.checkGlError;
import static java.lang.Math.cos;
import static java.lang.Math.sin;

import android.content.Context;

import android.opengl.GLES20;

import android.util.Log;

import com.google.vr.sdk.base.Eye;


/**
 * Utility class to generate & render spherical meshes for video or images. Use the static creation
 * methods to construct the Mesh's data. Then call the Mesh constructor on the GL thread when ready.
 * Use glDraw method to render it.
 */
public final class CubeMapMesh extends Mesh {

    private class TransformType {
        public static final int NO_TRANSFORM = 0;
        public static final int MIRRORING_HORIZONTALLY = 1;
        public static final int ROTATION_180_ANTICLOCKWISE = 2;
        public static final int ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR = 3;
        public static final int ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR= 4;
        public static final int ROTATION_90_ANTICLOCKWISE = 5;
        public static final int ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR = 6;
        public static final int ROTATION_270_ANTICLOCKWISE = 7;
    }

    private class FaceID {
        public static final int CUBE_MAP_RIGHT = 0;
        public static final int CUBE_MAP_LEFT = 1;
        public static final int CUBE_MAP_TOP = 2;
        public static final int CUBE_MAP_BOTTOM = 3;
        public static final int CUBE_MAP_BACK = 4;
        public static final int CUBE_MAP_FRONT = 5;
    }

    private static final int VIEWPORT_HFOV = 1920;

    private static final int VIEWPORT_VFOV = 1920;

    // Basic vertex & fragment shaders to render a mesh with 3D position & 2D texture data.
    private static final String[] VERTEX_SHADER_CODE =
            new String[] {
                    "uniform mat4 uMvpMatrix;",
                    "attribute vec3 aPosition;",
                    "attribute vec3 transPosition;",
                    "varying vec3 vTexCoords;",

                    // Standard transformation.
                    "void main() {",
                    "  vTexCoords = transPosition;",
                    "  gl_Position = uMvpMatrix * vec4(aPosition, 1.0);",
                    "  gl_Position = gl_Position.xyww;",
                    "}"
            };
    private static final String[] FRAGMENT_SHADER_CODE =
            new String[] {
                    // This is required since the texture data is GL_TEXTURE_EXTERNAL_OES.
                    // "#extension GL_OES_EGL_image_external : require",
                    "precision mediump float;",

                    // Standard texture rendering shader.
                    // "uniform samplerExternalOES uTexture;",
                    "uniform samplerCube uTexture;",
                    "varying vec3 vTexCoords;",
                    "void main() {",
                    "  gl_FragColor = textureCube(uTexture, vTexCoords);",
                    "}"
            };
    private boolean isChangeTransformType = false;
    // Constants related to vertex data.
    private static final int VERTEX_NUM_FOR_SKYBOX = 36;

    private static final int POSITION_COORDS_PER_VERTEX = 3; // X, Y, Z.
    // The vertex contains texture coordinates for both the left & right eyes. If the scene is
    // rendered in VR, the appropriate part of the vertex will be selected at runtime. For a mono
    // scene, only the left eye's UV coordinates are used.
    // For mono media, the UV coordinates are duplicated in each. For stereo media, the UV coords
    // point to the appropriate part of the source media.
    /**
     * Generates a 3D UV sphere for rendering monoscopic or stereoscopic video.
     *
     * <p>This can be called on any thread. The returned {@link Mesh} isn't valid until
     * {@link #glInit(int)} is called.
     *
     * @param radius Size of the sphere. Must be > 0.
     * @param latitudes Number of rows that make up the sphere. Must be >= 1.
     * @param longitudes Number of columns that make up the sphere. Must be >= 1.
     * @param verticalFovDegrees Total latitudinal degrees that are covered by the sphere. Must be in
     *    (0, 180].
     * @param horizontalFovDegrees Total longitudinal degrees that are covered by the sphere.Must be
     *    in (0, 360].
     * @param mediaFormat A MEDIA_* value.
     * @return Unintialized Mesh.
     */
    public static CubeMapMesh Create(
            MeshParams params, Context context) {
        float[] vertexData = new float[] {
                // vertex postion         transform position
                // right- z (for flip)
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                // left- z (for flip)
                -1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                // top- z (for flip)
                -1.0f,  1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                -1.0f,  1.0f, -1.0f,
                // bottom- z (for flip)
                -1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                -1.0f, -1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                // back- x (for flip)
                -1.0f, -1.0f,  1.0f,
                -1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f,  1.0f,  1.0f,
                1.0f, -1.0f,  1.0f,
                -1.0f, -1.0f,  1.0f,
                // front- x (for flip)
                -1.0f,  1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f,  1.0f, -1.0f,
                -1.0f,  1.0f, -1.0f,
        };
        float[] transData = new float[] {
                // vertex postion         transform position
                // right- z (for flip)
                     1.0f, -1.0f,  1.0f,
                     1.0f, -1.0f, -1.0f,
                      1.0f,  1.0f, -1.0f,
                      1.0f,  1.0f, -1.0f,
                      1.0f,  1.0f,  1.0f,
                     1.0f, -1.0f,  1.0f,
                // left- z (for flip)
                   -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f,  1.0f,
                    -1.0f,  1.0f,  1.0f,
                     -1.0f,  1.0f,  1.0f,
                   -1.0f,  1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                // top- z (for flip)
                     -1.0f,  1.0f,  1.0f,
                     1.0f,  1.0f,  1.0f,
                    1.0f,  1.0f, -1.0f,
                     1.0f,  1.0f, -1.0f,
                   -1.0f,  1.0f, -1.0f,
                  -1.0f,  1.0f,  1.0f,
                // bottom- z (for flip)
                   -1.0f, -1.0f,  1.0f,
                  -1.0f, -1.0f, -1.0f,
                     1.0f, -1.0f,  1.0f,
                     1.0f, -1.0f,  1.0f,
                     -1.0f, -1.0f, -1.0f,
                     1.0f, -1.0f, -1.0f,
                // back- x (for flip)
                   1.0f, -1.0f,  1.0f,
                    1.0f,  1.0f,  1.0f,
                   -1.0f,  1.0f,  1.0f,
                  -1.0f,  1.0f,  1.0f,
                    -1.0f, -1.0f,  1.0f,
                     1.0f, -1.0f,  1.0f,
                // front- x (for flip)
                     1.0f,  1.0f, -1.0f,
                   1.0f, -1.0f, -1.0f,
                 -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, -1.0f,
                     -1.0f,  1.0f, -1.0f,
                      1.0f,  1.0f, -1.0f,
        };
        return new CubeMapMesh(vertexData, transData, context);
    }

    /** Used by static constructors. */
    private CubeMapMesh(float[] vertexData, float[] transData, Context context) {
        vertices = vertexData;
        transVertices = transData;
        vertexBuffer = Utils.createBuffer(vertices);
        transBuffer = Utils.createBuffer(transData);
        mContext = context;
    }

    /**
     * Finishes initialization of the GL components.
     *
     * @param textureId GL_TEXTURE_EXTERNAL_OES used for this mesh.
     */
    /* package */ void glInit(int textureId) {
        this.textureId = textureId;

        program = Utils.compileProgram(VERTEX_SHADER_CODE, FRAGMENT_SHADER_CODE);

        mvpMatrixHandle = GLES20.glGetUniformLocation(program, "uMvpMatrix");
        positionHandle = GLES20.glGetAttribLocation(program, "aPosition");
        transpositionHandle = GLES20.glGetAttribLocation(program, "transPosition");
        textureHandle = GLES20.glGetUniformLocation(program, "uTexture");

        checkGlError();
        Log.e(TAG, "Cubemap Mesh " + positionHandle + textureHandle);
    }

    /**
     * Renders the mesh. This must be called on the GL thread.
     *
     * @param mvpMatrix The Model View Projection matrix.
     * @param eyeType An {@link Eye.Type} value.
     */
    /* package */ void glDraw(float[] mvpMatrix, int eyeType, int[] transformType, int width, int height) {
        // Configure shader.
        GLES20.glUseProgram(program);
        checkGlError();
        GLES20.glViewport(0,0, width, height);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_CUBE_MAP, textureId);

        GLES20.glUniformMatrix4fv(mvpMatrixHandle, 1, false, mvpMatrix, 0);
        GLES20.glUniform1i(textureHandle, 0);
        if (!isChangeTransformType && transformType != null)
        {
            ChangeTransformVertices(transformType);
            isChangeTransformType = true;
        }
        checkGlError();
        GLES20.glEnableVertexAttribArray(positionHandle);
        GLES20.glVertexAttribPointer(positionHandle, POSITION_COORDS_PER_VERTEX,
                GLES20.GL_FLOAT, false, 0, vertexBuffer);
        GLES20.glEnableVertexAttribArray(transpositionHandle);
        GLES20.glVertexAttribPointer(transpositionHandle, POSITION_COORDS_PER_VERTEX,
                GLES20.GL_FLOAT, false, 0,  transBuffer);
        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, VERTEX_NUM_FOR_SKYBOX);
        GLES20.glUseProgram(0);
        Log.e(TAG, "Cubemap Mesh draw");
    }

    /** Cleans up the GL resources. */
    /* package */ void glShutdown() {
        if (program != 0) {
            GLES20.glDeleteProgram(program);
            GLES20.glDeleteTextures(1, new int[]{textureId}, 0);
        }
    }

    void ChangeTransformVertices(int[] transformtype)
    {
        boolean needChanged = false;
        for (int i = 0; i < transformtype.length; i++)
        {
            if (transformtype[i] != TransformType.NO_TRANSFORM)
            {
                needChanged = true;
                break;
            }
        }
        if (!needChanged)
        {
            return;
        }

        int face_id = FaceID.CUBE_MAP_RIGHT;
        for (; face_id <= FaceID.CUBE_MAP_FRONT; face_id++)
        {
            if (transformtype[face_id] != TransformType.NO_TRANSFORM)
            {
                for (int j = face_id * 18; j < (face_id + 1) * 18; j++)
                {
                    if (j % 3 == 0)
                    {
                        if (transformtype[face_id] == TransformType.MIRRORING_HORIZONTALLY)
                        {
                            if (face_id == FaceID.CUBE_MAP_RIGHT || face_id == FaceID.CUBE_MAP_LEFT)
                                transVertices[j + 2] = -transVertices[j + 2];
                            else if (face_id == FaceID.CUBE_MAP_TOP || face_id == FaceID.CUBE_MAP_BOTTOM)
                                transVertices[j] = -transVertices[j];
                            else if (face_id == FaceID.CUBE_MAP_BACK || face_id == FaceID.CUBE_MAP_FRONT)
                                transVertices[j] = -transVertices[j];
                        }
                        else
                        {
                            if (transformtype[face_id] == TransformType.ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR)
                            {
                                if (face_id == FaceID.CUBE_MAP_RIGHT || face_id == FaceID.CUBE_MAP_LEFT)
                                    transVertices[j + 2] = -transVertices[j + 2];
                                else if (face_id == FaceID.CUBE_MAP_TOP || face_id == FaceID.CUBE_MAP_BOTTOM)
                                    transVertices[j] = -transVertices[j];
                                else if (face_id == FaceID.CUBE_MAP_BACK || face_id == FaceID.CUBE_MAP_FRONT)
                                    transVertices[j] = -transVertices[j];
                            }
                            float transDegree = 0;
                            if (face_id == FaceID.CUBE_MAP_RIGHT || face_id == FaceID.CUBE_MAP_BOTTOM || face_id == FaceID.CUBE_MAP_BACK)
                            {
                                if (transformtype[face_id] == TransformType.ROTATION_180_ANTICLOCKWISE || transformtype[face_id] == TransformType.ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR)
                                    transDegree = (float) Math.PI;
                                else if (transformtype[face_id] == TransformType.ROTATION_90_ANTICLOCKWISE || transformtype[face_id] == TransformType.ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = (float) Math.PI / 2;
                                else if (transformtype[face_id] == TransformType.ROTATION_270_ANTICLOCKWISE || transformtype[face_id] == TransformType.ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = (float) Math.PI / 2 * 3;
                            }
                            else
                            {
                                if (transformtype[face_id] == TransformType.ROTATION_180_ANTICLOCKWISE || transformtype[face_id] == TransformType.ROTATION_180_ANTICLOCKWISE_AFTER_MIRRORING_HOR)
                                    transDegree = (float) Math.PI;
                                else if (transformtype[face_id] == TransformType.ROTATION_90_ANTICLOCKWISE || transformtype[face_id] == TransformType.ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = (float) Math.PI / 2 * 3;
                                else if (transformtype[face_id] == TransformType.ROTATION_270_ANTICLOCKWISE || transformtype[face_id] == TransformType.ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                                    transDegree = (float) Math.PI / 2;
                            }

                            // different face id
                            if (face_id == FaceID.CUBE_MAP_RIGHT) // NY
                            {
                                float y = transVertices[j + 1];
                                float z = transVertices[j + 2];
                                transVertices[j + 1] = (float) (y * cos(transDegree) - z * sin(transDegree));
                                transVertices[j + 2] = (float) (y * sin(transDegree) + z * cos(transDegree));
                            }
                            else if (face_id == FaceID.CUBE_MAP_LEFT) // PY
                            {
                                float y = transVertices[j + 1];
                                float z = transVertices[j + 2];
                                transVertices[j + 1] = (float) (y * cos(transDegree) - z * sin(transDegree));
                                transVertices[j + 2] = (float) (y * sin(transDegree) + z * cos(transDegree));
                            }
                            else if (face_id == FaceID.CUBE_MAP_TOP) // PZ
                            {
                                float x = transVertices[j];
                                float z = transVertices[j + 2];
                                transVertices[j] = (float) (x * cos(transDegree) - z * sin(transDegree));
                                transVertices[j + 2] = (float) (x * sin(transDegree) + z * cos(transDegree));
                            }
                            else if (face_id == FaceID.CUBE_MAP_BOTTOM) // NZ
                            {
                                float x = transVertices[j];
                                float z = transVertices[j + 2];
                                transVertices[j] = (float) (x * cos(transDegree) - z * sin(transDegree));
                                transVertices[j + 2] = (float) (x * sin(transDegree) + z * cos(transDegree));
                            }
                            else if (face_id == FaceID.CUBE_MAP_BACK) // NX
                            {
                                float x = transVertices[j];
                                float y = transVertices[j + 1];
                                transVertices[j] = (float) (x * cos(transDegree) - y * sin(transDegree));
                                transVertices[j + 1] = (float) (x * sin(transDegree) + y * cos(transDegree));
                            }
                            else if (face_id == FaceID.CUBE_MAP_FRONT) // PX
                            {
                                float x = transVertices[j];
                                float y = transVertices[j + 1];
                                transVertices[j] = (float) (x * cos(transDegree) - y * sin(transDegree));
                                transVertices[j + 1] = (float) (x * sin(transDegree) + y * cos(transDegree));
                            }
                            // first anti-clockwise and then hor-mirror
                            if (transformtype[face_id] == TransformType.ROTATION_90_ANTICLOCKWISE_BEFORE_MIRRORING_HOR || transformtype[face_id] == TransformType.ROTATION_270_ANTICLOCKWISE_BEFORE_MIRRORING_HOR)
                            {
                                if (face_id == FaceID.CUBE_MAP_RIGHT || face_id == FaceID.CUBE_MAP_LEFT)
                                    transVertices[j + 2] = -transVertices[j + 2];
                                else if (face_id == FaceID.CUBE_MAP_TOP || face_id == FaceID.CUBE_MAP_BOTTOM)
                                    transVertices[j] = -transVertices[j];
                                else if (face_id == FaceID.CUBE_MAP_BACK || face_id == FaceID.CUBE_MAP_FRONT)
                                    transVertices[j] = -transVertices[j];
                            }
                        }
                    }
                }
            }
        }
        transBuffer.put(transVertices);
        transBuffer.position(0);
    }
}
