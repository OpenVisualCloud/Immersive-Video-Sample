/*
 * Copyright (c) 2019, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 *
 */

//!
//! \file     FFmpegMediaSource.cpp
//! \brief    Implement class for FFmpegMediaSource.
//!
#ifdef USE_DMA_BUFFER
#include "FFmpegMediaSource.h"
// hard code 3x3 + 6x3
// low reso config
#define SOURCENUMBER 2
#define REGIONNUMBER 27
#define FULLWIDTH 7680
#define FULLHEIGHT 3840
#define PACKEDWIDTH 5760
#define PACKEDHEIGHT 3840
#define LOWWIDTH 3840
#define LOWHEIGHT 1920

#include <va/va.h>
#include <va/va_drmcommon.h>

#include <time.h>
clock_t start, stop;
double duration;

VABufferInfo buffer_info;
VAImage va_image;

VCD_NS_BEGIN

FFmpegMediaSource::FFmpegMediaSource()
{
    InitializeFFmpegSourceData();
}

FFmpegMediaSource::~FFmpegMediaSource()
{
}
static enum AVPixelFormat hw_pix_fmt;
static AVBufferRef *hw_device_ctx = NULL;

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }

    LOG(ERROR)<<"Failed to get HW surface format."<<std::endl;
    return AV_PIX_FMT_NONE;
}
static int decode_write(AVCodecContext *avctx, AVPacket *packet, void ** ret_buffer)
{
    AVFrame *frame = NULL, *sw_frame = NULL;
    AVFrame *tmp_frame = NULL;
    uint8_t* buffer = NULL;
    int size;
    int ret = 0;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        LOG(ERROR)<<"Error during decoding."<<std::endl;
        return ret;
    }

    while (1) {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            LOG(ERROR)<<"Can not alloc frame."<<std::endl;
            ret = AVERROR(ENOMEM);
            goto fail;
        }

        ret = avcodec_receive_frame(avctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {

            LOG(INFO)<<"avcodec_receive_frame ret error."<<std::endl;
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            LOG(ERROR)<<"Error while decoding."<<std::endl;
            goto fail;
        }

        if (frame->format == hw_pix_fmt) {
            /* retrieve data from GPU to CPU */
            start = clock();
            if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
                LOG(ERROR)<<"av_hwframe_transfer_data error."<<std::endl;
                goto fail;
            }
            stop = clock();
            duration = ((double)(stop - start))/CLOCKS_PER_SEC;
            LOG(INFO)<<"av_hwframe_transfer_data " << duration << " seconds"<<std::endl;
            tmp_frame = sw_frame;
        } else
            tmp_frame = frame;
        size = av_image_get_buffer_size((AVPixelFormat)tmp_frame->format, tmp_frame->width,
                                        tmp_frame->height, 1);
        buffer = (uint8_t * )av_malloc(size);
        if (!buffer) {
            LOG(ERROR)<<"Can not alloc buffer."<<std::endl;
            ret = AVERROR(ENOMEM);
            goto fail;
        }
        start = clock();

        ret = av_image_copy_to_buffer(buffer, size,
                                      (const uint8_t * const *)tmp_frame->data,
                                      (const int *)tmp_frame->linesize, (AVPixelFormat)tmp_frame->format,
                                      tmp_frame->width, tmp_frame->height, 1);
        stop = clock();
        duration = ((double)(stop - start))/CLOCKS_PER_SEC;
        LOG(INFO)<<"av_image_copy_to_buffer " << duration << " seconds"<<std::endl;
        if (ret < 0) {
            LOG(ERROR)<<"Can not copy image to buffer."<<std::endl;
            goto fail;
        }
#if 0 //following code is to dump a frame
        static int frame_count2=0;
                 printf("frame_count2 = %u\n",frame_count2);
                 FILE *fp;
                if (frame_count2 == 1)
                {
                    if((fp=fopen("/tmp/1.raw","w"))==NULL) {
                        printf("File cannot be opened\n");
                        exit(1);

                    }
                    fwrite(buffer, 1, size,fp);
                    if(fclose(fp)!=0) {
                        printf("File cannot be closed\n");
                        exit(1);
                    }
                    else
                        printf("File is now closed\n");
                }

                frame_count2++;
#endif
                *ret_buffer = buffer;
fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        //av_freep(&buffer);
        if (ret < 0)
            return ret;
    }
}

static int decode_write2(AVCodecContext *avctx, AVPacket *packet, void ** ret_buffer)
{
    AVFrame *frame = NULL, *sw_frame = NULL;
    uint8_t* buffer = NULL;
    int ret = 0;
    VADisplay display;

    ret = avcodec_send_packet(avctx, packet);
    if (ret < 0) {
        LOG(ERROR)<<"Error during decoding."<<std::endl;
        return ret;
    }

    while (1) {
        if (!(frame = av_frame_alloc()) || !(sw_frame = av_frame_alloc())) {
            LOG(ERROR)<<"Can not alloc frame."<<std::endl;
            ret = AVERROR(ENOMEM);
            goto fail;
        }
#if USE_DMA_BUFFER
        ret = avcodec_receive_frame2(avctx, frame, &display);
#else
        //exit if not define USE_DMA_BUFFER
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        return 0;
#endif
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {

            //std::cout << "avcodec_receive_frame ret error."<< std::endl;
            av_frame_free(&frame);
            av_frame_free(&sw_frame);
            return 0;
        } else if (ret < 0) {
            LOG(ERROR)<<"Error while decoding."<<std::endl;
            goto fail;
        }

        if (frame->format == hw_pix_fmt) {
            vaSyncSurface(display, (VASurfaceID)(long)frame->data[3]);
            vaDeriveImage(display, (VASurfaceID)(long)frame->data[3], &va_image);
            buffer_info.mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
            vaAcquireBufferHandle(display, va_image.buf, &buffer_info);
        }

#if DUMP_FRAME //dump a frame
        static int frame_count2=0;
        LOG(INFO)<<"frame_count2 = "<< frame_count2 <<std::endl;
        FILE *fp;
        if (frame_count2 == 1)
        {
            if((fp=fopen("/tmp/1.raw","w"))==NULL) {
                LOG(ERROR)<<"File cannot be opened."<<std::endl;
                exit(1);
            }
            fwrite(buffer, 1, size,fp);
            if(fclose(fp)!=0) {
                LOG(ERROR)<<"File cannot be closed."<<std::endl;
                exit(1);
            }
        else
            LOG(INFO)<<"File is now closed."<<std::endl;
        }
        frame_count2++;
#endif
        *ret_buffer = buffer;
fail:
        av_frame_free(&frame);
        av_frame_free(&sw_frame);
        //av_freep(&buffer);
        if (ret < 0)
            return ret;
    }
}

