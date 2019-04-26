#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

#define READBLOCKSIZE  11//256
#define READDATALENGTH 2048

struct termios termold,termnew;
int readsize;
//check buf
int check_string(char *buf)
{
	char checkbuf[10]="123456789",count = 0;
	for(count=0;count<10;count++)
	{
		if(checkbuf[count]==buf[count]){
			
		}else{
			return 0 ;
		}
	}
	return 1 ;
}
int main(int argc,char* argv[])
{
    int fd_com;
	int retlen,txlen;
	long int rxtotal=0,txtotal=0,faillen=0;
	char txbuf[10]="123456789";//"abcdefghi\n";
	char rxbuf[10]={0} ; //null ;
    /*
	if(argc<2){
       printf("---argc ierror---\r\n") ;
	   return -1 ;
	}
	*/
    /*
	if(argc>1)
		readsize=atoi(argv[1]);
	else
		readsize= READDATALENGTH;
		*/
	if((fd_com=open("/dev/ttyO2",O_RDWR))==-1){
		printf("The Com port open error!\n");
	    return -1 ;
	}
    printf("------open ComPart success------\r\n") ;

	tcgetattr(fd_com,&termold);
	tcgetattr(fd_com,&termnew);
	cfsetspeed(&termnew,B460800);
	cfmakeraw(&termnew);
	tcsetattr(fd_com,TCSANOW,&termnew);

    for(;;){
    //clear rxdate buf 
	memset(rxbuf,0,sizeof(rxbuf));
	retlen= read(fd_com,rxbuf,sizeof(rxbuf));
	
	if(check_string(rxbuf)){                 //ok
	  rxtotal+=retlen;
	}else{
	  faillen++ ;	
	  printf("\n****fail receive： %d****\n",faillen) ;
	}
	
	txlen= write(fd_com,txbuf,sizeof(txbuf));	
	txtotal+=txlen ;
	//print per 1000bits
	if(!(txtotal%8000)){
	  printf("-Tx total:%ld-",txtotal) ;
	  printf("\n-Rx total:%ld %s-\n",rxtotal,rxbuf) ;
	}
	  
	}
	//usleep(100000);
	tcsetattr(fd_com,TCSANOW,&termold);

	close(fd_com);
}
