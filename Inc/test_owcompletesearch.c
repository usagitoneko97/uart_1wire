#include <stdint.h>
#include "unity.h"
#include "owcompletesearch.h"
#include "search.h"
#include "mock_onewireio.h"
#include "owvariable.h"
#include "linkedlist.h"
#include <stdlib.h>


uint8_t owRxVal;
unsigned char bitPos = 0;
uint8_t *fakeIdBits = NULL;
uint8_t *fakeCmpIdBits = NULL;
int fakeReadState = 0;

uint8_t sendF0txDataTest[] = {SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ZERO, SEND_ONE, SEND_ONE,SEND_ONE,SEND_ONE};

void init64BitId(uint8_t *id,uint8_t *cmp_id, uint8_t startBit) {
  fakeIdBits = id;
  fakeCmpIdBits = cmp_id;
  bitPos = startBit;
}

uint8_t fakeRead(int numOfCalls){
    uint8_t resultBit;
    if(!lastDeviceFlag){
      while(bitPos < 64){
        switch (fakeReadState) {
          case 0: resultBit = fakeIdBits[bitPos];
                  fakeReadState = 1;
                  return resultBit;
                  break;
          case 1: resultBit = fakeCmpIdBits[bitPos++];
                  fakeReadState = 0;
                  return resultBit;
                  break;
        }
      }
  }
}

void fakeWrite(unsigned char byte, int numOfCalls){
}

void fakewriteSendArray(uint8_t* data, int length, int numOfCalls){

}

uint8_t fake_OW_Rx(int numOfCalls){
  return owRxVal;
}


void setUp(void){
  // OW_Rx_StubWithCallback(fake_OW_Rx);
  Read_StubWithCallback(fakeRead);
  write_StubWithCallback(fakeWrite);
  writeSendArray_StubWithCallback(fakewriteSendArray);
  eventOw.data = &owdata;

}

void tearDown(void){
  fakeIdBits = NULL;
  fakeCmpIdBits = NULL;

  ((OwData*)(eventOw.data))->uartRxVal = 0;
}


/*void test_owcompletesearch_given_RX_F0_expect_DeviceNA(void)
{
  owRxVal = 0xf0;
  TEST_ASSERT_EQUAL(DEVICE_NA, resetOW());
}*/

/**
 * Expected 1byte return result
 * ---------------------------------
 * 0 1 0 0 1 0 1 1
 *       4b
 * 1 1 1 0 0 0 1 0
 *      e2
 * 0 1 1 0 0 1 1 1
 *       67fi
 */
void test_owcompletesearch_given_OW_presencePulse_RX_10_given_above_number(void){
  uint8_t fakeIdBitVal []=       {0, 1, 0, 0, 0, 1, 1, 1,  0, 1, 0, 1, 0, 0, 1, 0,  0, 1, 0, 0, 0, 1, 1, 0};
  uint8_t fakeCmpIdBitVal[] =   {0, 0, 1, 1, 1, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 1,  0, 0, 0, 1, 1, 0, 0, 1};
  init64BitId(fakeIdBitVal, fakeCmpIdBitVal, 0);

  /*Mocking*/
  setUartBaudRate_Expect(9600);
  //owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xe0;
  //owRxCallBackData = 0xE0;  //data that received in interrupt
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(FALSE);
  setUartBaudRate_Expect(115200);
  writeSendArray_Expect(sendF0txDataTest, 8);
  //OW_Tx_Expect(sendF0_txData);

  initRomSearching(&eventOw,&owdata);
  eventOw.byteLength = 1;
  owHandler(&eventOw);
  owHandler(&eventOw); //uartRxCallback will call this function

  TEST_ASSERT_EQUAL(0xe2, romDataBuffer[0][0]);
  TEST_ASSERT_EQUAL(0x4b, romDataBuffer[1][0]);
  TEST_ASSERT_EQUAL(TRUE, lastDeviceFlag);
  TEST_ASSERT_EQUAL(0, lastDiscrepancy);

}


void test_owcompletesearch_given_OW_0xf0_expect_noDevice(void){
  /*Mocking*/
  setUartBaudRate_Expect(9600);
  //owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xf0;
  //owRxCallBackData = 0xf0;  //data that received in interrupt
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(FALSE);

  initRomSearching(&eventOw,&owdata);
  eventOw.byteLength = 1;
  owHandler(&eventOw);
  TEST_ASSERT_EQUAL(FALSE, owHandler(&eventOw)); //callback of uartTx from reset
}

