/*
 * A library for controlling a Microchip RN2483 LoRa radio.
 *
 * @Author JP Meijers
 * @Date 18/12/2015
 *
 */

#include "Arduino.h"
#include "rn2483.h"

extern "C" {
#include <string.h>
#include <stdlib.h>
}

/*
  @param serial Needs to be an already opened stream to write to and read from.
*/
rn2483::rn2483(SoftwareSerial& serial):
_serial(serial)
{
  _serial.setTimeout(2000);
}

rn2483::rn2483(HardwareSerial& serial):
_serial(serial)
{
  _serial.setTimeout(2000);
}

void rn2483::autobaud()
{
  String response = "";
  while (response=="")
  {
    delay(1000);
    _serial.write((byte)0x00);
    _serial.write(0x55);
    _serial.println();
    _serial.println("sys get ver");
    response = _serial.readStringUntil('\n');
  }
}

void rn2483::init()
{
  if(_use_wan) initWan();
  else if(_use_raw) initRaw();
  else if(_use_ttn) initTTN(_ttnAddr);
  else 
  {
    _use_wan = true;
    initWan(); //default is to use WAN
  }
}

void rn2483::initRaw()
{
  _use_raw = true;

  //clear serial buffer
  while(_serial.read() != -1);

  _serial.println("mac pause");
  _serial.readStringUntil('\n');
  _serial.println("radio set mod lora");
  _serial.readStringUntil('\n');
  _serial.println("radio set freq "+_raw_frequency);
  _serial.readStringUntil('\n');
  _serial.println("radio set pwr 14");
  _serial.readStringUntil('\n');
  _serial.println("radio set sf sf12"); //sf7 is fastest, sf12 is furthest
  _serial.readStringUntil('\n');
  _serial.println("radio set afcbw 41.7");
  _serial.readStringUntil('\n');
  _serial.println("radio set rxbw 25");
  _serial.readStringUntil('\n');
  _serial.println("radio set bitrate 50000");
  _serial.readStringUntil('\n');
  _serial.println("radio set fdev 25000");
  _serial.readStringUntil('\n');
  _serial.println("radio set prlen 8");
  _serial.readStringUntil('\n');
  _serial.println("radio set crc on");
  _serial.readStringUntil('\n');
  _serial.println("radio set iqi off");
  _serial.readStringUntil('\n');
  _serial.println("radio set cr 4/5");
  _serial.readStringUntil('\n');
  _serial.println("radio set wdt 60000");
  _serial.readStringUntil('\n');
  _serial.println("radio set sync 12");
  _serial.readStringUntil('\n');
  _serial.println("radio set bw 500"); //500kHz max, 250kHz, 125kHz min
  _serial.readStringUntil('\n');
}

void rn2483::initWan()
{
  _use_wan = true;

  //clear serial buffer
  while(_serial.read() != -1);

  _serial.println("sys get hweui");
  String addr = _serial.readStringUntil('\n');
  addr.trim();
  
  _serial.println("mac reset 868");
  _serial.readStringUntil('\n');
  _serial.println("mac set appeui "+_appeui);
  _serial.readStringUntil('\n');
  _serial.println("mac set appkey "+_appkey);
  _serial.readStringUntil('\n');

  if(addr!="" && addr.length() == 16)
  {
    _serial.println("mac set deveui "+addr);
  }
  else 
  {
    _serial.println("mac set deveui "+_default_deveui);
  }
  _serial.readStringUntil('\n');
  _serial.println("mac set pwridx 1");
  _serial.readStringUntil('\n');
  _serial.println("mac set adr on");
  _serial.readStringUntil('\n');
  // _serial.println("mac set retx 10");
  // _serial.readStringUntil('\n');
  // _serial.println("mac set linkchk 60");
  // _serial.readStringUntil('\n');
  // _serial.println("mac set ar on");
  // _serial.readStringUntil('\n');
  _serial.setTimeout(30000);
  _serial.println("mac save");
  _serial.readStringUntil('\n');
  bool joined = false;

  for(int i=0; i<10 && !joined; i++)
  {
    _serial.println("mac join otaa");
    _serial.readStringUntil('\n');
    String receivedData = _serial.readStringUntil('\n');

    if(receivedData.startsWith("accepted"))
    {
      joined=true;
      delay(1000);
    }
    else
    {
      delay(1000);
    }
  }
  _serial.setTimeout(2000);
}

void rn2483::initTTN(String addr)
{
  _use_ttn = true;
  _ttnAddr = addr;

  //clear serial buffer
  while(_serial.read() != -1);
  
  _serial.println("mac reset 868");
  _serial.readStringUntil('\n');

  _serial.println("mac set rx2 3 869525000");
  _serial.readStringUntil('\n');

  _serial.println("mac set nwkskey 2B7E151628AED2A6ABF7158809CF4F3C");
  _serial.readStringUntil('\n');
  _serial.println("mac set appskey 2B7E151628AED2A6ABF7158809CF4F3C");
  _serial.readStringUntil('\n');

  _serial.println("mac set devaddr "+addr);
  _serial.readStringUntil('\n');

  _serial.println("mac set adr off");
  _serial.readStringUntil('\n');
  _serial.println("mac set ar off");
  _serial.readStringUntil('\n');

  _serial.println("mac set pwridx 1"); //1=max, 5=min
  _serial.readStringUntil('\n');
  _serial.println("mac set dr 5"); //0= min, 7=max
  _serial.readStringUntil('\n');

  _serial.setTimeout(60000);
  _serial.println("mac save");
  _serial.readStringUntil('\n');
  _serial.println("mac join abp");
  _serial.readStringUntil('\n');
  _serial.readStringUntil('\n');
  _serial.setTimeout(2000);
  delay(1000);
}