RenderStatus FFmpegMediaSource::GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo)
{
    int ret = 0;

    LOG(INFO)<<"entering FFmpegMediaSource::GetFrame."<<std::endl;
    if (m_mediaSourceInfo.pixFormat == PixelFormat::AV_PIX_FMT_NV12) //hardware decoding
    {

        LOG(INFO)<<"hw decoding."<<std::endl;
        start = clock();

        if (av_read_frame(m_ffmpegSourceData.fmt_ctx, m_ffmpegSourceData.packet) < 0)
        {
            // av_free_packet(m_ffmpegSourceData.packet);
            av_packet_unref(m_ffmpegSourceData.packet);
            LOG(ERROR)<<"av_read_frame error."<<std::endl;
            return RENDER_ERROR;
        }
        stop = clock();
        duration = ((double)(stop - start))/CLOCKS_PER_SEC;
        LOG(INFO)<<"av_read_frame "<<duration<<" seconds"<<std::endl;
        m_mediaSourceInfo.currentFrameNum++;
        if (m_ffmpegSourceData.packet->stream_index == m_ffmpegSourceData.stream_idx)
        {
            ret = decode_write(m_ffmpegSourceData.codec_ctx,m_ffmpegSourceData.packet, (void **)m_ffmpegSourceData.av_frame->data);

        }
        // av_free_packet(m_ffmpegSourceData.packet);
        av_packet_unref(m_ffmpegSourceData.packet);
    }
    else if (m_mediaSourceInfo.pixFormat == PixelFormat::AV_PIX_FMT_NV12_DMA_BUFFER) //hardware decoding +dma buffer sharing
    {

        LOG(INFO)<<"hw decoding + dma buffer sharing."<<std::endl;
        start = clock();

        if (av_read_frame(m_ffmpegSourceData.fmt_ctx, m_ffmpegSourceData.packet) < 0)
        {
            // av_free_packet(m_ffmpegSourceData.packet);
            av_packet_unref(m_ffmpegSourceData.packet);
            LOG(ERROR)<<"av_read_frame error."<<std::endl;
            return RENDER_ERROR;
        }
        stop = clock();
        duration = ((double)(stop - start))/CLOCKS_PER_SEC;
        LOG(INFO)<<"av_read_frame "<<duration<<" seconds"<<std::endl;
        m_mediaSourceInfo.currentFrameNum++;
        if (m_ffmpegSourceData.packet->stream_index == m_ffmpegSourceData.stream_idx)
        {
            ret = decode_write2(m_ffmpegSourceData.codec_ctx,m_ffmpegSourceData.packet, (void **)m_ffmpegSourceData.av_frame->data);

        }
        // av_free_packet(m_ffmpegSourceData.packet);
        av_packet_unref(m_ffmpegSourceData.packet);
    }
   else // software decoding
    {
        LOG(INFO)<<"sw decoding."<<std::endl;
        do
        {
            if (av_read_frame(m_ffmpegSourceData.fmt_ctx, m_ffmpegSourceData.packet) < 0)
            {
                // av_free_packet(m_ffmpegSourceData.packet);
                av_packet_unref(m_ffmpegSourceData.packet);
                LOG(ERROR)<<"av_read_frame error."<<std::endl;
                return RENDER_ERROR;
            }
            m_mediaSourceInfo.currentFrameNum++;
            if (m_ffmpegSourceData.packet->stream_index == m_ffmpegSourceData.stream_idx)
            {
                int32_t frame_finished = 0; //should set 0 for software decoding
                if (avcodec_decode_video2(m_ffmpegSourceData.codec_ctx, m_ffmpegSourceData.av_frame, &frame_finished, m_ffmpegSourceData.packet) < 0)
                {
                    // av_free_packet(m_ffmpegSourceData.packet);
                    av_packet_unref(m_ffmpegSourceData.packet);
                    return RENDER_ERROR;
                }

                /*
                if (frame_finished) //render.cpp
                {
                    // ABANDON colorSpace Conversion using SW.
                    // if (!m_ffmpegSourceData.conv_ctx)
                    // {
                    //     m_ffmpegSourceData.conv_ctx = sws_getContext(m_ffmpegSourceData.codec_ctx->width,
                    //         m_ffmpegSourceData.codec_ctx->height, m_ffmpegSourceData.codec_ctx->pix_fmt,
                    //         m_ffmpegSourceData.codec_ctx->width, m_ffmpegSourceData.codec_ctx->height, AV_PIX_FMT_RGB24,
                    //         SWS_BICUBIC, NULL, NULL, NULL);
                    // }
                    // sws_scale(m_ffmpegSourceData.conv_ctx, m_ffmpegSourceData.av_frame->data, m_ffmpegSourceData.av_frame->linesize, 0,
                    //     m_ffmpegSourceData.codec_ctx->height, m_ffmpegSourceData.gl_frame->data, m_ffmpegSourceData.gl_frame->linesize);
                }
                else
                {
                    return RENDER_ERROR;
                }*/
            }
            // av_free_packet(m_ffmpegSourceData.packet);
            av_packet_unref(m_ffmpegSourceData.packet);
        } while ((m_ffmpegSourceData.packet->stream_index != m_ffmpegSourceData.stream_idx));

    }

    //buffer allocated.
    LOG(INFO)<<"m_mediaSourceInfo.pixFormat= "<< m_mediaSourceInfo.pixFormat << "ret= "<< ret <<std::endl;
    switch (m_mediaSourceInfo.pixFormat)
    {
    case PixelFormat::PIX_FMT_RGB24:
        buffer[0] = (uint8_t *)malloc(sizeof(uint8_t) * m_ffmpegSourceData.codec_ctx->width * m_ffmpegSourceData.codec_ctx->height * 3);
        buffer[0] = m_ffmpegSourceData.av_frame->data[0];
        break;
    case PixelFormat::PIX_FMT_YUV420P:
        //buffer[0] = (uint8_t *)malloc(sizeof(uint8_t) * m_ffmpegSourceData.codec_ctx->width * m_ffmpegSourceData.codec_ctx->height);
        buffer[0] = m_ffmpegSourceData.av_frame->data[0];
        //buffer[1] = (uint8_t *)malloc(sizeof(uint8_t) * m_ffmpegSourceData.codec_ctx->width * m_ffmpegSourceData.codec_ctx->height * 0.25);
        buffer[1] = m_ffmpegSourceData.av_frame->data[1];
        //buffer[2] = (uint8_t *)malloc(sizeof(uint8_t) * m_ffmpegSourceData.codec_ctx->width * m_ffmpegSourceData.codec_ctx->height * 0.25);
        buffer[2] = m_ffmpegSourceData.av_frame->data[2];
        break;
    case PixelFormat::AV_PIX_FMT_NV12:
    case PixelFormat::AV_PIX_FMT_NV12_DMA_BUFFER:
        buffer[0] = m_ffmpegSourceData.av_frame->data[0];
        buffer[1] = m_ffmpegSourceData.av_frame->data[0]+ m_ffmpegSourceData.codec_ctx->width *m_ffmpegSourceData.codec_ctx->height;
        break;

    default:
        LOG(ERROR)<<"Wrong PixelFormat."<<std::endl;
        break;
    }
    //get rwpk and region information.
    if (RENDER_STATUS_OK != SetRegionInfo(regionInfo))
    {
        return RENDER_ERROR;
    }
    return RENDER_STATUS_OK;
}

