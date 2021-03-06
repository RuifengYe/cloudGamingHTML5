//all the include and pragma information
#include "stdafx.h"
#pragma  warning(disable:4996)
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <time.h>
#define HAVE_STRUCT_TIMESPEC
#include "pthread.h"
extern "C" {
#include <libavutil/opt.h>  
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  
}


#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avdevice.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"pthreadVC2.lib")
#define __STDC_CONSTANT_MACROS
using namespace std;

//define global variables
LPDIRECT3D9 g_pD3D = NULL;
D3DDISPLAYMODE ddm;
D3DPRESENT_PARAMETERS d3dpp;
IDirect3DDevice9 * g_pd3dDevice;
IDirect3DSurface9 * pSurface;

//buffer is a data structure that holds the data of exactly one pixel
struct buffer {
	byte *frame;
	buffer() {
		frame = new byte[1920 * 1080 * 3];
	}
};


//queue is a generic loop queue, which means the head and rear will wrapback when they hit the end
template <typename T>
class queue {
public:
	T * test;
	int head;
	int rear;
	int size;

	queue(int size) {
		test = new T[size];
		head = 0;
		rear = 0;
		this->size = size;
	}

	void push(T target) {
		test[rear] = target;
		rear = (rear + 1) % size;
	}

	T pop() {
		T tmp = test[head];
		head = (head + 1) % size;
		return tmp;
	}
	bool isEmpty() {
		return head == rear;
	}
	bool isFull() {
		return (rear + 1) % size == head;
	}
};


//initialize the configuration of d3d related 
void d3dini() {
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	//ZeroMemory(&d3dpp, sizeof(d3dpp));
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &ddm);
	d3dpp.Windowed = TRUE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat = ddm.Format;
	d3dpp.BackBufferHeight = ddm.Height;
	d3dpp.BackBufferWidth = ddm.Width;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.hDeviceWindow = GetDesktopWindow();
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	HRESULT hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);
	g_pd3dDevice->CreateOffscreenPlainSurface(ddm.Width, ddm.Height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pSurface, NULL);
}
//int Capture()
//{
	//string address ="d:/1.png";
	//LPCSTR address_pointer = NULL;
	//address_pointer = address.c_str();
	//g_pd3dDevice->GetFrontBufferData(0, pSurface);
		//---以下指令可以把‘表面’保存成内存数据
		//LPD3DXBUFFER bufferedImage = NULL;
		//D3DXSaveSurfaceToFileInMemory(&bufferedImage, D3DXIFF_PNG, pSurface, NULL, NULL);
		//bufferedImage->Release();
		//---以下指令可以把‘表面’保存成指定格式的文件（如果需要其它格式，请参考 MSDN）
	//hr = D3DXSaveSurfaceToFile(address_pointer, D3DXIFF_PNG, pSurface, NULL, NULL);//s保存为png格式
//}

//flush out the remaining data in the encoder
int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
			NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame) {
			ret = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}


