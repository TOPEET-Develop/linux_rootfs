#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>
#include <unistd.h>     /*Unix标准函数定义*/
#include <sys/types.h>  /**/
#include <sys/stat.h>   /**/
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#define TITLE "485 TEST"
#define FALSE 1
#define TRUE 0
#define RS485_SEND "echo 1 > /sys/class/misc/io_ctl/gpio_state" 
#define RS485_RECEIVE "echo 0 > /sys/class/misc/io_ctl/gpio_state"

#define CTL_485_FLAGS_YES   1
#define CTL_485_FLAGS_NO    0

FILE *outfile;
void print_usage();
char buf[512] = {0};/* Recvice data buffer */
char message[104] = {0};
int  fd = 0;
static int  hex = 0;
int ctl_485_flags=CTL_485_FLAGS_NO;
int speed_arr[] = { 
	B921600, B460800, B230400, B115200, B57600, B38400, B19200, 
	B9600, B4800, B2400, B1200, B300, 
};
int name_arr[] = {
	921600, 460800, 230400, 115200, 57600, 38400,  19200,  
	9600,  4800,  2400,  1200,  300, 
};

void shownInformation(void)
{
        printf("*********************************************\n");
        printf("\t %s \t\n", TITLE);
        printf("*********************************************\n\n");
}

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios   Opt;

	tcgetattr(fd, &Opt);

	cfmakeraw(&Opt);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i]){
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0)
				perror("tcsetattr fd1");
			return;
		}
		tcflush(fd,TCIOFLUSH);
	}

        if (i == 12){
                printf("\tSorry, please set the correct baud rate!\n\n");
                print_usage(stderr, 1);
        }
}
/*
	*@brief   设置串口数据位，停止位和效验位
	*@param  fd     类型  int  打开的串口文件句柄*
	*@param  databits 类型  int 数据位   取值 为 7 或者8*
	*@param  stopbits 类型  int 停止位   取值为 1 或者2*
	*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;

	if  ( tcgetattr( fd,&options)  !=  0) {
		perror("SetupSerial 1");
		return(FALSE);
	}
	
	options.c_cflag &= ~CSIZE ;
	options.c_oflag = 0;
	switch (databits) /*设置数据位数*/ {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
	}
	
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;   /* Clear parity enable */
		options.c_iflag &= ~INPCK;     /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
		options.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;     /* Enable parity */
		options.c_cflag &= ~PARODD;   /* 转换为偶效验*/ 
		options.c_iflag |= INPCK;       /* Disnable parity checking */
		break;
	case 'S':	
	case 's':  /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
	}
	/* 设置停止位*/  
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (FALSE);
	}
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;

	options.c_cc[VTIME] = 150; // 15 seconds
	options.c_cc[VMIN] = 0;

	options.c_lflag &= ~(ECHO | ICANON);

	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0) {
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

/**
 *@breif 打开串口
 */
int OpenDev(char *Dev)
{
	int fd = open( Dev, O_RDWR );         //| O_NOCTTY | O_NDELAY
	if (-1 == fd) { /*设置数据位数*/
		perror("Can't Open Serial Port");
		return -1;
	} else{
		return fd;
	}
}


/* The name of this program */
const char * program_name;

/* Prints usage information for this program to STREAM (typically
 * stdout or stderr), and exit the program with EXIT_CODE. Does not
 * return.
 */

void print_usage (FILE *stream, int exit_code)
{
	fprintf(stream, "Usage: %s option [ dev... ] \n", program_name);
	fprintf(stream,
			"\t-h  --help     Display this usage information.\n"
			"\t-d i --device   The device ttyS[0-3] or ttyO[0-5]\n"
            	        "\t-b  --baudrate Set the baud rate you can select\n"
            		"\t               [230400, 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 300]\n"
			"\t-s  --string   Write the device data\n"
			"\t-l  --hexadecimal  recv in hexadecimal");
	exit(exit_code);
}

void *thread_receive_function(void *arg)
{
	unsigned int i = 0, j = 1,num_read;
	unsigned long recvTotal=0;
//	outfile = fopen("./tmp","w+");
	tcflush(fd, TCIOFLUSH); //清空串口缓冲区数据
	while(1){
		num_read = read(fd, buf, sizeof(buf));
		if (num_read > 0)
		{
			recvTotal+=num_read;
			
			if(hex == 1)
			{
				printf("recvTimes = %4d recvTotal=%4ld num = %2d recv = ",j++,recvTotal,num_read);
				for(i = 0;i< num_read;i++)
				{
					printf("%x",buf[i]);
				}
				printf("\n");
				memset(buf,'\0',sizeof(buf));
			}
			else
			{
//			fprintf(outfile,"receive num = %d ",i++);
				buf[num_read] = '\n';
//			fwrite(buf,num_read,1,outfile);
//			fprintf(outfile,"%s","\n");
				printf("recvTimes=%4d recvTotal = %4ld num = %2d recv = %s", j++,recvTotal,num_read, buf);
				memset(buf, '\0', sizeof(buf));
			}
			
			
			
		}
	}
}

void func_receive()
{
	int res;
	int stop;
	pthread_t a_thread;

	if(ctl_485_flags == CTL_485_FLAGS_YES)
	{	
		system(RS485_RECEIVE);
	}
	usleep(100);

	res = pthread_create(&a_thread, NULL, thread_receive_function, NULL);
	if (res != 0){
		perror("Thread creation failed");
		exit(1);
	}
    usleep(100);
	while (1){
		printf("Select 3 : Stop Receive\n");
		printf(">\n");
		scanf ("%d", &stop);
		getchar();
		if (stop == 3){
	/*		fclose(outfile);*/
			res = pthread_cancel(a_thread);

			if (res != 0){
				perror("Thread cancelation failed");
				exit(1);
			}
			break;
		}else{
			printf("Sorry! Please select [3] stop receive\n\n");
		}
	}
}

void *thread_send_function(void *arg)
{
        unsigned int num_send = 0, j = 0;
        unsigned long sendTotal=0;
        tcflush(fd, TCIOFLUSH); //清空串口缓冲区数据
        while (1){
                  j = write(fd, message, strlen(message));
				  num_send++;
				  sendTotal+=j;
				  printf("sendTimes = %4d sendTotal=%4d  num = %2d send = %s\n",num_send,sendTotal,j,message );
                  sleep(1);
        }
}

void func_send()
{
	unsigned char len;
	int stop;
	pthread_t a_thread;
	int res;

	printf("\tPlease enter the information to be sent off!\n");

	if(ctl_485_flags == CTL_485_FLAGS_YES )
	{
		system(RS485_SEND);//控制GPIO0_13引脚电平使485允许发送，禁止接收。
	}
	usleep(100);

	while (1){
		memset (message, '\0', sizeof(message));
		scanf("%[^\n]", message);
		printf("message = %s\n", message);
		len = strlen(message);
		printf("len = %d\n", len);

		if (len > 0){
			res = pthread_create(&a_thread, NULL, thread_send_function, (void *)message);
			if (res != 0){
				perror("Thread creation failed");
				exit(1);
			}

			while (1){
				printf("Information is sent......\n");
				printf("Select 3 : Stop Send\n");
				printf(">");
				scanf ("%d", &stop);
				getchar();
				if (stop == 3){
					res = pthread_cancel(a_thread);

		                        if (res != 0){
                		                perror("Thread cancelation failed");
                                		exit(1);
		                        }		

					break;
				}else{
					printf("Sorry! Please select [3] stop send\n\n");
				}
			}
			break;
		}else{
			printf("Sorry you can not enter an empty message!\n\n");
		}
	}
}

/*显示二级菜单*/
void display_two_menu()
{
	int flag_num;

	while (1){
		printf("\nSelect 1 : Send a message\n");
		printf("Select 2 : Receive messages\n");
		printf(">");
	
		scanf("%d", &flag_num);
		getchar();
		
		switch (flag_num){
		case 1 : 
			func_send();
			break;
		case 2 : 
			func_receive();
			break;
		default :
			printf("\n\nSorry! Please select the number of 1 to 2...\n");
		}
	}
}

void funct_select()
{	
	display_two_menu();
}


/*
 *@breif  main()
 */
int main(int argc, char *argv[])
{
	int   next_option, havearg = 0;
	char *device;
	int i=0,j=0;
	int nread;		/* Read the counts of data */
	int speed = 0;
	char *xmit = "6"; 	/* Default send data */ 
	int pd;
	char buf[32];

       
    setvbuf(stdout, (char *)NULL, _IONBF, 0);//disable stdio out buffer;

	const char *const short_options = "lhd:s:b:";

	const struct option long_options[] = {
		{ "help",   0, NULL, 'h'},
		{ "device", 1, NULL, 'd'},
		{ "string", 1, NULL, 's'},
		{ "baudrate", 1, NULL, 'b'},
		{"hexadecimal",0,NULL,'l'},
		{ NULL,     0, NULL, 0  }
	};

	program_name = argv[0];

	do {
		next_option = getopt_long (argc, argv, short_options, long_options, NULL);
		switch (next_option) {
		case 'h':
			print_usage (stdout, 0);
		case 'd':
			device = optarg;
			havearg = 1;
			break;
		case 'b':
			speed = atoi(optarg);
			break;
		case 's':
			xmit = optarg;
			havearg = 1;
			break;
		case 'l':
			hex = 1;
			havearg = 1;
			break;
		case -1:
			if (havearg)  
				break;
		case '?':
			print_usage (stderr, 1);
			default:
			abort ();
		}
	}while(next_option != -1);

	sleep(1);
	fd = OpenDev(device);

	shownInformation();

	if (fd > 0) {
		set_speed(fd, speed);
	} else {
		fprintf(stderr, "Error opening %s: %s\n", device, strerror(errno));
		exit(1);
	}

	if (set_Parity(fd,8,1,'N')== FALSE) {
		fprintf(stderr, "Set Parity Error\n");
		close(fd);
		exit(1);
	}

	funct_select();

	close(fd);
	exit(0);
}

