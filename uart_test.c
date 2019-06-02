#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include<time.h>  //getSystemTimer
#include<sys/time.h>
#include <sys/timeb.h>

#include <signal.h>

static  char*   port_6[7] = {"xxx", "/dev/ttyO1","/dev/ttyO2","/dev/ttyO3","/dev/ttyO4","/dev/ttyS1","/dev/ttyS2",} ;
								
static char*   port_10[17] = {"xxx","/dev/ttyS1","/dev/ttyS2",
									"/dev/ttyS3","/dev/ttyS4",
									"/dev/ttyS5","/dev/ttyS6",
									"/dev/ttyS7","/dev/ttyS8",
									"/dev/ttyS9","/dev/ttyS10",
									"/dev/ttyS11","/dev/ttyS12",
									"/dev/ttyS13","/dev/ttyS14",
									"/dev/ttyS15","/dev/ttyS16"};

static  struct termios termold[17],termnew[17];



void*  uart1_read_thread(void* arg) ;
void*  read_thread(void* arg) ;    
void*  write_thread(void* arg) ;

int    start_port = 1 ;
int    total_ports = 0 ;
static 	int retlen[17]={0} ;
static 	int txlen[17]={0};

static  long unsigned int rxtotal[17]={0};
static  long unsigned int txtotal[17]={0};
static  long unsigned int faillen[17]={0};
static  int fd[17]={0};


void   alarm_post() ; 

