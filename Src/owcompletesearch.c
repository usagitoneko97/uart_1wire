#include "owcompletesearch.h"
#include "onewireio.h"
#include "search.h"
#include "owvariable.h"


int result_reset;
uint8_t result ;
uint8_t txdata;
int state;
uint8_t sendF0_txData1[] = {SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ONE, SEND_ONE,SEND_ONE,SEND_ONE};
/*deviceAvail resetOW(){
  //uart send F0 9600baud
  //expect receive 0x10 to 0x80
  txdata = 0xF0;
  result =  OW_TxRx(&txdata);
  if(result == 0xf0)
    return DEVICE_NA;
  else  //TODO add additional condition
    return DEVICE_AVAILABLE;
}*/

void clear_OWSm(){
  state = 0;
}


int search_SM(Event event){
  uint8_t data;
  switch (event) {
    case RESET_OW:
    	  owSetUpRxIT();
    	  owUartTxDma(0xf0);
          return TRUE;

    case REPLY:	//rxIt trigger here
          if(isUartFrameError()){
            //Throw()
            return FALSE;
          }
          data = owRxCallBackData;
          if(data == 0xF0){
            //no device response
            // Throw();
            return FALSE;
          }
          // else if(data >= 0x10 && data <= 0x90){
          /*if the higher bit has response */
          else if ((data & 0xf0) != 0xf){
            //device is there
            return TRUE;
          }
          else{
            //unknown state
            return FALSE;
          }

    case SEND_F0:
          Write_SendArray(sendF0_txData1, 8);
          return TRUE;

    case BITSEARCH:
          if(_firstSearch(1)== FALSE){
            return FALSE;
          }
          while(LastDeviceFlag != TRUE){
            if(_bitSearch(1) == FALSE)
              return FALSE;
          }
    	  /*if(firstSearch())
    		  return TRUE;
    	  else
    		  return FALSE;*/
    default:
    	// dump system error
    	return FALSE;
  }
}

int completeSearch_OW(){
  switch (state) {
    case RESET:
    	setUartBaudRate(9600);
    	state = 1;  //assume that this fuc will be uart_tx callback
        search_SM(RESET_OW);
        return TRUE;
    case 1:
        if(search_SM(REPLY)){
        	volatile int i;

        	i++;
        	setUartBaudRate(115200);
        	/*Write(0);
        	Write(1);
        	Write(1);*/
        	search_SM(SEND_F0);
        	owRxCallBackData = 0;
        	i++;

        	//state = 2;  //assume that this fuc will be uart_tx callback
        	//return TRUE; //dont return to go to next case
        	if(search_SM(BITSEARCH)){
        		//success
        		state = RESET;
        		return TRUE;
        	}
        	else{
        		// throw();
        		state = RESET;
        		return FALSE; //process done
        	}
        	}
        else{
          //throw();
          return FALSE;
        }
    case 2:
        if(search_SM(BITSEARCH)){
          //success
          state = RESET;
          return TRUE;
        }
        else{
          // throw();
          state = RESET;
          return FALSE; //process done
        }
    default:
      	return FALSE;
   }


}