RenderStatus FFmpegMediaSource::SetRegionInfo(struct RegionInfo *regionInfo)
{
    if (NULL == regionInfo)
    {
        return RENDER_ERROR;
    }
    // hard code to set regionInfo
    regionInfo->sourceNumber = SOURCENUMBER;
    regionInfo->regionWisePacking = (RegionWisePacking *)malloc(sizeof(RegionWisePacking));
    if (NULL == regionInfo->regionWisePacking)
    {
        return RENDER_ERROR;
    }
    regionInfo->regionWisePacking->numRegions = REGIONNUMBER;
    regionInfo->regionWisePacking->projPicWidth = FULLWIDTH;
    regionInfo->regionWisePacking->projPicHeight = FULLHEIGHT;
    regionInfo->regionWisePacking->packedPicWidth = PACKEDWIDTH;
    regionInfo->regionWisePacking->packedPicHeight = PACKEDHEIGHT;
    regionInfo->regionWisePacking->rectRegionPacking = (RectangularRegionWisePacking *)malloc(sizeof(RectangularRegionWisePacking) * regionInfo->regionWisePacking->numRegions);
    if (NULL == regionInfo->regionWisePacking->rectRegionPacking)
    {
        return RENDER_ERROR;
    }
    uint32_t highStep = 1280;
    uint32_t highReso = 1280;
    // hard code to set rwpk
    for (uint32_t i = 0; i < 9; i++)
    {
        regionInfo->regionWisePacking->rectRegionPacking[i].packedRegWidth = highReso;
        regionInfo->regionWisePacking->rectRegionPacking[i].packedRegHeight = highReso;
        regionInfo->regionWisePacking->rectRegionPacking[i].packedRegLeft = (i % 3) * highStep;
        regionInfo->regionWisePacking->rectRegionPacking[i].packedRegTop = i / 3 * highStep;

        regionInfo->regionWisePacking->rectRegionPacking[i].projRegWidth = highReso;
        regionInfo->regionWisePacking->rectRegionPacking[i].projRegHeight = highReso;
        regionInfo->regionWisePacking->rectRegionPacking[i].projRegLeft = (i % 3) * highStep;
        regionInfo->regionWisePacking->rectRegionPacking[i].projRegTop = i / 3 * highStep;
    }
    uint32_t lowStep = 640;
    uint32_t lowReso = 640;
    for (uint32_t i = 0; i < 18; i++)
    {
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].packedRegWidth = lowReso;
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].packedRegHeight = lowReso;
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].packedRegLeft = highReso * 3 + i / 6 * lowStep;
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].packedRegTop = (i % 6) * lowStep;

        regionInfo->regionWisePacking->rectRegionPacking[i + 9].projRegWidth = lowReso;
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].projRegHeight = lowReso;
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].projRegLeft = (i % 6) * lowStep;
        regionInfo->regionWisePacking->rectRegionPacking[i + 9].projRegTop = i / 6 * lowStep;
    }
    regionInfo->sourceInfo = (struct SourceInfo *)malloc(sizeof(struct SourceInfo) * regionInfo->sourceNumber);
    if (NULL == regionInfo->sourceInfo)
    {
        return RENDER_ERROR;
    }
    regionInfo->sourceInfo[0].sourceWidth = regionInfo->regionWisePacking->projPicWidth;
    regionInfo->sourceInfo[0].sourceHeight = regionInfo->regionWisePacking->projPicHeight;
    regionInfo->sourceInfo[0].tileColumnNumber = regionInfo->sourceInfo[0].sourceWidth / regionInfo->regionWisePacking->rectRegionPacking[0].projRegWidth;
    regionInfo->sourceInfo[0].tileRowNumber = regionInfo->sourceInfo[0].sourceHeight / regionInfo->regionWisePacking->rectRegionPacking[0].projRegHeight;
    //low reso tile hard code
    regionInfo->sourceInfo[1].sourceWidth = LOWWIDTH;
    regionInfo->sourceInfo[1].sourceHeight = LOWHEIGHT;
    regionInfo->sourceInfo[1].tileColumnNumber = regionInfo->sourceInfo[1].sourceWidth / regionInfo->regionWisePacking->rectRegionPacking[9].projRegWidth;
    regionInfo->sourceInfo[1].tileRowNumber = regionInfo->sourceInfo[1].sourceHeight / regionInfo->regionWisePacking->rectRegionPacking[9].projRegHeight;
    return RENDER_STATUS_OK;
}

