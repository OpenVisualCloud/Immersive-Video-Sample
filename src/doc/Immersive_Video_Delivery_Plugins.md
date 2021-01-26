# Immersive Video Delivery Plugins

## Introduction
Immersive Video Delivery libraries provide plugin mechanism for customized process.
Now there are totally four types plugins supported:
- ViewportPredict_Plugin : plugin for predicting user viewport in next segment to reduce the latency of motion to high quality in OmafDashAccess library.
- OMAFPacking_Plugin : plugin for region-wise packing information generation for extractor track used in VROmafPacking library.
- StreamProcess_Plugin : plugin for media stream process, including both video stream and audio strem, also used in VROmafPacking library.
- 360SCVP_Plugin/TileSelection_Plugins: plugins with multiple tile selection methods for 2D/3D videos under different using scenarios in 360SCVP library.

## ViewportPredict_Plugin
The main function of ViewportPredict_Plugin is to predict viewport angles with linear regression model using trajectory feedback in real time.
The plugin provides C APIs for user.
- `ViewportPredict_Init`: It is the initialization function with input parameter `PredictOption`. It is called only once in initialization process.
- `ViewportPredict_SetViewport`: It need to be called to set viewport each frame. The structure of viewport information is `ViewportAngle`, including (yaw, pitch, roll, pts, priority);
- `ViewportPredict_PredictPose`: It is called before timely downloading segment to obtain the predicted pose. Linear regression model is applied in the plugin, and an adaptive correction based on real-time feedback of the viewing trajectory is adopted to further improve the accuracy of prediction.
- `ViewportPredict_unInit`: It is the uninitialization function to be called in the end of the process.

## OMAFPacking_Plugin
The main function of OMAFPacking_Plugin is to generate HEVC tiles layout for each specific extractor track and then generate the region-wise packing information for tiles stitched sub-picture.
- Firstly, the API Initialize should be called with all input video streams information defined by structure 'VideoStreamInfo', video streams index, selected tiles number in the extractor trak, the maximum selected tiles number in all extractor tracks and the external log callback.
- Then, the API GenerateMergedTilesArrange is to generate HEVC tiles layout which is defined by structure 'TileArrangement' in sub-picture for extractor track.
- After tiles arrangement is generated, API GenerateDstRwpk is called to construct region-wise packing information for tiles stitched sub-picture.
- Next, the API GenerateTilesMergeDirection is called to generate further detailed tiles merging layout defined by structure 'TilesMergeDirectionInCol' which will include below information for each selected tile : original video stream index, original tile index according to the order of raster scannig, and destination position in tiles merged sub-picture. These information will be used to calculate each extractor in extractor track.
- In addition, in order to construct new video stream from selected tiles stitching, APIs GetPackedPicWidth, GetPackedPicHeight, and GetMergedTilesArrange also need to be called to generate new SPS/PPS/Slice header.

## StreamProcess_Plugin
The main function of StreamProcess_Plugin is to get media stream information by parsing stream headers, and handle frames to be written into DASH segments.
The base class 'MediaStream' is for both video stream process plugin and audio stream process plugin and it will specify the basic attributes 'MediaType' and 'CodecId'.