void test_owcompletesearch_given_OW_FrameError_expect_FALSE(void){
  setUartBaudRate_Expect(9600);
  //owSetUpRxIT_Expect();
  owUartTxDma_Expect(0xf0);
  /*uart receive it will trigger after uart tx, data will update to below*/
  ((OwData*)(eventOw.data))->uartRxVal = 0xf0;
  /*Callback from 1 wire receive*/
  isUartFrameError_ExpectAndReturn(TRUE);
  // OW_Rx_ExpectAndReturn(-1);
  initRomSearching(&eventOw,&owdata);
  eventOw.byteLength = 1;
  owHandler(&eventOw);
  TEST_ASSERT_EQUAL(FALSE, owHandler(&eventOw)); //callback of uartTx from reset
}

void xtest_resetAndVerifyOw_given_state_RESET_OW(void){
  Event eventFromDoRomSearch;
  eventFromDoRomSearch.evtType = INITIATE_COMMAND;
  owResetPrivate.state = RESET_OW;
  owSetUpRxIT_Expect(uartRxDataBuffer, 1);
  owUartTxDma_Expect(0xf0);
  owUartTxDma_Expect(0xf0);
  resetAndVerifyOw(&eventFromDoRomSearch);
}

void test_resetOw_given_state_REPLY_OW_event_UART_FRAME_ERROR_expect_systemError(void){
  /*Mock*/
  Event evt;

  TxRxCallbackList *txRxListPointerNext;
  txRxListPointerNext = malloc(sizeof(txRxListPointerNext));

  txRxListPointerNext->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointerNext;

  evt.evtType = UART_FRAME_ERROR;
  evt.data = NULL;
  owResetPrivate.state = REPLY_OW;
  systemError_Expect(evt.evtType);

  resetAndVerifyOw(&evt);
  //free(txRxListPointerNext);
}

void test_resetOw_given_state_REPLY_OW_event_UART_TIMEOUT_expect_systemError(void){
  Event evt;
  evt.evtType = UART_TIMEOUT;
  evt.data = NULL;
  owResetPrivate.state = REPLY_OW;

  TxRxCallbackList *txRxListPointerNext;
  txRxListPointerNext = malloc(sizeof(txRxListPointerNext));

  txRxListPointerNext->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointerNext;
  systemError_Expect(UART_TIMEOUT);

  resetAndVerifyOw(&evt);
}

void test_resetOw_given_state_REPLY_OW_given_uartRxVal_0xe0_event_UART_RX_SUCCESS_expect_DEVICE_AVAILABLE(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  *(txRxCpltEvData.uartRxVal) = 0xe0;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;
  TxRxCallbackList txRxNext;
  TxRxCallbackList *txRxListPointer;
  txRxListPointer = &txRxNext;
  txRxListPointer->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointer;

  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);

  //TODO test Asserts
}

void test_resetOw_given_state_REPLY_OW_given_uartRxVal_0xf0_event_UART_RX_SUCCESS_expect_DEVICE_NOT_AVAILABLE(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  *(txRxCpltEvData.uartRxVal) = 0xf0;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;

  TxRxCallbackList txRxNext;
  TxRxCallbackList *txRxListPointer;
  txRxListPointer = &txRxNext;
  txRxListPointer->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointer;

  systemError_Expect(RESET_DEVICE_NOT_AVAILABLE);
  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
}

void test_resetOw_given_state_REPLY_OW_given_uartRxVal_0xdf_event_UART_RX_SUCCESS_expect_DEVICE_UNKNOWN_ERROR(void){
  Event evt;
  TxRxCpltEvData  txRxCpltEvData;
  *(txRxCpltEvData.uartRxVal) = 0xdf;
  txRxCpltEvData.length = 1;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxCpltEvData;
  owResetPrivate.state = REPLY_OW;
  TxRxCallbackList txRxNext;
  TxRxCallbackList *txRxListPointer;
  txRxListPointer = &txRxNext;
  txRxListPointer->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointer;

  systemError_Expect(RESET_DEVICE_UNKNOWN_ERROR);
  resetAndVerifyOw(&evt);
  TEST_ASSERT_EQUAL(RESET_OW, owResetPrivate.state);
}

