#include <iostream>
#include <vector>
#include <cstring>
#include <avtool.h>
#include <sockets.h>
#include <sstream>
enum RTSP_ERR
{
	
	RTSP_SOCKET_DISCONNECT=1,
	RTSP_SOCKET_ERR,
	RTSP_NO_REQUEST,
	RTSP_UNKNOW_OPTION,
	RTSP_UNKNOW_CSEQ
};
enum RTSP_STAT
{
	RTSP_OPTION=1,
	RTSP_OPTION_DESCRIBE,
	RTSP_OPTION_SETUP,
	RTSP_OPTION_PLAY,
	RTSP_OPTION_TEARDOWN
};
#ifndef K
#define K 1024
#endif
static const char *end_mark="\r\n";
class RtspSession
{
private:
	tcpSender sock_hander;
	std::vector<std::string> reqest;
	std::string session;
	int response_option(int);
	int response_describe(int);
	int response_setup(int);
	int response_play(int);
	int play();
	int client_rtp_port,client_rtcp_port;
	int server_rtp_port,server_rtcp_port;
public:
	RtspSession(){}
	RtspSession(SOCKET _fd):sock_hander(_fd){}
	RtspSession(const tcpSender &tcpsender):sock_hander(tcpsender){}
	void init_rtp_port(const uint16 &rtp,const uint16 &rtcp){server_rtp_port=rtp;server_rtcp_port=rtcp;session="123456789asdfghjkl";}
	int read_request();
	int response_request();
	int get_mothod();
	int get_cseq();
	std::string get_url();
	int get_client_rtp_port();
	void dump_requst();
	std::string get_client_ip(){return sock_hander.get_remote_ip();}
	uint16 get_client_port(){return sock_hander.get_remote_port();}
};

int RtspSession::get_client_rtp_port()
{
	const char *transport="Transport:";
	const char *rtp_port="client_port=";
	if(get_mothod()!=RTSP_OPTION_SETUP)
		return -1;
	for(std::vector<std::string>::iterator iter=reqest.begin();iter!=reqest.end();iter++)
	{
		 if(!strncasecmp(iter->c_str(),transport,strlen(transport)))//iter->compare(0,strlen(cseq_mark),cseq_mark))
		 {
		 	char buf[24];
			const char *rtp_port_fmt="%d-%d";
			const char *p=strcasestr(iter->c_str(),rtp_port)+strlen(rtp_port);
		 	sscanf(p,rtp_port_fmt,&client_rtp_port,&client_rtcp_port);
			return 0;
		 }
		 	
	}
	return -1;
}
std::string RtspSession::get_url()
{
	std::vector<std::string>::iterator option_line=reqest.begin();
	std::string::iterator beg=option_line->begin()+option_line->find(" ")+1,
	 				 end=option_line->begin()+option_line->find_last_of(" ");
	return  std::string(beg,end);
}
int RtspSession::read_request()
{
	char buf[K*2]={0};
	
	char *end;
	int ret;
	char *p;
	char *cur;
	
	ret=sock_hander.recv(buf, K*2);
	if(ret<0)
		return -RTSP_SOCKET_ERR;
	else if(ret==0)
		return -RTSP_SOCKET_DISCONNECT;
	end=buf+strlen(buf);
	cur=buf;
	reqest.clear();
	do
	{
	p=strstr(cur,end_mark);
	if(!p||p+strlen(end_mark)==end)//search end
		break;
	*p=0;
	std::string s(cur);
	reqest.push_back(s);
	p+=strlen(end_mark);
	cur=p;
	}while(1);
	return ret;
}
int RtspSession::get_mothod()
{
	const char *cmd_option="OPTION";
	const char *cmd_desribe="DESCRIBE";
	const char *cmd_setup="SETUP";
	const char *cmd_play="PLAY";
	const char *cmd_teardown="TEARDOWN";
	if(reqest.empty())
		return -RTSP_NO_REQUEST;
	std::vector<std::string>::iterator iter=reqest.begin();
	if(!iter->compare(0,strlen(cmd_option),cmd_option))
		return RTSP_OPTION;
	else if(!iter->compare(0,strlen(cmd_desribe),cmd_desribe))
		return RTSP_OPTION_DESCRIBE;
	else if(!iter->compare(0,strlen(cmd_setup),cmd_setup))
		return RTSP_OPTION_SETUP;
	else if(!iter->compare(0,strlen(cmd_play),cmd_play))
		return RTSP_OPTION_PLAY;
	else if(!iter->compare(0,strlen(cmd_teardown),cmd_teardown))
		return RTSP_OPTION_TEARDOWN;
	else
		return RTSP_UNKNOW_OPTION;
}

