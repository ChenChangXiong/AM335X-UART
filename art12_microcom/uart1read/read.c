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
#include <stdio.h>
static  char*   port_6[7] = {"xxx", "/dev/ttyO1","/dev/ttyO2","/dev/ttyO3","/dev/ttyO4","/dev/ttyS1","/dev/ttyS2",} ;
								
static char*   port_10[17] = {"xxx","/dev/ttyS1","/dev/ttyS2",
									"/dev/ttyS3","/dev/ttyS4",
									"/dev/ttyS5","/dev/ttyS6",
									"/dev/ttyS7","/dev/ttyS8",
									"/dev/ttyS9","/dev/ttyS10",
									"/dev/ttyS1","/dev/ttyS2",
									"/dev/ttyS13","/dev/ttyS14",
									"/dev/ttyS15","/dev/ttyS16"};

static  struct termios termold[17],termnew[17];



void*  read_thread(void* arg) ;    

int    total_ports = 0 ;
static 	int retlen[17]={0} ;
static 	int txlen[17]={0};

static  long unsigned int rxtotal[17]={0};
static  long unsigned int txtotal[17]={0};
static  long unsigned int faillen[17]={0};
static  int fd[17]={0};

void   stop_signal() ;  //CTRL + C
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
	
	printf("\n************************ %d:%d:%d  %1.1fKb/S*************************\n",(timeoffset/60)/60,(timeoffset/60)%60,timeoffset%60,98000.0/msecond) ;  //48.8*2
}
//uart configration
void uart_config(char uart1,char uart2)
{

	  tcgetattr(fd[12],&termold[12]);
	  tcgetattr(fd[12],&termnew[12]);
	  cfmakeraw(&termnew[12]);
	  cfsetspeed(&termnew[12],115200);  //{115200,460800,921600}
	  tcsetattr(fd[12],TCSANOW,&termnew[12]);
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
    for(i=11;i<=12;i++)
	{
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

int main(int argc,char* argv[])
{
    static char device12_txbuf[10]="123456789";;
	static char device12_rxbuf[10]={0} ; 
    static char checkbuf[10]="123456789" ;
	
	unsigned long int count = 0 ;
	char i = 0 ;

	fd[12]=open(port_10[12],O_RDWR | O_NOCTTY) ;

	if(fd[12]==-1)
	{
		printf("---------open ComParts  fail--------\r\n") ;
		exit(-1) ;
	}
	uart_config(11,12) ;

	printf("---------open ComParts  success--------\r\n") ;
		//CTRL + C
	signal(SIGINT,stop_signal) ;
	
	signal(SIGALRM, alarm_post) ;
	alarm(5) ;
	while(1){
		retlen[12] = read(fd[12],device12_rxbuf,sizeof(device12_rxbuf)) ;  //send after receive
		if(strcmp(checkbuf, device12_rxbuf)==0){
			rxtotal[12]++;
		    printf("success:   %s   len:%d---\n",device12_rxbuf,retlen[12]) ;
		}else{
			faillen[12]++ ;	
		    printf("fail:     %s    len:%d---\n",device12_rxbuf,retlen[12]) ;
		}
		txtotal[12]++ ;
	}

	tcsetattr(fd[12],TCSANOW,&termold[12]);
	close(fd[12]);

}

void  stop_signal()
{
	printf("---The application will stop---\n") ; 
	tcsetattr(fd[12],TCSANOW,&termold[12]);
	close(fd[12]);
	exit(0) ;
}

