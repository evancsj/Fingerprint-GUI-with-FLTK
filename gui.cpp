#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <string.h>
#include <stdlib.h>
#include "UART.h"
using namespace std;

#define FALSE -1
#define TRUE 0

int fd =FALSE;

class fingerprintgui:public Fl_Window{
    public:
        fingerprintgui(int w, int h,const char* title);
        ~fingerprintgui();
        Fl_Button* Open;
        Fl_Button* Enroll;
        Fl_Button* Identify;
        Fl_Input* Port;
        Fl_Input* InputName;
        Fl_Output* OutputName;

    private:
        static void cb_Open(Fl_Widget*,void*);
        inline void cb_Open_i();

        static void cb_Enroll(Fl_Widget*,void*);
        inline void cb_Enroll_i();

        static void cb_Identify(Fl_Widget*,void*);
        inline void cb_Identify_i();
};

int main(){
    fingerprintgui win(450,200,"Fingerprint GUI");
    return Fl::run();
}

fingerprintgui::fingerprintgui(int w,int h,const char*title):Fl_Window(w,h,title){
    begin();

    Open = new Fl_Button(280,30,70,30,"&Open");
    Open->callback(cb_Open,this);

    Enroll = new Fl_Button(280,80,70,30,"&Enroll");
    Enroll->callback(cb_Enroll,this);

    Identify = new Fl_Button(280,130,70,30,"&Identify");
    Identify->callback(cb_Identify,this);

    Port = new Fl_Input(150,30,100,30,"Port");
    InputName = new Fl_Input(150,80,100,30,"Input Name");
    OutputName = new Fl_Output(150,130,100,30,"Output Name");

    end();
    resizable(this);
    show();
}

fingerprintgui::~fingerprintgui(){}

void fingerprintgui::cb_Open(Fl_Widget* o,void* v){
    ((fingerprintgui*)v)->cb_Open_i();
}

void fingerprintgui::cb_Open_i(){
    int ret;
    char *port = new char[strlen(Port->value())+1];
    strcpy(port,Port->value());
    fd = UART_Open(fd,port);
    if(FALSE == fd){
        printf("open error\n");
    }
    ret = UART_Init(fd);
    if(FALSE == ret){
        printf("Set Port Error\n");
    }
    else
        printf("success!\n");
}

void fingerprintgui::cb_Enroll(Fl_Widget* o,void* v){
    ((fingerprintgui*)v)->cb_Enroll_i();
}

void fingerprintgui::cb_Enroll_i(){
    int ret;
    unsigned char send_buf[24];
    unsigned char rcv_buf[24];
    unsigned char rcv_buf1[24];
    int i = 0;
    int order;
    int tem,sum=0;
    for(i=0;i<24;i++){send_buf[i]=0x00;}
    send_buf[0] = 0x55;
    send_buf[1] = 0xAA;
    send_buf[2] = 0x03;
    send_buf[3] = 0x01;
    send_buf[4] = 0x02;
    send_buf[5] = 0x00;
    char *temc = new char[strlen(InputName->value())+1];
    strcpy(temc,InputName->value());
    tem = atoi(temc);
    if(tem>3000 || tem<1){
        printf("Please input a template number between 1 and 3000\n");
        return;
    }
    send_buf[6] = tem%256;
    send_buf[7] = tem/256;
    for(i=0;i<8;i++){sum=sum+send_buf[i];}
    send_buf[22] = sum%256;
    send_buf[23] = sum/256;
    ret = UART_Send(fd,send_buf,24);
    if(FALSE==ret){
        printf("write error!\n");
        exit(1);
    }
    ret = UART_Recv(fd,rcv_buf,24);
    if(rcv_buf[6] == 0x01){
        printf("ERR_TMPL_NOT_EMPTY\n");
        return;
    }
    ret = UART_Recv(fd,rcv_buf1,24);
    printf("First finger!\n");
    while(rcv_buf1[6] == 0x01){
        if(rcv_buf1[8] == 0x23) printf("ERR_TIME_OUT\n");
        else if(rcv_buf1[8] == 0x21) printf("ERR_BAD_QUALITY\n");
        ret = UART_Recv(fd,rcv_buf1,24);
    }
    ret = UART_Recv(fd,rcv_buf,24);
    printf("Second finger!\n");
    ret = UART_Recv(fd,rcv_buf1,24);
    while(rcv_buf1[6] == 0x01){
        if(rcv_buf1[8] == 0x23) printf("ERR_TIME_OUT\n");
        else if(rcv_buf1[8] == 0x21) printf("ERR_BAD_QUALITY\n");
        ret = UART_Recv(fd,rcv_buf1,24);
    }
    ret = UART_Recv(fd,rcv_buf,24);
    printf("Third finger!\n");
    ret = UART_Recv(fd,rcv_buf1,24);
    while(rcv_buf1[6] == 0x01){
        if(rcv_buf1[8] == 0x23) printf("ERR_TIME_OUT\n");
        else if(rcv_buf1[8] == 0x21) printf("ERR_BAD_QUALITY\n");
        ret = UART_Recv(fd,rcv_buf1,24);
    }
    ret = UART_Recv(fd,rcv_buf,24);
    if(rcv_buf[6] == 0x01){
        if(rcv_buf[8] == 0x19){
            printf("ERR_DUPLICATION_ID\n");
            order = rcv_buf[11]*256+rcv_buf[10];
            printf("The order is %d\n",order);
        }
        else if(rcv_buf[8] == 0x30){
            printf("ERR_GENERALIZE\n");
        }
    }
    else{
        printf("Success\n");
        order = rcv_buf[9]*256+rcv_buf[8];
        printf("The order is %d\n",order);
    }
}

void fingerprintgui::cb_Identify(Fl_Widget* o,void* v){
    ((fingerprintgui*)v)->cb_Identify_i();
}

void fingerprintgui::cb_Identify_i(){
    int ret;
    unsigned char send_buf[24];
    unsigned char rcv_buf[24];
    unsigned char rcv_buf1[24];
    int i = 0;
    int order;
    for(i=0;i<24;i++){send_buf[i]=0x00;}
    send_buf[0] = 0x55;
    send_buf[1] = 0xAA;
    send_buf[2] = 0x02;
    send_buf[3] = 0x01;
    send_buf[22] = 0x02;
    send_buf[23] = 0x01;
    ret = UART_Send(fd,send_buf,24);
    if(FALSE==ret){
        printf("write error!\n");
        exit(1);
    }
    printf("Please input your finger.\n");
    ret = UART_Recv(fd,rcv_buf,24);
    ret = UART_Recv(fd,rcv_buf1,24);
    if(rcv_buf1[6] == 0x01){
        if(rcv_buf1[8] == 0x12) printf("ERR_IDENTIFY\n");
        else if(rcv_buf1[8] == 0x23) printf("ERR_TIME_OUT\n");
        else if(rcv_buf1[8] == 0x21) printf("ERR_BAD_QUALITY\n");
        OutputName->value("none");
    }
    else{
        order = rcv_buf1[9]*256+rcv_buf1[8];
        char string[4];
        sprintf(string,"%d",order);
        printf("success\n",order);
        OutputName->value(string);
    }
}
