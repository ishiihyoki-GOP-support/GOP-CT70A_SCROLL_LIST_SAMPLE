#ifndef __SCI
#define __SCI
#ifdef __cplusplus
extern "C" {
#endif
	int PortInit(int portno);
	void PortClose();
	void PortSetRate(int rate);
	int commGetc();
	void commPutc(char c);
	void commPuts(char *s);
	char *commGets();
	int PortEnable();
	void CommConnect(int portNo);
	void SetRTS(int mode);
#ifdef __cplusplus
}
#endif
#endif