void rn2483::tx(String data)
{
  if(_use_wan) txCnf(data); //if not specified use acks on WAN
  else if(_use_ttn) txUncnf(data); //ttn does not have downstream, thus no acks
  else if(_use_raw) txRaw(data);
  else txUncnf(data); //we are unsure which mode we're in. Better not to wait for acks.
}

void rn2483::txRaw(String data)
{
  txData("radio tx ", data);
}

void rn2483::txCnf(String data)
{
  txData("mac tx cnf 1 ", data);
}

void rn2483::txUncnf(String data)
{
  txData("mac tx uncnf 1 ", data);
}

bool rn2483::txData(String command, String data)
{
  bool send_success = false;
  uint8_t busy_count = 0;
  uint8_t retry_count = 0;

  while(!send_success)
  {
    //retransmit a maximum of 10 times
    retry_count++;
    if(retry_count>10)
    {
      return false;
    }

    _serial.print(command);
    sendEncoded(data);
    _serial.println();
    String receivedData = _serial.readStringUntil('\n');

    if(receivedData.startsWith("ok"))
    {
      _serial.setTimeout(30000);
      receivedData = _serial.readStringUntil('\n');
      _serial.setTimeout(2000);
      
      if(receivedData.startsWith("mac_tx_ok"))
      {
        //SUCCESS!!
        send_success = true;
        return true;
      }

      else if(receivedData.startsWith("mac_rx"))
      {
        //we received data downstream
        //TODO: handle received data
        send_success = true;
        return true;
      }

      else if(receivedData.startsWith("mac_err"))
      {
        init();
      }

      else if(receivedData.startsWith("invalid_data_len"))
      {
        //this should never happen if the prototype worked
        send_success = true;
        return false;
      }

      else if(receivedData.startsWith("radio_tx_ok"))
      {
        //SUCCESS!!
        send_success = true;
        return true;
      }

      else if(receivedData.startsWith("radio_err"))
      {
        //This should never happen. If it does, something major is wrong.
        init();
      }

      else
      {
        //unknown response
        //init();
      }
    }

    else if(receivedData.startsWith("invalid_param"))
    {
      //should not happen if we typed the commands correctly
      send_success = true;
      return false;
    }

    else if(receivedData.startsWith("not_joined"))
    {
      init();
    }

    else if(receivedData.startsWith("no_free_ch"))
    {
      //retry
      delay(1000);
    }

    else if(receivedData.startsWith("silent"))
    {
      init();
    }

    else if(receivedData.startsWith("frame_counter_err_rejoin_needed"))
    {
      init();
    }

    else if(receivedData.startsWith("busy"))
    {
      busy_count++;

      if(busy_count>=10)
      {
        init();
      }
      else
      {
        delay(1000);
      }
    }

    else if(receivedData.startsWith("mac_paused"))
    {
      init();
    }

    else if(receivedData.startsWith("invalid_data_len"))
    {
      //should not happen if the prototype worked
      send_success = true;
      return false;
    }

    else
    {
      //unknown response after mac tx command
      init();
    }
  }

  return false; //should never reach this
}

void rn2483::sendEncoded(String input)
{
  char working;
  char buffer[3];
  for(int i=0; i<input.length(); i++)
  {
    working = input.charAt(i);
    sprintf(buffer, "%02x", int(working));
    _serial.print(buffer);
  }
}

String rn2483::base16encode(String input)
{
  char charsOut[input.length()*2+1];
  char charsIn[input.length()+1];
  input.trim();
  input.toCharArray(charsIn, input.length()+1);
  
  int i = 0;
  for(i = 0; i<input.length()+1; i++)
  {
    if(charsIn[i] == '\0') break;
    
    int value = int(charsIn[i]);
    
    char buffer[3];
    sprintf(buffer, "%02x", value);
    charsOut[2*i] = buffer[0];
    charsOut[2*i+1] = buffer[1];
  }
  charsOut[2*i] = '\0';
  String toReturn = String(charsOut);
  return toReturn;
}

String rn2483::base16decode(String input)
{
  char charsIn[input.length()+1];
  char charsOut[input.length()/2+1];
  input.trim();
  input.toCharArray(charsIn, input.length()+1);
  
  int i = 0;
  for(i = 0; i<input.length()/2+1; i++)
  {
    if(charsIn[i*2] == '\0') break;
    if(charsIn[i*2+1] == '\0') break;
    
    char toDo[2];
    toDo[0] = charsIn[i*2];
    toDo[1] = charsIn[i*2+1];
    int out = strtoul(toDo, 0, 16);
    
    if(out<128)
    {
      charsOut[i] = char(out);
    }
  }
  charsOut[i] = '\0';
  return charsOut;
}