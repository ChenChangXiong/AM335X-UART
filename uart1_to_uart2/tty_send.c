#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

#define SENDBLOCKSIZE 128

static  struct termios termold,termnew;
int sendfiletocomport(int fd_com,int fd_file);
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
    char txbuf[10]="123456789" ; //send buf 
	char rxbuf[10]={0} ; //null
    /*
	if(argc<2){
		printf("--argc incorrect--\n");
		return -1 ;
	}
	*/
	/*
	 *open Compart
	 */
	if((fd_com=open("/dev/ttyO1",O_RDWR))==-1){
		printf("The ComPort open error!\n");
		return -1 ;
	}
	printf("---------open ComPart %s is success--------\r\n",argv[1]) ;
	/*
	 * setting
	 */
	tcgetattr(fd_com,&termold);
	tcgetattr(fd_com,&termnew);
	cfmakeraw(&termnew);
	cfsetspeed(&termnew,B460800);
	tcsetattr(fd_com,TCSANOW,&termnew);
     
	while(1){
		txlen = write(fd_com,txbuf,sizeof(txbuf)) ;
		txtotal+=txlen ;

		memset(rxbuf,0,sizeof(rxbuf));        //clear rxdate buf 
		retlen = read(fd_com,rxbuf,sizeof(rxbuf)) ;  //send after receive
		
		if(check_string(rxbuf)){                 //ok
	      rxtotal+=retlen;
	    }else{
	      faillen++ ;	
	      printf("\n****fail receive： %d****\n",faillen) ;
	    }

		//print per 1000bits
		if(!(txtotal%8000)){
		  printf("-Tx total:%ld-",txtotal) ;
		  printf("\n-Rx total:%ld %s-\n",rxtotal,rxbuf) ;
		}
	}
	//retlen=sendfiletocomport(fd_com,fd_file);
    //close(fd_file) ;
	tcsetattr(fd_com,TCSANOW,&termold);
	close(fd_com);
}
int sendfiletocomport(int fd_com,int fd_file)
{
	int retlen=1;
	int filelen=0;
	char buf[SENDBLOCKSIZE+4];
	while(retlen>0){
		if((retlen=read(fd_file,buf,SENDBLOCKSIZE))>0){
			write(fd_com,buf,retlen);
			filelen+=retlen;
		}
	}
	return filelen;
}