int RtspSession::get_cseq()
{
	const char *cseq_mark="CSeq:";
	if(reqest.empty())
		return -RTSP_NO_REQUEST;
	dump_requst();
	for(std::vector<std::string>::iterator iter=reqest.begin();iter!=reqest.end();iter++)
	{
		 if(!iter->compare(0,strlen(cseq_mark),cseq_mark))
		 	return atoi(iter->c_str()+strlen(cseq_mark));
		 	
	}
	return -RTSP_UNKNOW_CSEQ;
	
}
int RtspSession::response_option(int cseq)
{

	const char * response="RTSP/1.0 200 OK\r\n"
						//"Server: PVSS/1.4.8 (Build/20090111; Platform/Win32; Release/StarValley; )"
						"Cseq: %d\r\n"
						"Public: DESCRIBE, SETUP, TEARDOWN, PLAY, OPTIONS\r\n\r\n";
	char buf[K];
	snprintf(buf,K,response,cseq);
	sock_hander.send(buf,strlen(buf));
	std::cerr<<buf<<std::endl;
	return 0;
}

int RtspSession::response_play(int cseq)
{
	const char *response="RTSP/1.0 200 OK\r\n"
						"CSeq: %d\r\n"
						"Session: %s\r\n";
	char buf[K];
	int len;
	len=snprintf(buf,K,response,cseq,session.c_str());
	sock_hander.send(buf, len);
	play();
	return 0;
}
int RtspSession::play()
{
	const char *file="";
	av_input in_file(file);
	return 0;
}
int RtspSession::response_describe(int cseq)
{
	const char *sdp="v=0\r\n"
					"o=- 0 0 IN IP4 127.0.0.1\r\n"
					"s=No Name\r\n"
					"c=IN IP4 0.0.0.0\r\n"
					"t=0 0\r\n"
					"a=tool:libavformat 55.19.104\r\n"
					"m=video 56789 RTP/AVP 96\r\n"
					"b=AS:200\r\n"
					"a=rtpmap:96 MP4V-ES/90000\r\n"
					"a=fmtp:96 profile-level-id=1\r\n\r\n";
	const char * response="RTSP/1.0 200 OK\r\n"
						//"Server: PVSS/1.4.8 (Build/20090111; Platform/Win32; Release/StarValley; )"
						"Content-Base: %s\r\n"
						"Content-Type: application/sdp\r\n"
						"Cseq: %d\r\n"
						"Content-Length: %d\r\n\r\n";
	char buf[K];
	snprintf(buf,K,response,get_url().c_str(),get_cseq(),strlen(sdp));
	sock_hander.send(buf,strlen(buf));
	sock_hander.send(sdp,strlen(sdp));
	std::cerr<<buf<<std::endl;
	std::cerr<<sdp<<std::endl;
	return 0;
}
int RtspSession::response_setup(int cseq)
{
	const char * response="RTSP/1.0 200 OK\r\n"
	"Transport: RTP/AVP/UDP;client_port=%d-%d;server_port=%d-%d\r\n"
	"CSeq: %d\r\n"
	"Session: %s\r\n\r\n";
	char buf[K];
	int len;
	get_client_rtp_port();
	len=snprintf(buf,K,response,
		client_rtp_port,client_rtcp_port,server_rtp_port,server_rtcp_port,
		get_cseq(),
		session.c_str());
	sock_hander.send(buf,len);
	return 0;
}
int RtspSession::response_request()
{
	switch (get_mothod()) 
	{
		case RTSP_OPTION :
			response_option(get_cseq());
			break;
		case RTSP_OPTION_DESCRIBE:
			response_describe(get_cseq());
			break;
		case RTSP_OPTION_SETUP:
			response_setup(get_cseq());
			break;
		case RTSP_OPTION_PLAY:
			response_play(get_cseq());
			break;
		default:
			break;
	}
		return 0;
}
void RtspSession::dump_requst()
{
	for(std::vector<std::string>::iterator iter=reqest.begin();iter!=reqest.end();iter++)
		std::cerr<<*iter<<std::endl;
}

