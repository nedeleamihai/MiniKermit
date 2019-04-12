#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

typedef struct {
	char SOH;
	char LEN;
	char SEQ;
	char TYPE;
	char DATA[250];
	unsigned short CHECK;
	char MARK;	

} MK;

typedef struct {
	char MAXL;
	char TIME;
	char NPAD;
	char PADC;
	char EOL;
	char QCTL;
	char QBIN;
	char CHKT;
	char REPT;
	char CAPA;
	char R;

}dataSI;

MK send_si(int sNo){
    MK si;
    si.SOH=sNo; 
    si.LEN=0x05; 
    si.SEQ=0x00;
    si.TYPE='S';
    dataSI dsi;
        dsi.MAXL=250;
	dsi.TIME=5;
	dsi.NPAD=0x00;
	dsi.PADC=0x00;
	dsi.EOL=0x0D;
	dsi.QCTL=0x00;
	dsi.QBIN=0x00;
	dsi.CHKT=0x00;
	dsi.REPT=0x00;
	dsi.CAPA=0x00;
	dsi.R=0x00;    
	char *data;
	data = malloc(sizeof(dataSI));
	memcpy(data,&dsi,sizeof(dsi));  
   	strcpy(si.DATA,data);  
   	si.CHECK=crc16_ccitt(&si,sizeof(si)-4);   
	si.MARK=0x0D;   

	return si;
}

MK send_sh(char* string, int sNo){
        MK sh;
        sh.SOH= sNo;
        sh.LEN=strlen(string); 
    	sh.SEQ=0x00;
    	sh.TYPE='F';
	memcpy(sh.DATA,string,strlen(string));
  	sh.CHECK=crc16_ccitt(&sh,sizeof(sh)-4); 
	sh.MARK=0x0D;
        return sh;  
}

int main(int argc, char** argv) {
    msg t;   
    int sequence = 0;
    init(HOST, PORT);
    
    MK si;
    si=send_si(sequence);  
    
    memcpy(t.payload, &si, sizeof(si));
    t.len=strlen(t.payload);  
    printf("Sending INIT...\n");
    send_message(&t);    
   
    msg *raspuns = receive_message_timeout(5000);   
   
    if(raspuns==NULL){
       fflush(stdin);
       printf("timeout");
    }else{
       fflush(stdin);
       printf("PRIMIT %s\n", raspuns->payload);
    } 

    int i; MK sh;

    for(i=1;i<=argc;i++){
         
          sh = send_sh(argv[i],sequence);
 	  memcpy(t.payload, &sh, sizeof(sh));
          t.len=strlen(t.payload);  
          printf("Sending SH...\n");
          send_message(&t);
          
          raspuns = receive_message_timeout(5000);  
          
          while(raspuns==NULL){
                   send_message(&t);
                   raspuns = receive_message_timeout(5000); 
          } 

          FILE *f = fopen(argv[i],"r"); 

          while(feof(f)){
               char buf[250];
	       fread(buf,sizeof(char),250,f);
               memcpy(sh.DATA,buf,250);
               sh.TYPE='F';
	       sh.SEQ = sequence;
               
               memcpy(t.payload, &sh, sizeof(sh));
               t.len=strlen(t.payload); 
	       send_message(&t);

               raspuns = receive_message_timeout(5000);   
   
               while(raspuns==NULL){
                   send_message(&t);
                   raspuns = receive_message_timeout(5000); 
               } 
	       sequence++;
          }

          sh.TYPE='Z';
          memcpy(t.payload, &sh, sizeof(sh));
          t.len=strlen(t.payload); 
	  send_message(&t);

          raspuns = receive_message_timeout(5000);      
          
          while(raspuns==NULL){
               send_message(&t);
               raspuns = receive_message_timeout(5000); 
          } 
         
	  if(i==argc){
               sh.TYPE='B';
               memcpy(t.payload, &sh, sizeof(sh));
               t.len=strlen(t.payload); 
	       send_message(&t);

               raspuns = receive_message_timeout(5000);   
   
          
               while(raspuns==NULL){
                   send_message(&t);
                   raspuns = receive_message_timeout(5000); 
               } 
          }   

   
    }

    

    
    return 0;
}
