#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>

#define SENDBLOCKSIZE 128

static  struct termios termold[7],termnew[7];

void*  device2_thread(void* arg) ;
void*  device3_thread(void* arg) ;
void*  device4_thread(void* arg) ;
void*  device5_thread(void* arg) ;
void*  device6_thread(void* arg) ;
static 	int retlen[7]={0} ;
static 	int txlen[7]={0};

static  long unsigned int rxtotal[7]={0};
static  long unsigned int txtotal[7]={0};
static  long unsigned int faillen[7]={0};
static  int fd[7]={0};
//uart configration
void uart_config(void)
{
	char i =0 ;
	for(i=1;i<7;i++){
	  tcgetattr(fd[i],&termold[i]);
	  tcgetattr(fd[i],&termnew[i]);
	  cfmakeraw(&termnew[i]);
	  cfsetspeed(&termnew[i],B115200);  //{115200,460800,921600}
	  tcsetattr(fd[i],TCSANOW,&termnew[i]);
	}
}
//check buf 
int  check_string(char *buf)
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
//print
void print_result(void)
{
	double fail=0.0;
	double all=0.0;
	char i=0;
	printf("\n**************************B115200**************************\n");
	for(i=1;i<7;i++){
	  fail = faillen[i];  
	  all = rxtotal[i]/10; //pack
	  printf("==ttyO%d:-Tx %ld ==\n",i,txtotal[i]/10) ;
	  printf("         Rx %ld     success:%ld    fail:%ld    %1.1f %==\n",rxtotal[i]/10,(rxtotal[i]-faillen[i])/10,faillen[i],(1-fail/all)*100) ;
	}
	  printf("***********************************************************\n");
}
int main(int argc,char* argv[])
{
    char txbuf[10]="123456789" ; //send buf 
	char rxbuf[10]={0} ; //null
	
	pthread_t  device2fp ;   //ttyO2
	pthread_t  device3fp ;   //ttyO3 
	pthread_t  device4fp ;   //ttyO4 
	pthread_t  device5fp ;   //ttyS1 
	pthread_t  device6fp ;   //ttyS2 
    /*
	if(argc<2){
		printf("--argc incorrect--\n");
		return -1 ;
	}
	*/
	/*
	 *open Compart
	 */
	fd[1]=open("/dev/ttyO1",O_RDWR) ;
	fd[2]=open("/dev/ttyO2",O_RDWR) ;
	fd[3]=open("/dev/ttyO3",O_RDWR) ;
	fd[4]=open("/dev/ttyO4",O_RDWR) ;
	fd[5]=open("/dev/ttyS1",O_RDWR) ;
	fd[6]=open("/dev/ttyS2",O_RDWR) ;
	
	if((fd[1]==-1)||(fd[2]==-1)||(fd[3]==-1)||(fd[4]==-1)||(fd[5]==-1)||(fd[6]==-1)){
		printf("The ComPort open error!\n");
		return -1 ;
	}
	printf("---------open ComPart %s is success--------\r\n",argv[1]) ;
	/*
	 * setting
	 */
    uart_config() ;
    //create pthread
	pthread_create(&device2fp,  NULL,  device2_thread,  (void*)fd[2]);
	//rs485.3<-->rs485.4
	pthread_create(&device3fp,  NULL,  device3_thread,  (void*)fd[3]);
	pthread_create(&device4fp,  NULL,  device4_thread,  (void*)fd[4]);
	//rs485.5<-->rs485.6  rs232.1<-->rs232.2
	pthread_create(&device5fp,  NULL,  device5_thread,  (void*)fd[5]);
	pthread_create(&device6fp,  NULL,  device6_thread,  (void*)fd[6]);

	while(1){
		txlen[1] = write(fd[1],txbuf,sizeof(txbuf)) ;
		txtotal[1]+=txlen[1] ;

		memset(rxbuf,0,sizeof(rxbuf));        //clear rxdate buf 
		retlen[1] = read(fd[1],rxbuf,sizeof(rxbuf)) ;  //send after receive
		
		if(check_string(rxbuf)){                 //ok
	      rxtotal[1]+=retlen[1];
	    }else{
	      faillen[1]++ ;	
	    }

		//print per 5000bits
		if(!(txtotal[1]%5000)){
           print_result() ;  //show
		}
	}
    //close fd
	tcsetattr(fd[1],TCSANOW,&termold[1]);
	tcsetattr(fd[2],TCSANOW,&termold[2]);
	tcsetattr(fd[3],TCSANOW,&termold[3]);
	tcsetattr(fd[4],TCSANOW,&termold[4]);
	tcsetattr(fd[5],TCSANOW,&termold[5]);
	tcsetattr(fd[6],TCSANOW,&termold[6]);
	close(fd[1]);
	close(fd[2]);
	close(fd[3]);
	close(fd[4]);
	close(fd[5]);
	close(fd[6]);
}
//thread for uart2
void*  device2_thread(void* arg) 
{
	int device2_fd = (int)arg ; //get fd
	
    char device2_txbuf[10]="123456789";
	char device2_rxbuf[10]={0} ; 
	
	while(1){
	//clear rxdate buf  ===read===
	memset(device2_rxbuf,0,sizeof(device2_rxbuf));
	retlen[2]= read(device2_fd,device2_rxbuf,sizeof(device2_rxbuf));
	
	if(check_string(device2_rxbuf)){                 //ok
	  rxtotal[2]+=retlen[2];
	}else{
      faillen[2]++ ;	
	}
	//===write===
	txlen[2]= write(device2_fd,device2_txbuf,sizeof(device2_txbuf));	
	txtotal[2]+=txlen[2] ;
	
	}
}
//thread for uart3  send_first
void*  device3_thread(void* arg) 
{
	int device3_fd = (int)arg ; //get fd
	
    char device3_txbuf[10]="123456789";
	char device3_rxbuf[10]={0} ; 
	
	while(1){
		txlen[3] = write(device3_fd,device3_txbuf,sizeof(device3_txbuf)) ;
		//txtotal[3]+=txlen[3] ;

		memset(device3_rxbuf,0,sizeof(device3_rxbuf));        //clear rxdate buf 
		retlen[3] = read(device3_fd,device3_rxbuf,sizeof(device3_rxbuf)) ;  //send after receive
		
		if(check_string(device3_rxbuf)){                 //ok
	      rxtotal[3]+=retlen[3];
	    }else{
	      faillen[3]++ ;	
	    }
	    txtotal[3]+=txlen[3] ;  //因为read会有阻塞 所以等接收到后再加  避免发送回比接收多10
	}
}
//thread for uart4  receive_first
void*  device4_thread(void* arg) 
{
	int device4_fd = (int)arg ; //get fd
	
    char device4_txbuf[10]="123456789";
	char device4_rxbuf[10]={0} ; 
	
	while(1){
	//clear rxdate buf  ===read===
	memset(device4_rxbuf,0,sizeof(device4_rxbuf));
	retlen[4]= read(device4_fd,device4_rxbuf,sizeof(device4_rxbuf));
	
	if(check_string(device4_rxbuf)){                 //ok
	  rxtotal[4]+=retlen[4];
	}else{
      faillen[4]++ ;	
	}
	//===write===
	txlen[4]= write(device4_fd,device4_txbuf,sizeof(device4_txbuf));	
	txtotal[4]+=txlen[4] ;
	
	}
}
//thread for uart5  send_first
void*  device5_thread(void* arg) 
{
	int device5_fd = (int)arg ; //get fd
	
    char device5_txbuf[10]="123456789";
	char device5_rxbuf[10]={0} ; 
	
	while(1){
		txlen[5] = write(device5_fd,device5_txbuf,sizeof(device5_txbuf)) ;
		//txtotal[5]+=txlen[5] ;

		memset(device5_rxbuf,0,sizeof(device5_rxbuf));        //clear rxdate buf 
		retlen[5] = read(device5_fd,device5_rxbuf,sizeof(device5_rxbuf)) ;  //send after receive
		
		if(check_string(device5_rxbuf)){                 //ok
	      rxtotal[5]+=retlen[5];
	    }else{
	      faillen[5]++ ;	
	    }
	    txtotal[5]+=txlen[5] ;  //因为read会有阻塞 所以等接收到后再加  避免发送回比接收多10
	}
}
//thread for uart6  receive_first
void*  device6_thread(void* arg) 
{
	int device6_fd = (int)arg ; //get fd
	
    char device6_txbuf[10]="123456789";
	char device6_rxbuf[10]={0} ; 
	
	while(1){
	//clear rxdate buf  ===read===
	memset(device6_rxbuf,0,sizeof(device6_rxbuf));
	retlen[6]= read(device6_fd,device6_rxbuf,sizeof(device6_rxbuf));
	
	if(check_string(device6_rxbuf)){                 //ok
	  rxtotal[6]+=retlen[6];
	}else{
      faillen[6]++ ;	
	}
	//===write===
	txlen[6]= write(device6_fd,device6_txbuf,sizeof(device6_txbuf));	
	txtotal[6]+=txlen[6] ;
	
	}
}