void xtest_romSearching_given_state_SEND_F0_expect_sendf0_expect_install_callback_toSelf(void){
  // evt->evtType =
  Event evt;
  romSearchingPrivate.state = SEND_F0;
  uartTxOw_Expect(sendF0txDataTest, 8);
  owSetUpRxIT_Expect(uartRxDataBuffer, 2);
  owUartTxDma_Expect(0xf0);
  owUartTxDma_Expect(0xf0);
  romSearching(&evt);
  TEST_ASSERT_EQUAL(ROM_SEARCHING, romSearchingPrivate.state);
  // TEST_ASSERT_NOT_NULL(txRxList.txRxCallbackFuncP);

}
//============================================================================
/*intepretSearchBit functino test*/
void test_intepretSearchBit_given_uartRx_0xff_0xfe_expect_BIT_1(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xff;
  uartRxVal_[1] = 0xfe;

  TEST_ASSERT_EQUAL(BIT_1 ,intepretSearchBit(uartRxVal_));
}

void test_intepretSearchBit_given_uartRx_0xfe_0xff_expect_BIT_0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xfe;
  uartRxVal_[1] = 0xff;

  TEST_ASSERT_EQUAL(BIT_0 ,intepretSearchBit(uartRxVal_));
}

void test_intepretSearchBit_given_uartRx_0xff_0xff_expect_BIT_0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xff;
  uartRxVal_[1] = 0xff;

  TEST_ASSERT_EQUAL(DEVICE_NOT_THERE ,intepretSearchBit(uartRxVal_));
}

void test_intepretSearchBit_given_uartRx_0xfe_0xfe_expect_BIT_0(void){
  uint8_t uartRxVal_[2];
  uartRxVal_[0] = 0xfe;
  uartRxVal_[1] = 0xfe;

  TEST_ASSERT_EQUAL(BIT_CONFLICT ,intepretSearchBit(uartRxVal_));
}
//===========================================================================
/**
 * given :idBit = 1
 *        cmpIdBit = 1
 * expect :system error ROM_SEARCH_NO_DEVICE
 */
void test_romSearching_error_given_idBit1_cmpIdBit1(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(2);
  *(txRxEvData.uartRxVal) = 0xff;
  *(txRxEvData.uartRxVal + 1) = 0xff;

  //FIXME delete this function pointer
  TxRxCallbackList *txRxListPointerNext;
  txRxListPointerNext = malloc(sizeof(txRxListPointerNext));

  txRxListPointerNext->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointerNext;

  /*create Item*/
  TxRxCallbackList *doRomSearchCallb;
  TxRxCallbackList *romSearchingCallb;
  doRomSearchCallb = malloc(sizeof(doRomSearchCallb));
  romSearchingCallb = malloc(sizeof(romSearchingCallb));
  doRomSearchCallb->txRxCallbackFuncP = doRomSearch;
  romSearchingCallb->txRxCallbackFuncP = romSearching;

  Item itemDoRomSearch;
  Item itemRomSearching;
  itemDoRomSearch.data = doRomSearchCallb;
  itemRomSearching.data = romSearchingCallb;

  LinkedList list;
  ListInit(&list);
  pushList(&list, &itemDoRomSearch);
  pushList(&list, &itemRomSearching);


  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  initGetBitRom(&romSearchingPrivate);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.idBitNumber = 1; //idBitNumber can be any value

  systemError_Expect(ROM_SEARCH_NO_DEVICE);
  romSearching(&evt);

/*  TEST_ASSERT_EQUAL(1 ,romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(FALSE ,romSearchingPrivate.bitSearchInformation.noDevice);
  TEST_ASSERT_EQUAL(0x01, romSearchingPrivate.bitSearchInformation.byteMask);*/

}
/**
 * NOTE this is the first bit search where the idBitNumber for first search is 1
 */
void test_romSearching_given_state_ROM_SEARCHING_event_UART_RX_SUCCESS_expect_idBitNumber_1_cmpIdBitNumber_0_idBitNumber_1(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(2);
  *(txRxEvData.uartRxVal) = 0xff;
  *(txRxEvData.uartRxVal + 1) = 0xfe;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  initGetBitRom(&romSearchingPrivate);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.idBitNumber = 1;

  owSetUpRxIT_Expect(uartRxDataBuffer, 2);
  owUartTxDma_Expect(0xff);
  owUartTxDma_Expect(0xff);
  romSearching(&evt);
  //TODO
  TEST_ASSERT_EQUAL(2, romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(0x01, *(romSearchingPrivate.bitSearchInformation.romNo));
  free(romSearchingPrivate.romNo);
  free(txRxEvData.uartRxVal);
  // evt.data =
  // evt

}

void test_romSearching_given_state_ROM_SEARCHING_event_UART_RX_SUCCESS_expect_idBitNumber_0_cmpIdBitNumber_1_idBitNumber_8(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(2);
  *(txRxEvData.uartRxVal) = 0xfe;
  *(txRxEvData.uartRxVal + 1) = 0xff;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  initGetBitRom(&romSearchingPrivate);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.romByteNum = 0;
  romSearchingPrivate.bitSearchInformation.idBitNumber = 8;
  romSearchingPrivate.bitSearchInformation.byteMask = 128;


  owSetUpRxIT_Expect(uartRxDataBuffer, 2);
  owUartTxDma_Expect(0xff);
  owUartTxDma_Expect(0xff);
  romSearching(&evt);
  //TODO
  TEST_ASSERT_EQUAL(9, romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.romByteNum);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.byteMask);
  TEST_ASSERT_EQUAL(0x0, *(romSearchingPrivate.bitSearchInformation.romNo));
  free(romSearchingPrivate.romNo);
  free(txRxEvData.uartRxVal);
  // evt.data =
  // evt

}
/**
 * given: the final bit
 * expect: generate event and execute callback function to parent
 */