VideoStream_Plugin
This plugin is used to parse video stream headers (VPS/SPS/PPS/SEI) and then get the detailed video stream informaiton, like resolution, tile split and so on. In addition, it will manage input video frames for DASH segmentation process.
- Firstly, the API Initialize will be called with the video stream index, basic video stream encoding configuration and video bitstream headers data defined by structure 'BSBuffer', and the initial information for VROmafPacking library. After getting video bitstream headers data, this plugin will begin parsing VPS/SPS/PPS/SEI. Then, most of video information can be get, like resolution, tile split, projection type and so on, by APIs GetSrcWidth, GetSrcHeight, GetTileInRow, GetTileInCol and GetProjType. In addition, the region-wise packing information for input video stream should also be generated during initialization by parsing SEI or other calculation methods.
- Secondly, when one frame from one video stream comes, API AddFrameInfo should be called to copy this frame information defined by structure 'FrameBSInfo' and then add new created frame information into frames list managed inside this plugin prepared for DASH segmentation.
- Then, in DASH segmentation process which may be in another thread, APIs SetCurrFrameInfo is called firstly to set the front frame in frames list as the current frame to be processed, then API GetCurrFrameInfo is used to get it in segmentation process thread. After getting new frame, call UpdateTilesNalu to update tile NALU data defined by structures 'Nalu' and 'TileInfo'. Lower library for segmentation will generate one segment when all frames included in the segment are ready, so API AddFrameToSegment should be called to hold the frame before the segment is generated. And after the segment is written, API DestroyCurrSegmentFrames will be called to release all frames data in this segment.
- At last, when all video frames have been written into segments, call API SetEOS to set EOS status, and DASH segmentation process will get this statue by API GetEOS, then to stop the process.

AudioStream_Plugin
This plugin is used to parse auido stream headers, like ADTS header for AAC audio stream, and then get the detailed audio stream information, like sample rate, channel number and so on. In addition, it will also manage input audio frames for DASH segmentation process.
- Firstly, the API Initialize will be called with the audio stream index, basic audio stream encoding configuration and the first audio frame bitstream data defined by structure 'BSBuffer', and the initial information for VROmafPacking library. After getting the first auido frame data, whether header data is included will be checked. If there is header data, like ADTS header, then parse header to get basic audio stream information, like sample rate, if not, the basic audio information must be set in input structure 'BSBuffer'. Then, APIs GetSampleRate, GetChannelNum and so on can be called to get these information.
- Secondly, when one frame from one audio stream comes, API AddFrameInfo should be called to copy this frame information defined by structure 'FrameBSInfo' and then add new created frame information into frames list managed inside the plugin prepared for DASH segmentation.
- Then, in DASH segmentation process which may be in another thread, APIs SetCurrFrameInfo is called firstly to set the front frame in frames list as the current frame to be processed, then API GetCurrFrameInfo is used to get it in segmentation process thread. After getting new frame, call GetHeaderDataSize to update and get header size for the frame, then raw audio data in this frame can be obtained. Like video stream process plugin, lower library for segmentation will generate one segment when all frames included in the segment are ready, so API AddFrameToSegment should be called to hold the frame before the segment is generated. And after the segment has been written, API DestroyCurrSegmentFrames will be called to release all frames data.
- At last, when all audio frames have been written into segments, call API SetEOS to set EOS status, and DASH segmentation process will get this status by API GetEOS, then to stop the process.

## 360SCVP_Plugins
The 360SCVP plugins will provide mulitiple functions on different types of videos. Though we support tile selection for multiple now, it is able to extend to other functions.
There is a structure for plugin definition which is defined in 360SCVPAPI.h. The PluginDef has three element for user to define their plugin type, format and library path.

TileSelection_Plugins
This is one kind of 360SCVP Plugins which can support 2D Planar video but also could be easy extended to 3D projection videos like ERP or cubemap. The TileSelectionPlugins_API.h has defined a base class 'TileSelection' and users can define their own child-class in the implementation. Below is an example process for Tile Selection on planar videos.
- Firstly, The base class has a constructor, a deconstructor and an initilaization function. When users want to execute tile selection, the initialization function should be called with proper configuration in the param_360SCVP struct. The param 'sourceResolutionNum' is the number of the high resolution streams. The struct 'pStreamInfo' is used to set the resolution and tile width and height of each stream. Do not forget to set the 'PluginDef' with the plugin type, format and library path.
- Then the API 'SetViewportInfo' is called to pass down the headpose information, which includes the viewport center point coordinates, the tile selection mode, and the moving direction and speed.
- Now the API 'GetTilesInViewport' can be called to get tile selection results based on the initial configurations and headpose information. The output struct array element stores the selected tile information including the upleft point coordinate, the streamId which indicates the resolution this tile is.
- When the playback finished, the unInit API will be called to release internal dynamic memory and reset internal variables.
