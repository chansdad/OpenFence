#include "PA6C.h"

// global variables
char PA6C::buffer[BUFFSIZ];        // string buffer for the sentence
char PA6C::buffidx;   

//Check that the buffer digits don't change length


PA6C::PA6C(){
;;
}

void PA6C::getGPRMC(){
  readLine();
	while(strncmp(buffer, "$GPRMC",6) != 0){
		readLine(); 
	}
  SerialUSB.println(buffer);
	return;
}

float PA6C::getLatitude(){
	char *parseptr;     // a character pointer for parsing
    parseptr = buffer+20;
	uint32_t raw = parsedecimal(parseptr);
	if (raw != 0) {
  		raw *= 10000;
  		parseptr = strchr(parseptr, '.')+1;
  		raw += parsedecimal(parseptr);
	}
  	float new_val = (raw/1000000);                               //Grab the degrees from the raw number
  	new_val = new_val + ((((float)raw/1000000)-new_val)/0.6);     //Find the minutes from the raw number
	parseptr = strchr(parseptr, ',') + 1;
	// read latitude N/S data
	if (parseptr[0] != ',') {
		if (parseptr[0] == 'S') new_val=new_val*-1;               //Change the sign if it is South or West
	}
  return new_val;
}

float PA6C::getLongitude(){
  char *parseptr;     // a character pointer for parsing
  parseptr = buffer+32;
  uint32_t raw = parsedecimal(parseptr);
  if (raw != 0) {
      raw *= 10000;
      parseptr = strchr(parseptr, '.')+1;
      raw += parsedecimal(parseptr);
  }
  float new_val = (raw/1000000);                               //Grab the degrees from the raw number
  new_val = new_val + ((((float)raw/1000000)-new_val)/0.6);     //Find the minutes from the raw number
  parseptr = strchr(parseptr, ',') + 1;
  // read latitude N/S data
  if (parseptr[0] != ',') {
    if (parseptr[0]  == 'W') new_val=new_val*-1;               //Change the sign if it is South or West
  }
  return new_val;
}

int PA6C::getTime(){
  // hhmmss time data
  char *parseptr;
  parseptr = buffer+7;
  uint32_t tmp = parsedecimal(parseptr); 
  int hour = tmp/ 10000;
  int minute = (tmp / 100) % 100;
  int second = tmp % 100;
  return tmp;
}

int PA6C::getDate(){
  char *parseptr;     // a character pointer for parsing
  parseptr = buffer+57;
  uint32_t tmp = parsedecimal(parseptr); 
  //uint8_t date = tmp / 10000;
  //month = (tmp / 100) % 100;
  //year = tmp % 100;
  return tmp;
}


// Private Functions:
uint32_t PA6C::parsedecimal(char *str){
  uint32_t d = 0;
  
  while (str[0] != 0) {
   if ((str[0] > '9') || (str[0] < '0'))
     return d;
   d *= 10;
   d += str[0] - '0';
   str++;
  }
  return d;
}

void PA6C::readLine(void){
  char c;
  buffidx = 0;
  while(1){
    while(!Serial.available()){}
    c=Serial.read();
    if (c=='\n')
      continue;
    if ((buffidx == BUFFSIZ-1) || (c=='\r')){
      buffer[buffidx]=0;
      return;
    }
    buffer[buffidx++]=c;
  }
}