RenderStatus FFmpegMediaSource::ClearFFmpegSourceData()
{
    if (m_ffmpegSourceData.fmt_ctx)
        avformat_free_context(m_ffmpegSourceData.fmt_ctx);
    if (m_ffmpegSourceData.codec_ctx)
        avcodec_close(m_ffmpegSourceData.codec_ctx);
    if (m_ffmpegSourceData.packet)
        av_free(m_ffmpegSourceData.packet);
    if (m_ffmpegSourceData.av_frame)
        av_free(m_ffmpegSourceData.av_frame);
    if (m_ffmpegSourceData.gl_frame)
        av_free(m_ffmpegSourceData.gl_frame);
    InitializeFFmpegSourceData();
    return RENDER_STATUS_OK;
}

RenderStatus FFmpegMediaSource::InitializeFFmpegSourceData()
{
    m_ffmpegSourceData.fmt_ctx = NULL;
    m_ffmpegSourceData.stream_idx = -1;
    m_ffmpegSourceData.video_stream = NULL;
    m_ffmpegSourceData.codec_ctx = NULL;
    m_ffmpegSourceData.decoder = NULL;
    m_ffmpegSourceData.packet = NULL;
    m_ffmpegSourceData.av_frame = NULL;
    m_ffmpegSourceData.gl_frame = NULL;
    m_ffmpegSourceData.conv_ctx = NULL;
    return RENDER_STATUS_OK;
}