void test_romSearching_lastBit(void){
  TxRxCpltEvData txRxEvData;
  txRxEvData.uartRxVal = malloc(2);
  *(txRxEvData.uartRxVal) = 0xff;
  *(txRxEvData.uartRxVal + 1) = 0xfe;

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
  evt.data = &txRxEvData;

  initGetBitRom(&romSearchingPrivate);  //end of SEND_F0 will call this function
  romSearchingPrivate.bitSearchInformation.romByteNum = 7;
  romSearchingPrivate.bitSearchInformation.idBitNumber = 64; //the last bit
  romSearchingPrivate.bitSearchInformation.byteMask = 0x80;

  TxRxCallbackList *txRxListPointerNext;
  txRxListPointerNext = malloc(sizeof(txRxListPointerNext));

  txRxListPointerNext->txRxCallbackFuncP = doRomSearch;
  txRxList.next = txRxListPointerNext;


  romSearching(&evt);
  //TODO
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.idBitNumber);
  TEST_ASSERT_EQUAL(0, romSearchingPrivate.bitSearchInformation.romByteNum);
  TEST_ASSERT_EQUAL(1, romSearchingPrivate.bitSearchInformation.byteMask);
  TEST_ASSERT_EQUAL(0x80, *(doRomSearchPrivate.romVal + 7));
  //TEST_ASSERT_EQUAL(128, *(romSearchingPrivate.romNo + 7));
  free(txRxEvData.uartRxVal);
  free(txRxListPointerNext);
  // evt.data =
  // evt
}

void someFunction(Event *evt){

}

void anotherFunction(Event *evt){

}


void test_registerCallback_given_emptyList(void){
  LinkedList list;
  ListInit(&list);
  registerCallback(someFunction, &list);
  TEST_ASSERT_EQUAL_PTR(someFunction, ((TxRxCallbackList*)((list.head)->data))->txRxCallbackFuncP);

  Event evt;
  evt.evtType = UART_RX_SUCCESS;
}

void test_registerCallback_given_list_with_1_data(void){
  LinkedList list;
  ListInit(&list);
  registerCallback(someFunction, &list);
  registerCallback(anotherFunction, &list);

  Item *itemHead = list.head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(anotherFunction, callBackList5->txRxCallbackFuncP);
  TEST_ASSERT_NOT_NULL(list.head);
  TxRxCallbackList *callBackListNext = (TxRxCallbackList*)(itemHead->next->data);
  TEST_ASSERT_EQUAL(2,list.len);
  TEST_ASSERT_EQUAL_PTR(someFunction, callBackListNext->txRxCallbackFuncP);
}

void test_unregisterCallback_given_list_with2_data_expect_1Data(void){
  LinkedList list;
  ListInit(&list);
  registerCallback(someFunction, &list);
  registerCallback(anotherFunction, &list);

  unregisterCallback(&list);

  Item *itemHead = list.head;
  TxRxCallbackList *callBackList5 = (TxRxCallbackList*)(itemHead->data);
  TEST_ASSERT_EQUAL_PTR(someFunction, callBackList5->txRxCallbackFuncP);
}
//TODO unregisterCallback test with null
//TODO unregisterCallback test with 1 data

void test_getCurrentCallback(void){
  LinkedList list;
  ListInit(&list);
  registerCallback(someFunction, &list);
  FuncP tempFuncP = getCurrentCallback();
  TEST_ASSERT_EQUAL(someFunction, tempFuncP);

}
//TODO checking for error while searching (lastDeviceFlag maybe?) (optional)