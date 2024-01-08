#include <Arduino.h>
#include <OneWire.h>

#define Ai 35
OneWire  ds(Ai);  // 在引脚10上（需要4.7K电阻）

void setup(void) {
  Serial.begin(115200);
}

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[9];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("没有更多地址。");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC不是有效的！");
      return;
  }
  Serial.println();
 
  // 第一个ROM字节表示芯片类型
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // 或者旧的DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("设备不是DS18x20系列设备。");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // 开始转换，最后以电源供电
  
  delay(1000);     // 可能需要750ms，也可能不需要
  // 我们可能在这里执行ds.depower()，但重置会处理它。
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // 读取Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // 我们可能需要9个字节
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // 将数据转换为实际温度
  // 因为结果是一个16位有符号整数，它应该存储到一个"int16_t"类型的变量中，
  // 这个变量总是16位，即使编译在32位处理器上也是如此。
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9位分辨率默认
    if (data[7] == 0x10) {
      // "count remain"给出完整的12位分辨率
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // 在较低分辨率下，低位是未定义的，所以我们将它们零化
    if (cfg == 0x00) raw = raw & ~7;  // 9位分辨率，93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10位分辨率，187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11位分辨率，375 ms
    //// 默认是12位分辨率，750 ms转换时间
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  温度 = ");
  Serial.print(celsius);
  Serial.print(" 摄氏度， ");
  Serial.print(fahrenheit);
  Serial.println(" 华氏度");
}