RenderStatus FFmpegMediaSource::Initialize(struct RenderConfig renderConfig)
{
    int ret, video_stream;
    // initialize libav
    // av_register_all();
    avformat_network_init();
    // open video
    if (avformat_open_input(&m_ffmpegSourceData.fmt_ctx, renderConfig.url, NULL, NULL) < 0)
    {
        std::cout << "failed to open input" << std::endl;
        ClearFFmpegSourceData();
        return RENDER_ERROR;
    }
    // find stream info
    if (avformat_find_stream_info(m_ffmpegSourceData.fmt_ctx, NULL) < 0)
    {
        std::cout << "failed to get stream info" << std::endl;
        ClearFFmpegSourceData();
        return RENDER_ERROR;
    }
    // dump debug info
    av_dump_format(m_ffmpegSourceData.fmt_ctx, 0, renderConfig.url, 0);
    // find the video stream
    for (unsigned int i = 0; i < m_ffmpegSourceData.fmt_ctx->nb_streams; i++)
    {
        if (m_ffmpegSourceData.fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_ffmpegSourceData.stream_idx = i;
            break;
        }
    }
    if (m_ffmpegSourceData.stream_idx == -1)
    {
        std::cout << "failed to find video stream" << std::endl;
        ClearFFmpegSourceData();
        return RENDER_ERROR;
    }
    m_ffmpegSourceData.video_stream = m_ffmpegSourceData.fmt_ctx->streams[m_ffmpegSourceData.stream_idx];
    m_ffmpegSourceData.codec_ctx = m_ffmpegSourceData.video_stream->codec;
    // find the decoder
    //m_ffmpegSourceData.decoder = avcodec_find_decoder(m_ffmpegSourceData.codec_ctx->codec_id);
    ret = av_find_best_stream(m_ffmpegSourceData.fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &m_ffmpegSourceData.decoder, 0);
    if (ret<0)
    {
        fprintf(stderr, "Cannot find a video stream in the input file\n");
        return RENDER_ERROR;
    }
    video_stream = ret;

    if (m_ffmpegSourceData.decoder == NULL)
    {
        std::cout << "failed to find decoder" << std::endl;
        ClearFFmpegSourceData();
        return RENDER_ERROR;
    }

    if(renderConfig.decoderType == VAAPI_DECODER) // HW decoder
    {
        for (int i = 0;; i++) {
            const AVCodecHWConfig *config = avcodec_get_hw_config(m_ffmpegSourceData.decoder,i);
            if (!config) {
                fprintf(stderr, "Decoder %s does not support device type %s.\n",
                        m_ffmpegSourceData.decoder->name, av_hwdevice_get_type_name(AV_HWDEVICE_TYPE_VAAPI));
                return RENDER_ERROR;
            }
            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == AV_HWDEVICE_TYPE_VAAPI) {
                hw_pix_fmt = config->pix_fmt;
                break;
            }
        }
    }

    if (!(m_ffmpegSourceData.codec_ctx = avcodec_alloc_context3(m_ffmpegSourceData.decoder)))
        return RENDER_ERROR;

    m_ffmpegSourceData.video_stream = m_ffmpegSourceData.fmt_ctx->streams[video_stream];
    if ((ret = avcodec_parameters_to_context(m_ffmpegSourceData.codec_ctx, m_ffmpegSourceData.video_stream->codecpar)) < 0) {
        LOG(ERROR)<<"avcodec_parameters_to_context error. Error code: %s\n"<<std::endl;
        return RENDER_ERROR;
    }

    //m_ffmpegSourceData.codec_ctx->get_format    = get_hw_format;

    if(renderConfig.decoderType == VAAPI_DECODER) // HW decoder
    {
        int ret_value = av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);
        if (ret_value < 0) {
            LOG(ERROR)<<"Failed to create a VAAPI device. Error code: %s\n"<<std::endl;
            m_mediaSourceInfo.pixFormat = PixelFormat::PIX_FMT_YUV420P;
        }
        else
        {
            if (renderConfig.useDMABuffer == 1)
                m_mediaSourceInfo.pixFormat = PixelFormat::AV_PIX_FMT_NV12_DMA_BUFFER;
            else
                m_mediaSourceInfo.pixFormat = PixelFormat::AV_PIX_FMT_NV12;
            m_ffmpegSourceData.codec_ctx->get_format    = get_hw_format;
        }
        if (hw_device_ctx)
            m_ffmpegSourceData.codec_ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
        else
            m_ffmpegSourceData.codec_ctx->hw_device_ctx = NULL;
        if (!m_ffmpegSourceData.codec_ctx->hw_device_ctx) {
            fprintf(stderr, "A hardware device reference create failed.\n");
        }
    	else
	    	fprintf(stderr, "A hardware device reference create success.\n");
    }
    else // SW decoder
         m_mediaSourceInfo.pixFormat = PixelFormat::PIX_FMT_YUV420P;

    // open the decoder
    if (avcodec_open2(m_ffmpegSourceData.codec_ctx, m_ffmpegSourceData.decoder, NULL) < 0)
    {
        std::cout << "failed to open codec" << std::endl;
        ClearFFmpegSourceData();
        return RENDER_ERROR;
    }
    // allocate the video frames
    m_ffmpegSourceData.av_frame = av_frame_alloc();
    m_ffmpegSourceData.gl_frame = av_frame_alloc();
    int32_t size = avpicture_get_size(AV_PIX_FMT_RGB24, m_ffmpegSourceData.codec_ctx->width, m_ffmpegSourceData.codec_ctx->height);
    uint8_t *internal_buffer = (uint8_t *)av_malloc(size * sizeof(uint8_t));
    avpicture_fill((AVPicture *)m_ffmpegSourceData.gl_frame, internal_buffer, AV_PIX_FMT_RGB24, m_ffmpegSourceData.codec_ctx->width, m_ffmpegSourceData.codec_ctx->height);
    m_ffmpegSourceData.packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    m_sourceType = MediaSourceType::SOURCE_VOD;
    if (RENDER_STATUS_OK != SetMediaSourceInfo(NULL))
    {
        return RENDER_ERROR;
    }
    return RENDER_STATUS_OK;
}

