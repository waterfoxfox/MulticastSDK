//***************************************************************************//
//* 版权所有  www.mediapro.cc
//*
//* 内容摘要：客户端对外组播 SDK DLL接口
//*	
//* 当前版本：V1.0		
//* 作    者：mediapro
//* 完成日期：2020-4-26
//**************************************************************************//

#ifndef _SD_TERMINAL_MULTICAST_SDK_H_
#define _SD_TERMINAL_MULTICAST_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DLL_EXPORTS
    #ifdef __GNUC__
      #define DLLIMPORT_MULTICAST_SDK __attribute__ ((dllexport))
    #else
      #define DLLIMPORT_MULTICAST_SDK __declspec(dllexport) 
    #endif
  #else
    #ifdef __GNUC__
      #define DLLIMPORT_MULTICAST_SDK 
    #else
      #define DLLIMPORT_MULTICAST_SDK 
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define DLLIMPORT_MULTICAST_SDK __attribute__ ((visibility ("default")))
  #else
    #define DLLIMPORT_MULTICAST_SDK
  #endif
#endif

#ifdef __APPLE__
#ifndef OBJC_BOOL_DEFINED
typedef int BOOL;
#endif 
#else
#ifndef BOOL
typedef int BOOL;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//日志输出的级别
typedef enum LOG_OUTPUT_LEVEL_MULTICAST
{
	MULTICAST_LOG_OUTPUT_LEVEL_DEBUG = 1,
	MULTICAST_LOG_OUTPUT_LEVEL_INFO,
	MULTICAST_LOG_OUTPUT_LEVEL_WARNING,
	MULTICAST_LOG_OUTPUT_LEVEL_ERROR,
	MULTICAST_LOG_OUTPUT_LEVEL_ALARM,
	MULTICAST_LOG_OUTPUT_LEVEL_FATAL,
	MULTICAST_LOG_OUTPUT_LEVEL_NONE
} LOG_OUTPUT_LEVEL_MULTICAST;


//FEC冗余方法
typedef enum FEC_REDUN_TYPE_MULTICAST
{
	//自动冗余度(组播不支持自动冗余度)
	MULTICAST_FEC_AUTO_REDUN_TYPE = 0,
	//固定冗余度
	MULTICAST_FEC_FIX_REDUN_TYPE
} FEC_REDUN_TYPE_MULTICAST;



//客户端类别
typedef enum TERMINAL_TYPE_MULTICAST
{	
	MULTICAST_TERMINAL_TYPE_RECV_ONLY = 0,
	MULTICAST_TERMINAL_TYPE_SEND_ONLY = 1
} TERMINAL_TYPE_MULTICAST;



//送外层视频码流时附带的信息
typedef struct VideoFrameInforMulticast
{
	unsigned int unWidth;
	unsigned int unHeight;
	unsigned int unFps;
	BOOL bPacketLost;
	BOOL bKeyFrame;
	BOOL bInfoUpdated;
	BOOL bIsHevc;
	//注意VPS\SPS\PPS中不含起始码
	unsigned char byVps[512];
	unsigned int unVpsSize;
	unsigned char bySps[512];
	unsigned int unSpsSize;
	unsigned char byPps[512];
	unsigned int unPpsSize;
}VideoFrameInforMulticast;


//送外层音频码流时附带的信息
typedef struct AudioFrameInforMulticast
{
	unsigned int unCodecType;
	unsigned int unSampleRate;
	unsigned int unChannelNum;
	unsigned int unFrameNo;
	BOOL bInfoUpdated;
}AudioFrameInforMulticast;



//回调函数
// 【注意事项】
//	1、通知型回调函数中应尽可能快的退出，不进行耗时操作，不调用SDTerminal系列API接口。
//  2、数据型回调函数中允许进行解码处理

// 收到服务器发来的视频
typedef  void (*RecvMulticastRemoteVideoFunc)(void* pObject, unsigned char* data, unsigned int unLen, unsigned int unPTS, VideoFrameInforMulticast* pFrameInfo);