//int _tmain(int argc, _TCHAR* argv[])
//{
//	AVFormatContext* pFormatCtx;
//	AVOutputFormat* fmt;
//	AVStream* video_st;
//	AVCodecContext* pCodecCtx;
//	AVCodec* pCodec;
//	//AVCodecParameters *pCodecPar;
//	AVPacket pkt;
//	uint8_t* picture_buf;
//	AVFrame* pFrame;
//	//D3DFORMAT fmt = (D3DFORMAT)MAKEFOURCC('I', '4', '2', '0');
//	int picture_size;
//	int y_size;
//	//int framecnt = 0;
//	int in_w = 1920, in_h = 1080;
//	//int framenum = 100;
//	const char* out_file = "ds.h264";
//	av_register_all();
//	pFormatCtx = avformat_alloc_context();
//	fmt = av_guess_format(NULL, out_file, NULL);
//	pFormatCtx->oformat = fmt;
//	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
//		printf("Failed to open output file! \n");
//		return -1;
//	}
//	video_st = avformat_new_stream(pFormatCtx, 0);
//	if (video_st == NULL) {
//		return -1;
//	}
//	//pCodecPar = avcodec_parameters_alloc();
//	pCodecCtx = video_st->codec;
//	//pCodecPar = video_st->codecpar;
//
//	pCodecCtx->codec_id = fmt->video_codec;
//	//pCodecCtx->codec_id = AV_CODEC_ID_UTVIDEO;
//	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
//	pCodecCtx->pix_fmt = AV_PIX_FMT_RGB24;
//	pCodecCtx->width = in_w;
//	pCodecCtx->height = in_h;
//	pCodecCtx->bit_rate = 1244160000;
//	pCodecCtx->gop_size = 1;
//
//	pCodecCtx->time_base.num = 1;
//	pCodecCtx->time_base.den = 25;
//	video_st->time_base.num = 1;
//	video_st->time_base.den = 25;
//	video_st->avg_frame_rate.den = 1;
//	video_st->avg_frame_rate.num= 25;
//	pCodecCtx->qmin = 10;
//	pCodecCtx->qmax = 51;
//	pCodecCtx->max_b_frames = 3;
//	AVDictionary *param = 0;
//	avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);
//	//H.264  
//	if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
//		av_dict_set(&param, "preset", "slow", 0);
//		av_dict_set(&param, "tune", "zerolatency", 0);
//		//av_dict_set(¶m, "profile", "main", 0);  
//	}
//	//H.265  
//	if (pCodecCtx->codec_id == AV_CODEC_ID_H265) {
//		av_dict_set(&param, "preset", "ultrafast", 0);
//		av_dict_set(&param, "tune", "zero-latency", 0);
//	}
//	//AVCodecID id = AV_CODEC_ID_UTVIDEO;
//	//pCodec = avcodec_find_encoder(id);
//	pCodec = avcodec_find_encoder_by_name("libx264rgb");
//	if (!pCodec) {
//		printf("Can not find encoder! \n");
//		return -1;
//	}
//	if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
//		printf("Failed to open encoder! \n");
//		return -1;
//	}
//	pFrame = av_frame_alloc();
//	picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
//	picture_buf = (uint8_t *)av_malloc(picture_size);
//	avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);//已经deprecated了
//	//Write File Header  
//	avformat_write_header(pFormatCtx, NULL);
//	av_new_packet(&pkt, picture_size);
//	y_size = pCodecCtx->width * pCodecCtx->height;
//
//	CoInitialize(NULL);
//	d3dini();
//	//for (int i = 0; i < 500; i++) {
//
//		DWORD begin, end;
//		begin = ::GetTickCount();
//		int i = 0;
//		while (1) {
//			end = ::GetTickCount();
//			DWORD time_ms = end - begin;
//			if (time_ms >= 40) {
//				cout << "The time is " << time_ms << endl;
//		g_pd3dDevice->GetFrontBufferData(0, pSurface);
//		//string address ="d:/1.png";
//		//std::wstring stemp = std::wstring(address.begin(), address.end());
//		//LPCWSTR address_pointer = stemp.c_str();
//		//D3DXSaveSurfaceToFile(address_pointer, D3DXIFF_PNG, pSurface, NULL, NULL);
//		D3DLOCKED_RECT d3d_rect;
//		pSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
//		byte *buffer=new byte[1920*1080*3];
//		byte* source=(byte *)d3d_rect.pBits;
//		for (int i = 0; i < 1920 * 1080; i++) {
//			buffer[i*3] = *(source + i * 4+2);
//			buffer[i*3+1]= *(source + i * 4 + 1);
//			buffer[i*3+2]= *(source + i * 4 );
//		}
//		byte * pSrc =(byte *) buffer;
//		//byte * pSrc = (byte *)d3d_rect.pBits;
//		//int stride = d3d_rect.Pitch;
//		pFrame->data[0] = pSrc;
//		//pFrame->pts = i * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
//		pFrame->pts = i;
//		pFrame->width = 1920;
//		pFrame->height = 1080;
//		pFrame->format = 2;
//		int got_picture = 0;
//		int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
//		if (ret < 0) {
//			printf("Failed to encode! \n");
//			return -1;
//		}
//		if (got_picture == 1) {
//			printf("Succeed to encode frame: %5d\tsize:%5d\n", i, pkt.size);
//			pkt.stream_index = video_st->index;
//			ret = av_write_frame(pFormatCtx, &pkt);
//			av_free_packet(&pkt);
//		}
//		pSurface->UnlockRect();
//		begin = end;
//		i++;
//			}
//		}
//	//}
//	int ret = flush_encoder(pFormatCtx, 0);
//	if (ret < 0) {
//		printf("Flushing encoder failed\n");
//		return -1;
//	}
//	av_write_trailer(pFormatCtx);
//	if (video_st) {
//		avcodec_close(video_st->codec);
//		av_free(pFrame);
//		av_free(picture_buf);
//	}
//	avio_close(pFormatCtx->pb);
//	avformat_free_context(pFormatCtx);
//	CoUninitialize();
//	pSurface->Release();
//	g_pd3dDevice->Release();
//	g_pD3D->Release();
//	return 0;
//}


