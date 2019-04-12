#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

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

unsigned short calculeazaCRC(MK p){
    return crc16_ccitt(&p, sizeof(p)-4);
}

int main(int argc, char** argv) {
    msg raspuns, ack;
    int sequence = 0;
    init(HOST, PORT);

    if (recv_message(&raspuns) < 0) {
        perror("Receive message");
        return -1;
    }

    MK si_get;    

    memcpy(&si_get,&raspuns.payload,sizeof(si_get));

    if(calculeazaCRC(si_get)==si_get.CHECK){  
    	strcpy(ack.payload,"ACK!");
    	ack.len=strlen(ack.payload);
        send_message(&ack);
    }else{
	strcpy(ack.payload,"NAK!");
    	ack.len=strlen(ack.payload);
        send_message(&ack);
    }

    FILE * f;
    while(1){

	recv_message(&raspuns);
	memcpy(&si_get,&raspuns.payload, sizeof(si_get));
	if(si_get.TYPE == 'S'){	
   	    send_message(&ack);
	}

	if(si_get.TYPE == 'F'){	
	    f = fopen(si_get.DATA, "wb");
	}


	if(si_get.TYPE == 'D'){
	   fwrite(si_get.DATA,sizeof(char), si_get.LEN, f);
	}

	if(si_get.TYPE == 'Z'){	
	   fclose(f);
	}

	if(si_get.TYPE == 'B'){
	   break;
	}

	ack.len = sequence;

	memcpy(&raspuns,&ack,sizeof(si_get));
	send_message(&raspuns);	
	sequence++;
    }


	return 0;
}
