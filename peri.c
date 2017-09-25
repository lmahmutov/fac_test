#include "peri.h"
#include "product_test.h"

//Operation the red of RGB
int test_rgb_r(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fd = 0;
	fd = open(SYSFS_RGB_R_BRIGHTNESS, O_WRONLY);
	if(fd == -1){
		perror("test_rgb_r");
		return -1;
	}
	if(_argc == 1 && _argv && _argv[1] && atoi(_argv[1]) >= 0 && atoi(_argv[1]) <= 100)
		write(fd, _argv[1], strlen(_argv[1]));
	else		
		write(fd, "100", strlen("100"));
	close(fd);
}

//Operation the green of RGB
int test_rgb_g(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fd = 0;
	fd = open(SYSFS_RGB_G_BRIGHTNESS, O_WRONLY);
	if(fd == -1){
		perror("test_rgb_g");
		return -1;
	}
	if(_argc == 1 && _argv && _argv[1] && atoi(_argv[1]) >= 0 && atoi(_argv[1]) <= 100)
		write(fd, _argv[1], strlen(_argv[1]));
	else		
		write(fd, "100", strlen("100"));
	close(fd);
	return 0;
}


//Operation the blue of RGB
int test_rgb_b(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fd = 0;
	fd = open(SYSFS_RGB_B_BRIGHTNESS, O_WRONLY);
	if(fd == -1){
		perror("test_rgb_B");
		return -1;
	}
	if(_argc == 1 && _argv && _argv[1] && atoi(_argv[1]) >= 0 && atoi(_argv[1]) <= 100)
		write(fd, _argv[1], strlen(_argv[1]));
	else		
		write(fd, "100", strlen("100"));
	close(fd);
	return 0;
}

//Operation the white of RGB
int test_rgb_white(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	test_rgb_r(_cmd, 0, NULL);
	test_rgb_g(_cmd, 0, NULL);
	test_rgb_b(_cmd, 0, NULL);
	return 0;
}

//Turn off the RGB
int test_rgb_close(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char *const a[2] = {NULL, "0"};
	test_rgb_r(_cmd, 1, a);
	test_rgb_g(_cmd, 1, a);
	test_rgb_b(_cmd, 1, a);
	return 0;
}

//Test the LUX for uart
int test_lumen_uart(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int value, ret = 0;
	float fvoltage;
    int fp;
	char buf[20];
    fp = open(LIGHT_PATH, O_RDONLY);
	if(fp == -1)
    {
     	perror("open light\n");
       	fvoltage = 0.0;
        return -1;
    }
    read(fp, buf, sizeof(buf));
    close(fp);

	// convert to integer
	sscanf(buf, "%d", &value);
	fvoltage = 0.8 * value;
	printf("Lux:%d\n", (int)fvoltage);
	
	return 0;
}

int play_music(char *music_name, float volume)
{
	char name[1024];
	if(music_name == NULL)
		return -1;
	if(volume > 1 || volume < 0)
		volume = 0;
	sprintf(name, "gst-launch-1.0 playbin uri=file:////%s volume=%f > //tmp//music &", \
			music_name, volume);
	system(name);
	return 0;
}


//Test speaker
int test_speaker(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	play_music("//home//root//fac//1khz.wav", 0.2);
	printf("Playing OK\n");

	return 0;
}

//Test key
int test_key(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	int fdKey;
	struct input_event key;
	int t = 0;
	fdKey = open(BUTTON_PATH, O_RDONLY);
	if(fdKey == -1){
		perror("open key!\n");
		return -1;
	}
	printf("Testing key...\n");
	
	while(1){
		if(read(fdKey, &key, sizeof(key)) == sizeof(key)){
			//printf("key.value=%d, key.type=%d, key.code=%d, key.time.tv_sec=%d\n", key.value, key.type, key.code, key.time.tv_sec);
			if(key.type == EV_KEY){
				if(key.value == 1){
					printf("Press key OK\n");
				}
				else if(key.value == 0){
					printf("Relase key OK\n");
				}
			}
			if(key.type == EV_KEY & key.value == 0)
				break;
			//printf("%d", key.time.tv_sec);
		}
	}
}

//exit
int exit_test(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	exit(0);
	return 0;
}

int m_play(cmd_tbl_s *_cmd, int _argc, char *const _argv[])
{
	char *music_name = NULL;
	float volume = 0.0;
	if(_cmd == NULL)
		return 0;
	int i = 0;
	if(_argc >= 1){
		if(_argv[1] == NULL)
			return -1;
		else
			music_name = _argv[1];
		if(_argv[2] == NULL)
			volume = 0.5;
		else
			volume = atof(_argv[2]);
		play_music(music_name, volume);
		printf("Play ok\n");
		return 0;
	}
	else
		printf("The parameter is error.Usage : m_play music_name volume");
	return -1;
}