// 收到服务器发来的音频
typedef  void (*RecvMulticastRemoteAudioFunc)(void* pObject, unsigned char* data, unsigned int unLen, unsigned int unPTS, AudioFrameInforMulticast* pFrameInfo);




//////////////////////////////////////////////////////////////////////////
// SDTerminal SDK接口

/***
* 环境初始化，系统只需调用一次，主要用于环境以及日志模块的初始化
* @param: outputPath表示日志存放路径，支持相对路径和绝对路径，若目录不存在将自动创建
* @param: outputLevel表示日志输出的级别，只有等于或者高于该级别的日志输出到文件，取值范围参考LOG_OUTPUT_LEVEL
* @return: 
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_Enviroment_Init(const char* outputPath, int outputLevel);

DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_Enviroment_Free();

/***
* 创建SDTerminal
* @return: 返回模块指针，为NULL则失败
*/
DLLIMPORT_MULTICAST_SDK void*  SDTerminalMulticast_Create();

/***
* 销毁SDTerminal，并设置指针为NULL，使用者应该做好与其他API之间的互斥保护
* @param ppTerminal: 模块指针的指针
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_Delete(void** ppTerminal);


/***
* 准备会话
* @param pTerminal: 模块指针
* @param eUserType: 客户端类型，有组播发送端与组播接收端两种。
* @param strLocalIp: 组播绑定的本地网络IP，当设置为NULL时，内部将使用INADDR_ANY，交由操作系统选择一个网卡IP。建议指定IP以防多IP时系统选择错误
* @param strMultiIp: 组播IP
* @param shMultiPort: 组播端口号，将使用本端口发送音频数据  使用本端口+2发送视频数据
* @return: <0为失败错误码，>=0为成功
*/
DLLIMPORT_MULTICAST_SDK int  SDTerminalMulticast_Online(void* pTerminal, TERMINAL_TYPE_MULTICAST eUserType, const char *strLocalIp, const char *strMultiIp, unsigned short shMultiPort);

 
    