//gettime from system
void getSystemTimer(void)
{
#if 0
	char *wdate[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"} ;
	time_t timep;
	struct tm *p;
	time(&timep);
	p=gmtime(&timep);
    printf("\n************************* %d:%d:%d **************************\n", p->tm_hour, p->tm_min, p->tm_sec);
#endif
    struct timeb timebuffer;  //get ms
	static long  oldsecond ;  long msecond = 0 ;
 
    static char flag=0;
	static long lastsecond = 0 ;   //old
	long timeoffset = 0 ;          //now
	struct timeval tv;
	struct timezone tz;
	
	gettimeofday(&tv,&tz);
	if(flag==0){
	  lastsecond = tv.tv_sec ;
	  flag = 1 ;
	}
	timeoffset = tv.tv_sec - lastsecond ; //get offsettime
	//ms
	ftime(&timebuffer);  
    msecond = (tv.tv_sec-oldsecond)*1000 + timebuffer.millitm ;
	oldsecond = tv.tv_sec ;
	printf("\n**************************    %d:%d:%d    ***************************\n",(timeoffset/60)/60,(timeoffset/60)%60,timeoffset%60) ;  //48.8*2
	//printf("\n************************ %d:%d:%d  %1.1fKb/S*************************\n",(timeoffset/60)/60,(timeoffset/60)%60,timeoffset%60,98000.0/msecond) ;  //48.8*2
}
//uart configration
void uart_config(char uart1,char uart2)
{
	/*
	 * setting
	 */
	char i =0 ;
	for(i=uart1;i<=uart2;i++){
		  tcgetattr(fd[i],&termold[i]);
		  tcgetattr(fd[i],&termnew[i]);
		  cfmakeraw(&termnew[i]);
		  cfsetspeed(&termnew[i],115200);  //{115200,460800,921600}
		  //设置阻塞超时时间
		  ////termnew[i].c_cc[VMIN]   =   0; //DATA_LEN 0非阻塞;                                     
		  ////termnew[i].c_cc[VTIME]  =   10;//每个单位是0.1秒  20就是2秒
		  tcsetattr(fd[i],TCSANOW,&termnew[i]);
	}
}
//check buf 
int  check_string(char *buf)
{
	char checkbuf[10]="0123456789",count = 0;
	for(count=0;count<=10;count++)
	{
		if(checkbuf[count]==buf[count]){
			
		}else{
			return 0 ;
		}
	}
	return 1 ;
}
//print
void print_result(void)
{
	long unsigned int fail=0;
	long unsigned int all=0;
	char i=0;
	double loss_pack = 0.0 ;
	//printf("\n**************************B115200**************************\n");
	getSystemTimer() ;
	for(i=start_port;i<=total_ports;i++){
	  fail = faillen[i];  
	  all = (rxtotal[i] + faillen[i]) ; //pack
	  if(faillen[i]>=all){
		  loss_pack = 1.0 ;
	  }else{
		  loss_pack = fail/all ;
	  }
	  //printf("\n==uartO%d-Tx: %ld    Rx %ld    success:%ld    fail:%ld    %1.1f %==\n",i,txtotal[i],all,rxtotal[i],faillen[i],(1.0-loss_pack)*100) ;
	  printf("\n==uartO%d-Tx: %ld    Rx %ld    success:%ld    fail:%ld ==\n",i,txtotal[i],all,rxtotal[i],faillen[i]) ;
	}
	printf("*****************************************************************\n");
	  
}
void   alarm_post() 
{
	print_result() ;  //show
	alarm(5) ;
}
void six_port_init()
{
	int i = 0 ;
	for(i=1;i<=6;i++){
		fd[i]=open(port_6[i],O_RDWR) ;
	}
	if((fd[1]==-1)||(fd[2]==-1)){
		printf("\n+++++open uart1 uart2 faill+++++\n") ;
	}else{
		printf("\n+++++open uart1 uart2 succcess+++++\n") ;
		//create pthread
		pthread_t  fpthread_read ;    
		pthread_create(&fpthread_read,  NULL,  read_thread,  (void*)2);
	}

	for(i=3;i<=total_ports;i+=2){
		if((fd[i]==-1)||(fd[i+1]==-1)){
			printf("\n+++++open uart%d uart%d faill+++++\n",i,i+1) ;
		}else{
			printf("\n+++++open uart%d uart%d succcess+++++\n",i,i+1) ;
			//rs485.3<-->rs485.4
			pthread_t  fpthread_read ;
			pthread_t  fpthread_write ; 
			pthread_create(&fpthread_read,  NULL,  read_thread,  (void*)i);
			pthread_create(&fpthread_write,  NULL, write_thread,  (void*)(i+1));
		}
	}
}
void ten_port_init()
{
	int i = 0 ;
	int flags = 0 ;
	for(i=1;i<=total_ports;i++){
		fd[i]=open(port_10[i],O_RDWR ) ;
		//flags = fcntl(fd[i],F_GETFL,0);
        //flags &= ~O_NONBLOCK;
        //fcntl(fd[i],F_SETFL,flags);
	}
	if((fd[1]==-1)||(fd[2]==-1)){
		printf("\n+++++open uart1 uart2 faill+++++\n") ;
	}else{
		printf("\n+++++open uart1 uart2 succcess+++++\n") ;
		//create pthread
		pthread_t  fpthread_read ; 
		pthread_create(&fpthread_read,  NULL,  uart1_read_thread,  (void*)2);
	}
	for(i=3;i<=total_ports;i+=2){
		if((fd[i]==-1)||(fd[i+1]==-1)){
			printf("\n+++++open uart%d uart%d faill+++++\n",i,i+1) ;
		}else{
			printf("\n+++++open uart%d uart%d succcess+++++\n",i,i+1) ;
			//rs485.3<-->rs485.4
			pthread_t  fpthread_read ;
			pthread_t  fpthread_write ; 
			pthread_create(&fpthread_read,  NULL,  read_thread,  (void*)i);
			pthread_create(&fpthread_write,  NULL, write_thread,  (void*)(i+1));
		}
	}
}
void check_for_port()
{
	int fd1 = 0 ,fd2 = 0 ;
	fd1=open(port_10[7],O_RDWR) ;
	fd2=open(port_10[8],O_RDWR) ;
	if(fd1==-1 && fd2 ==-1){
		total_ports = 6 ;
	}else{
		total_ports = 16 ;
		close(fd1) ;
		close(fd2) ;
	}
}
int test_two_port(char* port1, char* port2)
{
		static int ports_get[3] = {0} ;
	
		pthread_t  fpthread_read ;
		pthread_t  fpthread_write ; 
		
		ports_get[1] = atoi(port1) ;
		ports_get[2] = atoi(port2) ;
		fd[ports_get[1]] = open(port_10[ports_get[1]],O_RDWR) ;
		fd[ports_get[2]] = open(port_10[ports_get[2]],O_RDWR) ;

		if(fd[ports_get[1]]<0 || fd[ports_get[2]]<0)
		{
			if(fd[ports_get[1]]<0){
				printf("\n======串口 %d 打开失败======\n",ports_get[1]) ;
			}
			else{
				printf("\n======串口 %d 打开失败======\n",ports_get[2]) ;
			}
			return -1 ;
		}
		printf("\n======串口 %d %d 打开成功======\n",ports_get[1], ports_get[2]) ;
		start_port = ports_get[1] ;
		total_ports = ports_get[2] ;
		uart_config(ports_get[1],ports_get[2]) ;
	
		pthread_create(&fpthread_read,  NULL,  read_thread,  (void*)ports_get[1]);
		pthread_create(&fpthread_write,  NULL, write_thread,  (void*)ports_get[2]);
		return 0 ;

}
int main(int argc,char* argv[])
{
    static char device1_txbuf[10]="123456789";
	static char device1_rxbuf[10]={0} ; 
    static char checkbuf[10]="123456789" ;
	char i = 0 ;
    
	if(argc==3){
		if(test_two_port(argv[1], argv[2])==0){
			signal(SIGALRM, alarm_post) ;
			alarm(5) ;
			while(1)
			{
				sleep(1000) ;
			}
		}else{
			printf("\n======测试失败======\n") ;
			return-1;
		}
	}
	
	check_for_port() ;
	printf("\n======will start test on %d poart======\n",total_ports) ;
	if(total_ports==6){
		six_port_init() ;
		uart_config(1,total_ports) ;
	}else{
		ten_port_init() ;
		uart_config(1,total_ports) ;
	}
    //open fial 0~6   return
	if((fd[1]==-1) &&(fd[2]==-1)&&(fd[3]==-1)&&(fd[4]==-1)&&(fd[5]==-1)&&(fd[6]==-1)){
		printf("\n---所有端口打开失败 请检查线路---\n");
		printf("\n---所有端口打开失败 请检查线路---\n");
		return -1 ;
	}
	printf("---------open ComParts  success--------\r\n") ;
	signal(SIGALRM, alarm_post) ;
	alarm(5) ;
	while(1){
		txlen[1] = write(fd[1],device1_txbuf,sizeof(device1_txbuf)) ;
	    //sleep(1) ;
		
		memset(device1_rxbuf, 0, sizeof(device1_rxbuf));        //clear rxdate buf 
		retlen[1] = read(fd[1],device1_rxbuf,sizeof(device1_rxbuf)) ;  //send after receive
		if(strcmp(checkbuf, device1_rxbuf)==0){
			rxtotal[1]++;
		    //printf("success:   %s   len:%d---\n",device1_rxbuf,retlen[1]) ;
		}else{
			faillen[1]++ ;	
		    //printf("fail:     %s    len:%d---\n",device1_rxbuf,retlen[1]) ;
		}
		//txtotal[1]+=txlen[1] ;
		txtotal[1]++ ;
	}
    //close fd
	for(i=1;i<=total_ports;i++){
		tcsetattr(fd[i],TCSANOW,&termold[i]);
	    close(fd[i]);
	}
}

void*  uart1_read_thread(void* arg)
{
	int  paramer_in = (int)arg ;
	int  read_fd = 0 ; 
    char write_txbuf[10]="123456789";
	char read_rxbuf[10]={0} ; 
	char checkbuf[10]="123456789" ;
	//printf("开始进入  %d\n",paramer_in) ;
	read_fd = fd[paramer_in] ; //get fd
	while(1){
	//clear rxdate buf  ===read===
	memset(read_rxbuf,0,sizeof(read_rxbuf));
	retlen[paramer_in]= read(read_fd,read_rxbuf,sizeof(read_rxbuf));
	if(strcmp(checkbuf, read_rxbuf)==0){
		rxtotal[paramer_in]++;
		//printf("success:   %s   len:%d---\n",read_rxbuf,retlen[paramer_in]) ;
	}else{
		faillen[paramer_in]++ ;		
		//printf("fail:     %s    len:%d---\n",read_rxbuf,retlen[paramer_in]) ;
	}
	//===write===
	txlen[paramer_in]= write(read_fd,write_txbuf,sizeof(write_txbuf));	
	txtotal[paramer_in]++ ;
	//sleep(1) ;
	}
}
//thread for uart2
void*  read_thread(void* arg)
{
	int  paramer_in = (int)arg ;
	int  read_fd = 0 ; 
    char write_txbuf[10]="123456789";
	char read_rxbuf[10]={0} ; 
	char checkbuf[10]="123456789" ;
	//printf("线程开始进入  %d\n",paramer_in) ;
	read_fd = fd[paramer_in] ; //get fd
	while(1){
	//clear rxdate buf  ===read===
	memset(read_rxbuf,0,sizeof(read_rxbuf));
	retlen[paramer_in]= read(read_fd,read_rxbuf,sizeof(read_rxbuf));
	if(strcmp(checkbuf, read_rxbuf)==0){
		rxtotal[paramer_in]++;
		//printf("success:   %s   len:%d---\n",read_rxbuf,retlen[paramer_in]) ;
	}else{
		faillen[paramer_in]++ ;		
		//printf("fail:     %s    len:%d---\n",read_rxbuf,retlen[paramer_in]) ;
	}
	//===write===
	txlen[paramer_in]= write(read_fd,write_txbuf,sizeof(write_txbuf));	
	txtotal[paramer_in]++ ;
	}
}
//thread for uart3  send_first
void*  write_thread(void* arg)
{
	int  paramer_in = (int)arg ;
	int  write_fd = 0 ; 
    char write_txbuf[10]="123456789";
	char read_rxbuf[10]={0} ; 
	char checkbuf[10]="123456789" ;
	
	//printf("线程开始进入  %d\n",paramer_in) ;
	write_fd = fd[paramer_in] ; //get fd
	while(1){
		txlen[paramer_in] = write(write_fd,write_txbuf,sizeof(write_txbuf)) ;
		memset(read_rxbuf,0,sizeof(read_rxbuf));        //clear rxdate buf 
		retlen[paramer_in] = read(write_fd,read_rxbuf,sizeof(read_rxbuf)) ;  //send after receive
		if(strcmp(checkbuf, read_rxbuf)==0){
			rxtotal[paramer_in]++;
		}else{
			faillen[paramer_in]++ ;		
		}
	    //因为read会有阻塞 所以等接收到后再加  避免发送回比接收多
		txtotal[paramer_in]++ ;
	}
}
