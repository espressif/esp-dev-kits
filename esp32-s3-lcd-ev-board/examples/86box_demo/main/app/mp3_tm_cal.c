#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "driver/i2s.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "mp3dec.h"

#define SEEK_END 		2
#define ID3V1_size 		128

static const char *TAG = "mp3_cal";


const int SamplingrateTable[3][3]=
{
	{44100,22050,11025},
	{48000,24000,120000},
	{32000,16000,8000}
};
const char* ChannelDescrip[4]=
{
	"stereo",
	"Mixed stereo",
	"Two channel",
	"Mono"
};

const int BitrateTable[6][15]= { 
		{-1,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
		{-1,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
		{-1,32,40,48,56,64,80,96,112,128,160,192,224,256,320},

		{-1,32,64,96,128,160,192,2324,256,288,320,352,384,416,448},
		{-1,32,48,56,64,80,96,112,128,160,192,224,256,320,384},
		{-1,8,16,24,32,64,80,56,64,128,160,112,128,256,320}
}; //kbps (-1) means :free

static int GetBitRate(int bRateIndex,int LayerDescript, int Version)
{
	int i,j = 0; 
	if (Version==3 && LayerDescript==3 )j=0; 
	else if (Version==3 && LayerDescript==2 )j=1;
	else if (Version==3 && LayerDescript==1 )j=2; 
	else if (Version==2 && LayerDescript==1 )j=3;
	else if (Version==2 && LayerDescript==2 )j=4;
	else if (Version==2 && LayerDescript==3 )j=5; 
	i=BitrateTable[j][bRateIndex];
	ESP_LOGE(TAG, "[bRateIndex:%d LayerDescript:%d Version:%d], BitrateTable:%d!", bRateIndex, LayerDescript, Version, i);
	return i;
}

int mp3_play_time_cal(char * mp3_path)
{
	FILE *fp=NULL;
	int bHasID3V2=0;
	char cVer=0;
	char cRevision=0;
	int bHasExtHeader=0;
	int ID3V2_size=0;

	char cID3V2_head[10]={0}; 
	char cID3v2Frame_head[10]={0};
	char *cID3v2Frame=NULL;
	long iID3v2Frame_length=0;

	/*##########################Symbol In Frame###################################*/ 
	long iFrameStart=0,iFrameMove=0,iFrameCount=0;
	unsigned long iFrameSize=0;

	/*##########################Symbol In ID3V1###################################*/
	char cID3V1[128]={0};

	/*##########################Symbol Above######################################*/
	unsigned char* cBuffer=NULL;
	int cBuffer_size=1024,Bitrate=0,FrameSize=0,FrameCount=0,mp3Duration=0;
	char LayerDescript=0 ,bRateIndex=0 ,bSampleRate=0,Version=0,bPadding=0,bChannelMode=0;
	long int flength=0;
	int i=0,j=0;

	/*##########################Symbol Above######################################*/ 
	if((fp=fopen(mp3_path,"rb"))==NULL){
		ESP_LOGE(TAG, "ERROR:can't open the file:%s!", mp3_path);
		return 0;
	}
	else{
		ESP_LOGI(TAG, "open the file:%s!", mp3_path);
	}
	/*##########################ID3V2 Reading#####################################*/
	fread(cID3V2_head,10,1,fp);
	if( (cID3V2_head[0]=='I'||cID3V2_head[1]=='i')
			&&(cID3V2_head[1]=='D'||cID3V2_head[2]=='d')
			&&cID3V2_head[2]=='3'){
		bHasID3V2=1;
		cVer=cID3V2_head[3];
		cRevision=cID3V2_head[4];

		if(cID3V2_head[5]&0x64==0x64){
			bHasExtHeader = 1;
		}
		if(cID3V2_head[5]&0x10==0x10){
			bHasExtHeader = 1;
		}

		ID3V2_size= (cID3V2_head[6]&0x7F) << 21
			| (cID3V2_head[7]&0x7F) << 14
			| (cID3V2_head[8]&0x7F) << 7
			| (cID3V2_head[9]&0x7F) + 10 ; 
		ESP_LOGI(TAG, "ID3V2_size:%d bytes", ID3V2_size);
		iFrameStart=--ID3V2_size;
	}
	else{
		bHasID3V2=0;
		ESP_LOGI(TAG, "There is no ID3V2*");
	}

	rewind(fp); /*改文件指针指向文件头*/
	fseek(fp, 10L, 1);

	while((ftell(fp) + 10) <= ID3V2_size){
		memset(cID3v2Frame_head,0,10); /*内存清空*/
		fread(cID3v2Frame_head,10,1,fp); /*ID3V2帧信息头读取*/

		iID3v2Frame_length =(long)(
				cID3v2Frame_head[4]*0x100000000
				+cID3v2Frame_head[5]*0x10000
				+cID3v2Frame_head[6]*0x100
				+cID3v2Frame_head[7]); /*计算帧的大小.*/
				
		//ESP_LOGI(TAG, "iID3v2Frame_length:%d bytes", iID3v2Frame_length);

		if (cID3v2Frame_head[0]=='/0'
				&& cID3v2Frame_head[1]=='/0'
				&& cID3v2Frame_head[2]=='/0'
				&& cID3v2Frame_head[3]=='/0')
			break;
	}

	/*(mp3_FrameHeader) Reading*/
	fseek(fp, 0L, SEEK_END); //移文件指针到文件尾
	flength = ftell(fp); //文件长 

	rewind(fp);
	fseek(fp,10 + ID3V2_size, 1);
	while((ftell(fp) + 4) < flength){

		if (ftell(fp) + 1024 <= flength){
			cBuffer_size = 1024; 
		}
		else{ 
			cBuffer_size= flength - ftell(fp);
		} 

		cBuffer = malloc(cBuffer_size); 
		if (cBuffer==NULL){ // 检测动态内存分配是否已经成功 
			if (fp!=NULL){
				fclose(fp);
			} 
			ESP_LOGE(TAG, "ERROR:Memory Exhausted!"); 
			exit(1);
		} 
		memset(cBuffer,0,cBuffer_size); 
		fread(cBuffer,cBuffer_size,1,fp); 

		for(i=0; i < (cBuffer_size - 4); i++){ 
			/*Mp3帧头(FRAMEHEADER)格式如下：
			AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM 
			Part CC IN 'AAABBCCD' ,Layer description '00' - reserved*/ 
			LayerDescript = (cBuffer[i+1] & 0x6)>>1; 

			/*Part EEEE in 'EEEEFFGH', But '1111' is Bad Bitrate index, 
			"bad" means that this is not an allowed value*/ 
			bRateIndex = cBuffer[i+2] /0x10 ; // EEEEXXXX!='1111' 

			/*Part FF IN 'EEEEFFGH' ,'11' reserved*/ 
			bSampleRate = (cBuffer[i+2] & 0xA) >>2 ; 

			if (cBuffer[i] ==0xFF 
					&& cBuffer[i+1] > 0xE0 
					&& bRateIndex != 0xF 
					&& LayerDescript != 0x0 
					&& bSampleRate < 0x3 ){ 

				Version = (cBuffer[i+1] & 0x18)>>3; 
				bPadding=(cBuffer[i+1] & 0x2)>>1; 
				Bitrate = GetBitRate(bRateIndex, LayerDescript, Version); 
				bChannelMode=(cBuffer[i+3] & 0xC0)>>6; 

				if (bRateIndex != 0) { 
					switch (Version) 
					{ 
						case 0: 
							ESP_LOGI(TAG, "<%s, %d>MPEG 2.5 层 %d, %dHz %s",__FILE__, __LINE__, 4-LayerDescript, 
									SamplingrateTable[bSampleRate][2],ChannelDescrip[bChannelMode]); 

							FrameSize = ((72 * Bitrate*1000) / SamplingrateTable[bSampleRate][2]) + bPadding; 
							FrameCount=(int)((flength-(ftell(fp)-cBuffer_size+i)) /FrameSize); 
							mp3Duration=(int)(FrameCount*0.026); //每帧的播放时间：无论帧长是多少，每帧的播放时间都是26ms； 
							ESP_LOGI(TAG, "CBR:-> %dKbit %d帧",Bitrate,FrameCount); 
							ESP_LOGI(TAG, "长度:%d 秒",mp3Duration); 
							break;

						case 2: 
							ESP_LOGI(TAG, "MPEG 2.0 层 %d, %dHz %s", 4-LayerDescript, 
									SamplingrateTable[bSampleRate][1],ChannelDescrip[bChannelMode]); 

							FrameSize = ((72 * Bitrate*1000) / SamplingrateTable[bSampleRate][1]) + bPadding;
							FrameCount=(int)((flength-(ftell(fp)-cBuffer_size+i)) /FrameSize);
							mp3Duration=(int)(FrameCount*0.026); //每帧的播放时间：无论帧长是多少，每帧的播放时间都是26ms；
							ESP_LOGI(TAG, "CBR:-> %dKbit %d帧", Bitrate,FrameCount);
							ESP_LOGI(TAG, "长度:%d 秒",  mp3Duration);
							break;

						case 3: 
							//帧长度（字节） = (( 每帧采样数/ 8 * 比特率 ) / 采样频率 ) + 填充 * 4
							ESP_LOGI(TAG, "MPEG 1.0 层 %d, %dHz %s", 4-LayerDescript,
									SamplingrateTable[bSampleRate][0],ChannelDescrip[bChannelMode]); 

							FrameSize = (((1152/8) * Bitrate*1000) / SamplingrateTable[bSampleRate][0]) + bPadding;
							FrameCount=(int)((flength-(ftell(fp)-cBuffer_size+i)) /FrameSize);
							//每帧持续时间(毫秒) = 每帧采样数 / 采样频率 * 1000
							//mp3Duration=(int)(FrameCount*0.026); //每帧的播放时间：无论帧长是多少，每帧的播放时间都是26ms；
							mp3Duration=(int)(FrameCount*(1152*1000/SamplingrateTable[bSampleRate][0]))/1000; //每帧的播放时间：无论帧长是多少，每帧的播放时间都是26ms；

							ESP_LOGI(TAG, "CBR:->%dKbit %dFrame, Size:%d byte, frame:%d ms, total:%dkb", \
										Bitrate,FrameCount, FrameSize, (1152*1000/SamplingrateTable[bSampleRate][0]), FrameCount*FrameSize/1024);
							ESP_LOGI(TAG, "mp3Duration:%ds",mp3Duration);
							break;
					}
				}
				else{
					ESP_LOGI(TAG, "This a Free Rate MP3 File!");
				}

				cBuffer_size=-1;
				break;
			}
		}
		free(cBuffer);
		if (cBuffer_size ==(-1) ) break;
	}

	/*ID3V1 Reading*/
	fseek(fp,-128L,SEEK_END);
	fread(cID3V1,ID3V1_size ,1,fp);

	if((cID3V1[0]=='T'||cID3V1[0]=='t')
			&&(cID3V1[1]=='A'||cID3V1[1]=='a')
			&&(cID3V1[2]=='G'||cID3V1[2]=='g')){
		ESP_LOGI(TAG, "ID3V1_size:%d bytes", ID3V1_size );

		for(i=0;i<128;i++){
			putchar(cID3V1[i]);
		}
		putchar('\n');
	}
	else{
		ESP_LOGI(TAG, "There is no ID3V1");
	}
	fclose(fp);

	return mp3Duration;
}
typedef struct {
    char header[3];     /*!< Always "ID3" */
    char ver;           /*!< Version, equals to3 if ID3V2.3 */
    char revision;      /*!< Revision, should be 0 */
    char flag;          /*!< Flag byte, use Bit[7..5] only */
    char size[4];       /*!< TAG size */
} __attribute__((packed)) mp3_id3_header_v2_t;

int MP3_info(const char *path)
{
    FILE *fp = NULL;
    esp_err_t ret = ESP_OK;
    uint8_t *output = NULL;
    uint8_t *read_buf = NULL;
    MP3FrameInfo frame_info;
    HMP3Decoder mp3_decoder = MP3InitDecoder();

	int offset, tag_len;
	uint32_t FrameSize, FrameCount, mp3Duration, flength;

    ESP_RETURN_ON_FALSE(NULL != mp3_decoder, ESP_ERR_NO_MEM,
        TAG, "Failed create MP3 decoder");

	read_buf = malloc(MAINBUF_SIZE);
	ESP_GOTO_ON_FALSE(NULL != read_buf, ESP_ERR_NO_MEM, clean_up,
		TAG, "Failed allocate read buffer");

	output = malloc(1152 * sizeof(int16_t) * 2);
	ESP_GOTO_ON_FALSE(NULL != output, ESP_ERR_NO_MEM, clean_up,
            TAG, "Failed allocate output buffer");
    
    /* Read audio file from given path */
    fp = fopen(path, "rb");
    ESP_GOTO_ON_FALSE(NULL != fp, ESP_ERR_NOT_FOUND, clean_up,
        TAG, "File \"%s\" does not exist", path);

    ESP_LOGI(TAG, "fopen %s ok, %X", path, fp);

    /* Get ID3 head */
    mp3_id3_header_v2_t tag;
    if (sizeof(mp3_id3_header_v2_t) == fread(&tag, 1, sizeof(mp3_id3_header_v2_t), fp)) {
        if (memcmp("ID3", (const void *) &tag, sizeof(tag.header)) == 0) {
            tag_len =
                ((tag.size[0] & 0x7F) << 21) +
                ((tag.size[1] & 0x7F) << 14) +
                ((tag.size[2] & 0x7F) << 7) +
                ((tag.size[3] & 0x7F) << 0);
            fseek(fp, tag_len - sizeof(mp3_id3_header_v2_t), SEEK_SET);
            ESP_LOGI(TAG, "+ taglen:%d", tag_len - sizeof(mp3_id3_header_v2_t));
        } else {
            /* Not ID3 header */
            fseek(fp, 0, SEEK_SET);
            ESP_LOGI(TAG, "Not ID3 header");
        }
    }

    /* Start MP3 decoding */
    uint32_t total_frame = 0;
    int bytes_left = 0;
    unsigned char *read_ptr = read_buf;
    do {
        /* Read `mainDataBegin` size to RAM */
        if (bytes_left < MAINBUF_SIZE) {
            memmove(read_buf, read_ptr, bytes_left);
            size_t bytes_read = fread(read_buf + bytes_left, 1, MAINBUF_SIZE - bytes_left, fp);
            ESP_GOTO_ON_FALSE(bytes_read > 0, ESP_OK, clean_up,
                TAG, "No data read from strorage device");
            bytes_left = bytes_left + bytes_read;
            read_ptr = read_buf;
        }

        /* Find MP3 sync word from read buffer */
        offset = MP3FindSyncWord(read_buf, MAINBUF_SIZE);
        if (offset >= 0) {
            read_ptr += offset;         /*!< Data start point */
            bytes_left -= offset;       /*!< In buffer */

			total_frame++;
			if(total_frame == 1){
           	 	int mp3_dec_err = MP3Decode(mp3_decoder, &read_ptr, &bytes_left, (int16_t *) output, 0);
            	ESP_GOTO_ON_FALSE(ERR_MP3_NONE == mp3_dec_err, ESP_FAIL, clean_up,
                	TAG, "Can't decode MP3 frame");

            	/* Get MP3 frame info and configure I2S clock */
            	MP3GetLastFrameInfo(mp3_decoder, &frame_info);
		   }
        } else {
            /* Sync word not found in frame. Try to read next frame */
            ESP_LOGE(TAG, "MP3 sync word not found");
            bytes_left = 0;
            continue;
        }
    }while (vTaskDelay(1), NULL != path);

clean_up:

	fseek(fp, 0L, SEEK_END); //移文件指针到文件尾
	flength = ftell(fp); //文件长 

	FrameSize = (frame_info.outputSamps * frame_info.bitrate) / frame_info.samprate/8;// + bPadding;
	#if 0
	FrameCount=(int)((flength - offset) /FrameSize);
	#else
	FrameCount = total_frame;
	#endif
	mp3Duration = (FrameCount * frame_info.outputSamps / frame_info.samprate);

	printf("\r\nBitrate %dKbps", frame_info.bitrate/1000);
	printf("\r\nSamprate %dHz", frame_info.samprate);
	printf("\r\nBitsPerSample %db", frame_info.bitsPerSample);
	printf("\r\nnChans %d", frame_info.nChans);
	printf("\r\nLayer %d", frame_info.layer);
	printf("\r\nVersion %d", frame_info.version);
	printf("\r\nOutputSamps %d\r\n", frame_info.outputSamps);

	printf("%s, FrameSize:%d FrameCount:%d, mp3Duration:%d sec, %d\r\n", path, FrameSize, FrameCount, mp3Duration, flength);

    /* Clean up resources */
    if (NULL != mp3_decoder)    MP3FreeDecoder(mp3_decoder);

    if (NULL != read_buf)       free(read_buf);

    if (NULL != output)         free(output);

    if (NULL != fp)             fclose(fp);

    return ret;
}
