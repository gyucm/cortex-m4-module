#ifndef __WIFI_H_
#define __WIFI_H_


#define SSID "iot0"
#define PASS "iot00000"
#define LOGID "1"
#define PASSWD "PASSWD"
#define DST_IP "10.10.141.6x"
#define DST_PORT 5000

int connectWiFi();
int wifi_send_cmd (char *str,const char *rstr, unsigned long timeout);
int wifi_wait(const char *str, const char *rstr, unsigned long timeout);
int WIFI_init();
int WIFI_send(char * data);
#endif