//retrieve the frame data from the loop queue and encode it
void* frameConsumer(void * loopQueue) {
	queue<buffer> * tempQueue= (queue<buffer> *)loopQueue;
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* video_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;
	AVPacket pkt;
	uint8_t* picture_buf;
	AVFrame* pFrame;
	int picture_size;
	int y_size;
	int in_w = 1920, in_h = 1080;
	const char* out_file = "ds.h264";
	av_register_all();
	pFormatCtx = avformat_alloc_context();
	fmt = av_guess_format(NULL, out_file, NULL);
	pFormatCtx->oformat = fmt;
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
		printf("Failed to open output file! \n");
		return 0;
	}
	video_st = avformat_new_stream(pFormatCtx, 0);
	if (video_st == NULL) {
		return 0;
	}
	pCodecCtx = video_st->codec;
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_RGB24;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->bit_rate = 8500000;
	pCodecCtx->gop_size = 1;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 8;
	//video_st->time_base.num = 1;
	//video_st->time_base.den = 30;
	//video_st->avg_frame_rate.den = 1;
	//video_st->avg_frame_rate.num = 30;
	pCodecCtx->qmin = 16;
	pCodecCtx->qmax = 26;
	pCodecCtx->max_b_frames = 0;
	AVDictionary *param = 0;
	avcodec_parameters_from_context(video_st->codecpar, pCodecCtx);
	//H.264  
	if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
		//av_dict_set(&param, "preset", "slow", 0);
		//av_dict_set(&param, "tune", "zerolatency", 0);
		//av_dict_set(¶m, "profile", "main", 0);  
	}
	//H.265  
	if (pCodecCtx->codec_id == AV_CODEC_ID_H265) {
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zero-latency", 0);
	}
	pCodec = avcodec_find_encoder_by_name("libx264rgb");
	if (!pCodec) {
		printf("Can not find encoder! \n");
		return 0;
	}
	if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
		printf("Failed to open encoder! \n");
		return 0;
	}
	pFrame = av_frame_alloc();
	picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	picture_buf = (uint8_t *)av_malloc(picture_size);
	avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	//Write File Header  
	avformat_write_header(pFormatCtx, NULL);
	av_new_packet(&pkt, picture_size);
	y_size = pCodecCtx->width * pCodecCtx->height;
	CoInitialize(NULL);
	for (int i = 0; i < 700; i++) {
		while (tempQueue->isEmpty()) {
			Sleep(1);
		}
		byte* tempFrame = (tempQueue->pop()).frame;
		pFrame->data[0] = tempFrame;
		pFrame->pts = i;
		pFrame->width = 1920;
		pFrame->height = 1080;
		pFrame->format = 2;
		int got_picture = 0;
		int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
		/*if (i == 11) {
			cout << "wozaizheli" << endl;
			cout <<unsigned( *(pkt.data+5) )<< endl;
			cout << pkt.size << endl;
		}*/
		if (ret < 0) {
			printf("Failed to encode! \n");
			return 0;
		}
		if (got_picture == 1) {
			printf("Succeed to encode frame: %5d\tsize:%5d\n", i, pkt.size);
			pkt.stream_index = video_st->index;
			ret = av_write_frame(pFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
	}
	int ret = flush_encoder(pFormatCtx, 0);
	if (ret < 0) {
		printf("Flushing encoder failed\n");
		return 0;
	}
	av_write_trailer(pFormatCtx);
	if (video_st) {
		avcodec_close(video_st->codec);
		av_free(pFrame);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);
	CoUninitialize();
	return 0;
}

//extract the RGB data from the ARGB surface and feed it to the loop queue
void* frameProducer(void* loopQueue) {
	queue<buffer> * tempQueue = (queue<buffer> *)loopQueue;
	DWORD begin, end, time_ms, initial;
	DWORD test1, test2;
	begin = ::GetTickCount();
	initial = begin;
	while (begin-initial<40000) {
		end = ::GetTickCount();
		time_ms = end - begin;
		if (time_ms >= 30) {
			buffer frameData;
			while (tempQueue->isFull()) {
				Sleep(1);
			}
			//LPDIRECT3DSURFACE9 back = NULL;
			test1 = ::GetTickCount();
			//g_pd3dDevice->GetFrontBufferData(0, pSurface);
			g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
			test2 = ::GetTickCount();
			cout << test2 - test1 << endl;
			D3DLOCKED_RECT d3d_rect;
			if (pSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT) != D3D_OK)
				cout << "succeed" << endl;
			//byte *processedFrame = new byte[1920 * 1080 * 3];
			byte* source = (byte *)d3d_rect.pBits;
			cout << "hg" << endl;
			for (int i = 0; i < 1920 * 1080; i++) {
				frameData.frame[i * 3] = *(source + i * 4 + 2);
				frameData.frame[i * 3 + 1] = *(source + i * 4 + 1);
				frameData.frame[i * 3 + 2] = *(source + i * 4);
			}
			//memcpy(&processedFrame, frameData.frame, 1920 * 1080 * 3 * sizeof(byte));
			tempQueue->push(frameData);
			pSurface->UnlockRect();
			begin = end;
		}
	}
	return 0;
}

//void* frameProducer(void* loopQueue) {
//	cout << "haha" << endl;
//	queue<buffer> * tempQueue = (queue<buffer> *)loopQueue;
//	buffer frameData;
//	cout << 1 << endl;
//	return 0;
//}



//int _tmain(int argc, _TCHAR* argv[]) {
//	queue<buffer>* test = new queue<buffer>(100);
//	pthread_t producer;
//	pthread_t consumer;
//	pthread_create(&producer, NULL, frameProducer, (void*)test);
//	pthread_create(&consumer, NULL, frameConsumer, (void*)test);
//	pthread_exit(NULL);
//}


//main function
int main() {
	queue<buffer>* test = new queue<buffer>(500);
	pthread_t producer;
	pthread_t consumer;
	d3dini();
	pthread_create(&producer, NULL, frameProducer, (void*)test);
	//cout << "haha" << endl;
	pthread_create(&consumer, NULL, frameConsumer, (void*)test);
	pthread_exit(NULL);
}

