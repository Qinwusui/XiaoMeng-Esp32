#include<display.h>
#define U8X8_HAVE_HW_I2C
#ifdef U8X8_HAVE_HW_I2C          //定义OLED连接方式为I2C
#include <Wire.h>
#endif
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0 , /* reset=*/U8X8_PIN_NONE , /* clock=*/17 , /* data=*/18); // 1.3寸，指定引脚

void initScreen() {
    u8g2.begin();
    u8g2.setPowerSave(false);
}
