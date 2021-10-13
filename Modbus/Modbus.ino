byte data[] = {0x1, 0x3, 0x0, 0x8, 0x0, 0xA, 0x44, 0xF};
byte received[25], byteRead, bytesRead;
uint32_t tmpInt32, exportedenergy, importedenergy;
uint16_t tmpInt16, voltage, current, activepower, reactivepower, powerfactor, frequency;
int j, i = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Get Data Modbus");
  Serial.println("");
  Serial1.begin(9600);
}

void loop() {
  refreshData();
  Serial.print((String)exportedenergy + " " + (String)importedenergy + " " + (String)voltage + " " + (String)current + " " + (String)activepower + " " + (String)reactivepower + " " + (String)powerfactor + " " + (String)frequency);
  Serial.println(" ");
  delay(200);
}

uint16_t returnInt16(byte offset){
  tmpInt16 = received[offset];
  tmpInt16 <<= 8;
  tmpInt16 += received[offset+1];
  return tmpInt16;
}

uint32_t returnInt32(byte offset){
  tmpInt32 = returnInt16(offset);
  tmpInt32 <<= 16;
  tmpInt32 += returnInt16(offset+2);
  return tmpInt32;
}

void refreshData(){
  exportedenergy = 0;
  importedenergy = 0;
  voltage = 0;
  current = 0;
  activepower = 0;
  reactivepower = 0;
  powerfactor = 0;
  frequency = 0;
  
  Serial1.write(data, sizeof(data));
  delay(50);
  if (Serial1.available()) {
    i = 0;
    byteRead = Serial1.read();
    received[i++] = byteRead;
    
    if (byteRead == 0x01) {
      byteRead = Serial1.read();
      received[i++] = byteRead;
      if (byteRead == 0x03) {
        bytesRead = Serial1.read() * 2 + 2;
        received[i++] = byteRead;
        for (j=0; j<bytesRead; j++) {
          byteRead = Serial1.read();
          received[i++] = byteRead;
        }

        exportedenergy = returnInt32(3);
        importedenergy = returnInt32(7);
        voltage = returnInt16(11);
        current = returnInt16(13);
        activepower = returnInt16(15);
        reactivepower = returnInt16(17);
        powerfactor = returnInt16(19);
        frequency = returnInt16(21);
      }
    }
  }
}