int main()
{
	tcpListener rtsp_ctx(9999,20);

	RtspSession rtspsession(rtsp_ctx.accept());
	std::cerr<<rtspsession.get_client_ip()<<":"<<rtspsession.get_client_port()<<std::endl;
	rtspsession.init_rtp_port(4500,4501);
	rtspsession.read_request();
	rtspsession.response_request();
	rtspsession.read_request();
	rtspsession.response_request();
	rtspsession.read_request();
	rtspsession.response_request();
	sleep(2);
	return 0;
}
#if 0


int main(int argc ,char **argv)
{
	int i,ret,count_a=0,count_v=0;
	AVPacket pkt;
	avformat_network_init();
	if(argc!=2)
	{
		printf("Two parameters !!!\n");
		return -1;
	}
	const char *outfile="rtp://172.21.2.242:56789";
	//const char *outfile="out.avi";
	av_input test(argv[1]);
	av_output out(outfile,"rtp");
	//av_output out(outfile);
	
	//video
	AVStream *sv=out.add_stream(test.get_video_codecID());
	AVCodecContext *c;
	if(sv)
	{
		c=sv->codec;
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
		c->time_base=test.get_video_stream()->time_base;
		c->width=test.get_width();
		c->height=test.get_height();
		av_opt_set(c->priv_data, "preset", "ultrafast", 0);
		av_opt_set(c->priv_data, "tune","stillimage,fastdecode,zerolatency",0);
		av_opt_set(c->priv_data, "x264opts","crf=26:vbv-maxrate=728:vbv-bufsize=364:keyint=15",0);
	}
	
	//audio
	AVStream *sa=NULL;//=out.add_stream(test.get_audio_codecID());
	AVCodecContext *ca;
	if(sa)
	{
		ca=sa->codec;
		ca->flags |= CODEC_FLAG_GLOBAL_HEADER;
		printf("surce num %d den %d\n",test.get_audio_timebase().num,test.get_audio_timebase().den);
		printf("dest num %d den %d\n",sa->time_base.num,sa->time_base.den);
		//check_sample_fmt(ca->codec, test.get_audio_stream()->codec->sample_fmt);
		ca->sample_rate=test.get_audio_stream()->codec->sample_rate;
		ca->sample_fmt=test.get_audio_stream()->codec->sample_fmt;
		ca->channels=test.get_audio_stream()->codec->channels;
		ca->bit_rate=test.get_audio_stream()->codec->bit_rate;
	}

	out.write_header();
	char sdp[2000];
	AVFormatContext *ctx=out.get_ctx();
	av_sdp_create(&ctx,1,sdp,2000);
	printf("RTP:SDP\n%s",sdp);
	while(!test.get_frame(&pkt))
	{
		if(pkt.stream_index==test.get_video_index())
		{
			//printf("video : pts %10lld dts %10lld\n",pkt.pts,pkt.dts);
			pkt.pts=av_rescale_q(pkt.pts,test.get_video_timebase(),sv->time_base);
			pkt.stream_index=sv->index;
		}
		else if(0&&pkt.stream_index==test.get_audio_index())
		{
			//printf("audio : pts %10lld dts %10lld\n",pkt.pts,pkt.dts);
			pkt.pts=av_rescale_q(pkt.pts,ca->time_base,sa->time_base);
			pkt.stream_index=sa->index;
		}
		else
			{av_free_packet(&pkt);continue; }
		pkt.dts=pkt.pts;
		out.write_frame(&pkt);
		//usleep(1000000/test.get_video_fps());
		//printf("write\n");
		av_free_packet(&pkt);
	}
	out.write_tailer();
	return 0;
}
#endif