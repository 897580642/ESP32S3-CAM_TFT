/**********************************************************************
Camera 获取图像在 TFT屏幕显示 
**********************************************************************/
#define CONFIG_CAMERA_CONVERTER_ENABLED  1 //配置摄像头转换功能打开
#include <TFT_eSPI.h> 
#include <SPI.h>
#include "esp_camera.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "sdkconfig.h"
#define CAMERA_MODEL_ESP32S3_EYE
#include "camera_pins.h"
#define LED_BUILT_IN  2

#define CENTER 160
#include <vector>
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "face_recognition_tool.hpp"
//#include "face_recognition_112_v1_s16.hpp"
#include "face_recognition_112_v1_s8.hpp"
#define FACE_ID_SAVE_NUMBER 7
#define FACE_COLOR_WHITE 0x00FFFFFF
#define FACE_COLOR_BLACK 0x00000000
#define FACE_COLOR_RED 0x000000FF
#define FACE_COLOR_GREEN 0x0000FF00
#define FACE_COLOR_BLUE 0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)
TFT_eSPI my_lcd = TFT_eSPI(); 
//FaceRecognition112V1S16 recognizer;
FaceRecognition112V1S8 recognizer;
long int pi,pj,pl,pc;
uint32_t colordata;
uint16_t color16data[76800];
extern  const unsigned char gImage_456;
void setup() {
  my_lcd.init();  //液晶屏初始化
  my_lcd.setRotation(4);//TFT屏幕方向 默认1
  my_lcd.fillScreen(TFT_BLACK); 
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  Serial.println();
  pinMode(LED_BUILT_IN, OUTPUT);
  cameraSetup();
  disableCore0WDT();
  
}

static void rgb_print(fb_data_t *fb, uint32_t color, const char *str)
{
    fb_gfx_print(fb, (fb->width - (strlen(str) * 14)) / 2, 10, color, str);
}

void loop() {

      camera_fb_t * fb = NULL;
      while (1) {
        fb = esp_camera_fb_get();
        if (fb != NULL) {
          uint8_t slen[4];
          slen[0] = fb->len >> 0;
          slen[1] = fb->len >> 8;
          slen[2] = fb->len >> 16;
          slen[3] = fb->len >> 24;
          
          pl=0;
          pc=0;
          for(pi=0;pi<240;pi++)
          {

            for(pj=0;pj<320;pj++)
            {
               
                colordata=fb->buf[pl+1];
                colordata<<=8;
                colordata+=fb->buf[pl];
                color16data[pc]=colordata;
                //my_lcd.drawPixel(pi,pj,colordata);
                pl+=2;
                pc+=1;
            }
          }my_lcd.pushImage(0, 0, 320, 240,&color16data[0],true);//while(1);
         
          esp_camera_fb_return(fb);
        }
        else {
          Serial.println("Camera Error");
        }
      }
     
 
}

void cameraSetup() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_RGB565; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  config.conv_mode=YUV422_TO_RGB565;//图像转成RGB565格式
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  // for larger pre-allocated frame buffer.
  if(psramFound()){
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_QVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  s->set_vflip(s, 1); // flip it back
  s->set_brightness(s, 1); // up the brightness just a bit
  s->set_saturation(s, 0); // lower the saturation
  Serial.println("Camera configuration complete!");
  
}