struct MediaSourceInfo FFmpegMediaSource::GetMediaSourceInfo()
{
    return m_mediaSourceInfo;
}

void *FFmpegMediaSource::GetSourceMetaData()
{
    return &m_ffmpegSourceData;
}

bool FFmpegMediaSource::IsEOS()
{
    if (m_sourceType == MediaSourceType::SOURCE_VOD)
    {
        std::cout << "frame:" << m_mediaSourceInfo.currentFrameNum << std::endl;
        return m_mediaSourceInfo.currentFrameNum == m_mediaSourceInfo.frameNum;
    }
    else if (m_sourceType == MediaSourceType::SOURCE_LIVE)
    {
        return false; //live video should be controlled by player(pause and play, etc) will be added soon.
    }
    return false;
}

RenderStatus FFmpegMediaSource::SetMediaSourceInfo(void *mediaInfo)
{
    m_mediaSourceInfo.width = m_ffmpegSourceData.codec_ctx->width;
    m_mediaSourceInfo.height = m_ffmpegSourceData.codec_ctx->height;
    m_mediaSourceInfo.projFormat = VCD::OMAF::PF_ERP; //hard code
    //m_mediaSourceInfo.pixFormat = PixelFormat::AV_PIX_FMT_NV12; //PixelFormat::PIX_FMT_YUV420P; //hard code for hw
    m_mediaSourceInfo.hasAudio = false;
    m_mediaSourceInfo.audioChannel = 0;
    m_mediaSourceInfo.numberOfStreams = 1;
    m_mediaSourceInfo.stride = m_ffmpegSourceData.codec_ctx->width;
    m_mediaSourceInfo.frameNum = 100; //hard code
    m_mediaSourceInfo.currentFrameNum = 0;
    m_mediaSourceInfo.sourceWH = new SourceWH;
    m_mediaSourceInfo.sourceWH->width = new uint32_t[SOURCENUMBER];
    m_mediaSourceInfo.sourceWH->width[0] = FULLWIDTH; //hard
    m_mediaSourceInfo.sourceWH->width[1] = LOWWIDTH;
    m_mediaSourceInfo.sourceWH->height = new uint32_t[SOURCENUMBER];
    m_mediaSourceInfo.sourceWH->height[0] = FULLHEIGHT;
    m_mediaSourceInfo.sourceWH->height[1] = LOWHEIGHT;
    if (m_ffmpegSourceData.fmt_ctx->duration != AV_NOPTS_VALUE)
    {
        m_mediaSourceInfo.duration = m_ffmpegSourceData.video_stream->duration * (1.0 * m_ffmpegSourceData.video_stream->time_base.num / m_ffmpegSourceData.video_stream->time_base.den);
    }
    if (m_ffmpegSourceData.video_stream->r_frame_rate.den > 0)
    {
        m_mediaSourceInfo.frameRate = m_ffmpegSourceData.video_stream->r_frame_rate.num / m_ffmpegSourceData.video_stream->r_frame_rate.den;
    }
    isAllValid = true;
    return RENDER_STATUS_OK;
}

RenderStatus FFmpegMediaSource::ChangeViewport(float yaw, float pitch)
{
    //DO NOTHING
    return RENDER_STATUS_OK;
}

//use in dash media source fifo.
//ffmpegmediasource has no fifo.
//do nothing.
void FFmpegMediaSource::DeleteBuffer(uint8_t **buffer)
{
    return;
}

void FFmpegMediaSource::ClearRWPK(RegionWisePacking *rwpk)
{
    return;
}

VCD_NS_END
#endif /* USE_DMA_BUFFER */