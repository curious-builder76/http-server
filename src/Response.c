#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<inttypes.h>


#include "mimetypes.h"


typedef struct{
	size_t capacity;
	size_t used;
	char* buff;
}string_t;


string_t* string_new(){
	string_t* str=malloc(sizeof(string_t));
	if(!str) return NULL;
	str->capacity=1024;
	str->buff=malloc(str->capacity);
	if(!str->buff) {
		free(str);
		return NULL;
	}
	memset(str->buff,0,str->capacity);
	str->used=0;
	return str;
}



int string_grow(string_t* string){
	size_t grow_by=2;	

	size_t mem_required=string->capacity*grow_by;

	char* new_buff=realloc(string->buff,mem_required);
	if(!new_buff) return 1;

	string->buff=new_buff;
	string->capacity=mem_required;
	return 0;
}

int string_append(string_t* string,char* buff){
	size_t string_size=strlen(buff)+1;

	if(string->capacity-string->used<=string_size){
		if(string_grow(string)){
			return 1;
		}
	}
	memcpy(string->buff+string->used,buff,string_size);
	string_size--;
	string->used+=(string_size); // Let the new append operation overwrite this old nul terminator.
	return 0;
}
int string_addch(string_t* string,char ch){
	char buff[2];
	buff[0]=ch;
	buff[1]=0;
	return string_append(string,buff);
}

int string_append_int(string_t* string,size_t num){
	size_t factor=1;
	while(factor<=num){
		factor*=10;
	}
	char map[]="0123456789";
	while(num!=0){
		factor /= 10;
		size_t msd = num / factor;
		if(string_addch(string,map[msd])) {
			return 1;
		}
		num=num % factor;


	};

	return 0;
}
int string_reset(string_t* string){
	string->used=0;
	return 0;
}
char* string_raw(string_t* string){
	return string->buff;
}

size_t string_len(string_t* string){
	return string->used;
}

void string_destroy(string_t* str){
	if(!str) return;

	free(str->buff);
	free(str);
}


static string_t* header=NULL;

static int _init_flag=0;

int response_init(){
	if(_init_flag) return 0;
	mimetypes_init();
	header=string_new();
	if(!header) return 1;
	_init_flag=1;
	return 0;
}

int response_quit(){
	if(!_init_flag) return 1;
	string_destroy(header);
	mimetypes_quit();
	header=NULL;
	_init_flag=0;
	return 0;
}


struct {
	int status_code;
	char* string_status;
	char* reason;
}response_type[]={

	// 1xx Informational

	{ 100, "100", "Continue" },
	{ 101, "101", "Switching Protocols" },
	{ 102, "102", "Processing" },

	// 2xx Success

	{ 200, "200", "OK" },
	{ 201, "201", "Created" },
	{ 202, "202", "Accepted" },
	{ 203, "203", "Non-authoritative Information" },
	{ 204, "204", "No Content" },
	{ 205, "205", "Reset Content" },
	{ 206, "206", "Partial Content" },
	{ 207, "207", "Multi-Status" },
	{ 208, "208", "Already Reported" },
	{ 226, "226", "IM Used" },

	// 3xx Redirection

	{ 300, "300", "Multiple Choices" },
	{ 301, "301", "Moved Permanently" },
	{ 302, "302", "Found" },
	{ 303, "303", "See Other" },
	{ 304, "304", "Not Modified" },
	{ 305, "305", "Use Proxy" },
	{ 307, "307", "Temporary Redirect" },
	{ 308, "308", "Permanent Redirect" },

	// 4xx Client Error

	{ 400, "400", "Bad Request" },
	{ 401, "401", "Unauthorized" },
	{ 402, "402", "Payment Required" },
	{ 403, "403", "Forbidden" },
	{ 404, "404", "Not Found" },
	{ 405, "405", "Method Not Allowed" },
	{ 406, "406", "Not Acceptable" },
	{ 407, "407", "Proxy Authentication Required" },
	{ 408, "408", "Request Timeout" },
	{ 409, "409", "Conflict" },
	{ 410, "410", "Gone" },
	{ 411, "411", "Length Required" },
	{ 412, "412", "Precondition Failed" },
	{ 413, "413", "Payload Too Large" },
	{ 414, "414", "Request-URI Too Long" },
	{ 415, "415", "Unsupported Media Type" },
	{ 416, "416", "Requested Range Not Satisfiable" },
	{ 417, "417", "Expectation Failed" },
	{ 418, "418", "I’m a teapot" },
	{ 421, "421", "Misdirected Request" },
	{ 422, "422", "Unprocessable Entity" },
	{ 423, "423", "Locked" },
	{ 424, "424", "Failed Dependency" },
	{ 426, "426", "Upgrade Required" },
	{ 428, "428", "Precondition Required" },
	{ 429, "429", "Too Many Requests" },
	{ 431, "431", "Request Header Fields Too Large" },
	{ 444, "444", "Connection Closed Without Response" },
	{ 451, "451", "Unavailable For Legal Reasons" },
	{ 499, "499", "Client Closed Request" },

	// 5xx Server Error

	{ 500, "500", "Internal Server Error" },
	{ 501, "501", "Not Implemented" },
	{ 502, "502", "Bad Gateway" },
	{ 503, "503", "Service Unavailable" },
	{ 504, "504", "Gateway Timeout" },
	{ 505, "505", "HTTP Version Not Supported" },
	{ 506, "506", "Variant Also Negotiates" },
	{ 507, "507", "Insufficient Storage" },
	{ 508, "508", "Loop Detected" },
	{ 510, "510", "Not Extended" },
	{ 511, "511", "Network Authentication Required" },
	{ 599, "599", "Network Connect Timeout Error" },
	{ -1, NULL, NULL },
};

char* get_ext(char* filename){
	while(*filename && *filename!='.') filename++;
	return filename;
}
int xsendfile(int conn,char* filename,int status_code){
	int i=0;
	for(; response_type[i].status_code!=-1 && response_type[i].status_code!=status_code;i++);
	char* content_type=get_ext(filename);
	content_type=get_mime(content_type);
	string_reset(header);
	string_append(header,"HTTP/1.1 ");
	string_append(header,response_type[i].string_status);
	string_append(header," ");
	string_append(header,response_type[i].reason);
	string_append(header,"\r\n");
	string_append(header,"Content-type: ");
	string_append(header,content_type);
	string_append(header,"\r\n");
	
	FILE* file=fopen(filename,"rb");

	if(!file){
		return 1;
	};

	char buff[4096];

	fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);
	rewind(file);
	string_append(header,"Content-length: ");
	string_append_int(header,size);
	string_append(header,"\r\n\r\n");
	printf("Size: %zu\n",size);
	send(conn,string_raw(header),string_len(header),0);
	while(size>0){
		size_t read=fread(buff,1,sizeof(buff),file);
		size-=read;
		send(conn,buff,read,0);
	}
	fclose(file);
	return 0;
}