/***
* 结束会话
* @param pTerminal: 模块指针
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_Offline(void* pTerminal);



/***
* 发送音频数据
* @param pTerminal: 模块指针
* @param byBuf: 发送已编码的一帧音频码流【必须输入ADTS流】
* @param unLen: 数据长度
* @param unDts: SDTerminalMulticast_SetUseInternalTimeStamp指定为用户提供时间戳时，本参数供用户传入时间戳。默认为内部时间戳模式，本参数被忽略
* @return: 
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SendAudioData(void* pTerminal, unsigned char *byBuf, unsigned int unLen, unsigned int unDts);



/***
* 发送视频数据
* @param pTerminal: 模块指针
* @param byBuf: 发送已编码的一帧视频码流，内部自带拆分功能【必须输入带起始码的码流】
* @param unLen: 数据长度
* @param unDts: SDTerminalMulticast_SetUseInternalTimeStamp指定为用户提供时间戳时，本参数供用户传入时间戳。默认为内部时间戳模式，本参数被忽略
* @param bIsHevc:传入码流是否为HEVC, H264时传入FALSE，请务必与实际情况相符
* @return: 
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SendVideoData(void* pTerminal, unsigned char *byBuf, unsigned int unLen, unsigned int unDts, BOOL bIsHevc);


/***
* 设置上行传输参数，若不调用本API将使用默认传输参数【必须Online之前调用】
* @param pTerminal: 模块指针
* @param unJitterBuffDelay: 接收缓存时间，单位ms
* @param eFecRedunMethod: FEC 冗余调整方法
* @param unFecRedunRatio: FEC 固定冗余度时对应的上行冗余比率或自动冗余度时的对应的上行冗余上限，比如设置为30，则表示使用30%冗余。
* @param unFecMinGroupSize: FEC 分组的下限，512Kbps以下建议8，512Kbps~1Mbps建议设置为16，1Mbps~2Mbps建议设置24，2Mbps~4Mbp建议设置28，4Mbps以上建议36
* @param unFecMaxGroupSize: FEC 分组的上限，根据终端CPU能力而定，最大不超过72，越大FEC所消耗的CPU越高，抗丢包能力也越强
* @param bEnableNack：是否启用组播NACK功能，双方均启用时生效。
* @param usNackPort：当开启NACK功能时，组播发送端用于NACK信令、媒体传输的单播端口，收发双方保持设置一致时生效。
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SetTransParams(void* pTerminal, unsigned int unJitterBuffDelay, FEC_REDUN_TYPE_MULTICAST eFecRedunMethod, 
                                          unsigned int unFecRedunRatio, unsigned int unFecMinGroupSize, unsigned int unFecMaxGroupSize, BOOL bEnableNack, unsigned short usNackPort);


/***
* 获取音视频丢包率统计信息（内部已经乘100得到百分比）
* @param pTerminal: 模块指针
* @param pfVideoUpLostRatio: 视频上行丢包率
* @param pfVideoDownLostRatio: 视频下行丢包率
* @param pfAudioUpLostRatio: 音频上行丢包率
* @param pfAudioDownLostRatio: 音频下行丢包率
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_GetVideoAudioUpDownLostRatio(void* pTerminal, float *pfVideoUpLostRatio, float *pfVideoDownLostRatio, 
                                                        float *pfAudioUpLostRatio, float *pfAudioDownLostRatio);



/***
* 获取音视频码率统计信息，单位Kbps
* @param pTerminal: 模块指针
* @param pfVideoUpRate: 视频上行码率
* @param pfVideoDownRate: 视频下行码率
* @param pfAudioUpRate: 音频上行码率
* @param pfAudioDownRate: 音频下行码率
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_GetVideoAudioUpDownBitrate(void* pTerminal, float *pfVideoUpRate, float *pfVideoDownRate, 
                                                      float *pfAudioUpRate, float *pfAudioDownRate);



/***
* 获取SDK版本信息
* @param pTerminal: 模块指针
* @return: 版本号
*/
DLLIMPORT_MULTICAST_SDK unsigned int  SDTerminalMulticast_GetVersion(void* pTerminal);



/***
* 设置视频帧率信息，作为发送时内部的Smoother处理参考
* 注意该帧率要符合实际帧率,可以高于实际帧率，但不能低于实际帧率，否则将导致发送速度不足。不调用本函数时，默认关闭smooth处理【Online之前或者之后均可调用】
* @param pTerminal: 模块指针
* @param unFrameRate: 视频参考帧率
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SetVideoFrameRateForSmoother(void* pTerminal, unsigned int unFrameRate);



/***
* 指定使用内部自动生成时间戳还是使用外部提供的时间戳(Send接口传入)，默认为内部时间戳
* @param pTerminal: 模块指针
* @param bUseInternalTimestamp: TRUE标识采用内部时间戳，FALSE标识用户提供时间戳
* @return:
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SetUseInternalTimeStamp(void* pTerminal, BOOL bUseInternalTimestamp);




/***
* 设置接收视频回调函数【必须Online之前调用生效】
* @param pTerminal: 模块指针
* @param pfRecvRemoteVideoCallback: 接收视频的回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SetRecvRemoteVideoCallback(void* pTerminal, RecvMulticastRemoteVideoFunc pfRecvRemoteVideoCallback, void* pObject);


/***
* 设置接收音频回调函数【必须Online之前调用生效】
* @param pTerminal: 模块指针
* @param pfRecvRemoteAudioCallback: 接收音频的回调函数指针
* @param pObject: 透传给回调函数的用户指针
* @return: 
*/
DLLIMPORT_MULTICAST_SDK void  SDTerminalMulticast_SetRecvRemoteAudioCallback(void* pTerminal, RecvMulticastRemoteAudioFunc pfRecvRemoteAudioCallback, void* pObject);

#ifdef __cplusplus
}
#endif

#endif // _SD_TERMINAL_MULTICAST_SDK_H_
