#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#define CLOSE closesocket
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define CLOSE close
#endif
#define MAX 512
#define BUF 65536
typedef struct {int id,seats; char name[64],from[64],to[64],date[16],time[8]; double fare;} Bus;
typedef struct {int id,bus,seat;char passenger[80],phone[24];} Booking;
Bus buses[MAX];Booking bookings[MAX];int nb=0,nk=0;
void load(){FILE*f=fopen("buses.dat","rb");if(f){fread(&nb,sizeof(int),1,f);if(nb<0||nb>MAX)nb=0;fread(buses,sizeof(Bus),nb,f);fclose(f);}f=fopen("bookings.dat","rb");if(f){fread(&nk,sizeof(int),1,f);if(nk<0||nk>MAX)nk=0;fread(bookings,sizeof(Booking),nk,f);fclose(f);}}
void save(){FILE*f=fopen("buses.dat","wb");if(f){fwrite(&nb,sizeof(int),1,f);fwrite(buses,sizeof(Bus),nb,f);fclose(f);}f=fopen("bookings.dat","wb");if(f){fwrite(&nk,sizeof(int),1,f);fwrite(bookings,sizeof(Booking),nk,f);fclose(f);}}
void param(const char*q,const char*k,char*out,size_t n){out[0]=0;size_t kl=strlen(k);while(*q){if(!strncmp(q,k,kl)&&q[kl]=='='){q+=kl+1;size_t i=0;while(*q&&*q!='&'&&i+1<n){if(*q=='+'){out[i++]=' ';q++;}else if(*q=='%'&&isxdigit((unsigned char)q[1])&&isxdigit((unsigned char)q[2])){unsigned x;sscanf(q+1,"%2x",&x);out[i++]=(char)x;q+=3;}else out[i++]=*q++;}out[i]=0;return;}q=strchr(q,'&');if(!q)return;q++;}}
int num(const char*q,const char*k){char s[32];param(q,k,s,sizeof(s));return atoi(s);}double money(const char*q,const char*k){char s[32];param(q,k,s,sizeof(s));return atof(s);}
int busidx(int id){for(int i=0;i<nb;i++)if(buses[i].id==id)return i;return -1;}int bookidx(int id){for(int i=0;i<nk;i++)if(bookings[i].id==id)return i;return -1;}
int dateok(const char*s){if(strlen(s)!=10||s[4]!='-'||s[7]!='-')return 0;int y,m,d;if(sscanf(s,"%4d-%2d-%2d",&y,&m,&d)!=3)return 0;if(y<2020||m<1||m>12||d<1||d>31)return 0;return 1;}
void jsonstr(char*out,size_t cap,const char*s){size_t i=strlen(out);if(i+2>=cap)return;out[i++]='"';for(;*s&&i+7<cap;s++){unsigned char c=*s;if(c=='"'||c=='\\'){out[i++]='\\';out[i++]=c;}else if(c<32){i+=snprintf(out+i,cap-i,"\\u%04x",c);}else out[i++]=c;}out[i++]='"';out[i]=0;}
void sendresp(int fd,int code,const char*type,const char*body){char h[512];int n=snprintf(h,sizeof(h),"HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n",code,code==200?"OK":"Bad Request",type,strlen(body));send(fd,h,n,0);send(fd,body,(int)strlen(body),0);}
void api(int fd,const char*path,const char*q){char out[BUF]={0},a[128],b[128],c[128],d[128],e[128],f[128];int id=num(q,"id");
if(!strcmp(path,"/api/buses")){strcat(out,"[");for(int i=0;i<nb;i++){Bus*x=&buses[i];char t[256];if(i)strcat(out,",");snprintf(t,sizeof(t),"{\"id\":%d,\"seats\":%d,\"fare\":%.2f,\"name\":",x->id,x->seats,x->fare);strcat(out,t);jsonstr(out,BUF,x->name);strcat(out,",\"from\":");jsonstr(out,BUF,x->from);strcat(out,",\"to\":");jsonstr(out,BUF,x->to);strcat(out,",\"date\":");jsonstr(out,BUF,x->date);strcat(out,",\"time\":");jsonstr(out,BUF,x->time);strcat(out,"}");}strcat(out,"]");}
else if(!strcmp(path,"/api/availability")){strcat(out,"[");for(int i=0;i<nb;i++){Bus*x=&buses[i];char t[256];if(i)strcat(out,",");snprintf(t,sizeof(t),"{\"id\":%d,\"seats\":%d,\"fare\":%.2f,\"name\":",x->id,x->seats,x->fare);strcat(out,t);jsonstr(out,BUF,x->name);strcat(out,",\"from\":");jsonstr(out,BUF,x->from);strcat(out,",\"to\":");jsonstr(out,BUF,x->to);strcat(out,",\"date\":");jsonstr(out,BUF,x->date);strcat(out,",\"time\":");jsonstr(out,BUF,x->time);strcat(out,",\"taken\":[");int first=1;for(int j=0;j<nk;j++)if(bookings[j].bus==x->id){if(!first)strcat(out,",");first=0;snprintf(t,sizeof(t),"%d",bookings[j].seat);strcat(out,t);}strcat(out,"]}");}strcat(out,"]");}
else if(!strcmp(path,"/api/ticket")){char phone[32];param(q,"phone",phone,sizeof(phone));int i=bookidx(id);if(i<0||!phone[0]||strcmp(bookings[i].phone,phone))strcpy(out,"{\"error\":\"Booking not found or phone does not match\"}");else{Booking*x=&bookings[i];int j=busidx(x->bus);char t[256];snprintf(t,sizeof(t),"{\"id\":%d,\"bus\":%d,\"seat\":%d,\"passenger\":",x->id,x->bus,x->seat);strcat(out,t);jsonstr(out,BUF,x->passenger);if(j>=0){strcat(out,",\"route\":");jsonstr(out,BUF,buses[j].from);strcat(out,",\"destination\":");jsonstr(out,BUF,buses[j].to);strcat(out,",\"date\":");jsonstr(out,BUF,buses[j].date);strcat(out,",\"time\":");jsonstr(out,BUF,buses[j].time);snprintf(t,sizeof(t),",\"fare\":%.2f",buses[j].fare);strcat(out,t);}strcat(out,"}");}}
else if(!strcmp(path,"/api/bookings")){strcat(out,"[");for(int i=0;i<nk;i++){Booking*x=&bookings[i];char t[200];if(i)strcat(out,",");snprintf(t,sizeof(t),"{\"id\":%d,\"bus\":%d,\"seat\":%d,\"passenger\":",x->id,x->bus,x->seat);strcat(out,t);jsonstr(out,BUF,x->passenger);strcat(out,",\"phone\":");jsonstr(out,BUF,x->phone);strcat(out,"}");}strcat(out,"]");}
else if(!strcmp(path,"/api/bus/save")){param(q,"name",a,sizeof(a));param(q,"from",b,sizeof(b));param(q,"to",c,sizeof(c));param(q,"date",d,sizeof(d));param(q,"time",e,sizeof(e));int seats=num(q,"seats");double fare=money(q,"fare");int idx=busidx(id);if(!*a||!*b||!*c||!dateok(d)||strlen(e)!=5||seats<1||seats>100||fare<0||(!id&&nb>=MAX))strcpy(out,"{\"error\":\"Invalid bus details\"}");else{if(idx<0){idx=nb++;id=1001;for(int i=0;i<nb-1;i++)if(buses[i].id>=id)id=buses[i].id+1;}else{for(int i=0;i<nk;i++)if(bookings[i].bus==id&&bookings[i].seat>seats){strcpy(out,"{\"error\":\"Existing booking exceeds new seat capacity\"}");break;}}if(!*out){Bus*x=&buses[idx];memset(x,0,sizeof(*x));x->id=id;x->seats=seats;x->fare=fare;snprintf(x->name,sizeof(x->name),"%s",a);snprintf(x->from,sizeof(x->from),"%s",b);snprintf(x->to,sizeof(x->to),"%s",c);snprintf(x->date,sizeof(x->date),"%s",d);snprintf(x->time,sizeof(x->time),"%s",e);save();sprintf(out,"{\"ok\":true,\"id\":%d}",id);}}}
else if(!strcmp(path,"/api/bus/delete")){int i=busidx(id),used=0;for(int j=0;j<nk;j++)if(bookings[j].bus==id)used=1;if(i<0||used)strcpy(out,"{\"error\":\"Bus not found or has reservations\"}");else{memmove(&buses[i],&buses[i+1],(nb-i-1)*sizeof(Bus));nb--;save();strcpy(out,"{\"ok\":true}");}}
else if(!strcmp(path,"/api/booking/save")){int bus=num(q,"bus"),seat=num(q,"seat"),bi=busidx(bus),idx=bookidx(id),taken=0;param(q,"passenger",a,sizeof(a));param(q,"phone",b,sizeof(b));for(int i=0;i<nk;i++)if(bookings[i].bus==bus&&bookings[i].seat==seat&&bookings[i].id!=id)taken=1;if(bi<0||seat<1||seat>buses[bi].seats||!*a||strlen(b)<7||strlen(b)>20||taken||(!id&&nk>=MAX))strcpy(out,"{\"error\":\"Invalid details or seat already booked\"}");else{if(idx<0){idx=nk++;id=5001;for(int i=0;i<nk-1;i++)if(bookings[i].id>=id)id=bookings[i].id+1;}Booking*x=&bookings[idx];memset(x,0,sizeof(*x));x->id=id;x->bus=bus;x->seat=seat;snprintf(x->passenger,sizeof(x->passenger),"%s",a);snprintf(x->phone,sizeof(x->phone),"%s",b);save();sprintf(out,"{\"ok\":true,\"id\":%d}",id);}}
else if(!strcmp(path,"/api/booking/delete")){int i=bookidx(id);if(i<0)strcpy(out,"{\"error\":\"Reservation not found\"}");else{memmove(&bookings[i],&bookings[i+1],(nk-i-1)*sizeof(Booking));nk--;save();strcpy(out,"{\"ok\":true}");}}
else if(!strcmp(path,"/api/report")){strcpy(out,"Booking ID,Passenger,Phone,Bus ID,Seat,Fare\n");for(int i=0;i<nk;i++){Booking*x=&bookings[i];int j=busidx(x->bus);char t[256];snprintf(t,sizeof(t),"%d,\"%s\",\"%s\",%d,%d,%.2f\n",x->id,x->passenger,x->phone,x->bus,x->seat,j>=0?buses[j].fare:0);if(strlen(out)+strlen(t)<BUF-1)strcat(out,t);}sendresp(fd,200,"text/csv; charset=utf-8",out);return;}
else strcpy(out,"{\"error\":\"Unknown endpoint\"}");sendresp(fd,200,"application/json; charset=utf-8",out);}
static int is_admin_route(const char *url){
 return !strcmp(url,"/admin") || !strcmp(url,"/admin.html") || !strcmp(url,"/api/bookings") || !strcmp(url,"/api/report") || !strncmp(url,"/api/bus/",9) || !strcmp(url,"/api/booking/delete");
}
static int auth_ok(const char *request){
 const char *pw=getenv("ADMIN_PASSWORD");
 if(!pw || !*pw) return 0;
 const char *header=strstr(request,"\r\nAuthorization: Basic ");
 if(!header) return 0;
 header+=strlen("\r\nAuthorization: Basic ");
 const char *end=strstr(header,"\r\n");
 if(!end || end-header>1024) return 0;
 static const char b64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 char encoded[1024];char raw[512];snprintf(raw,sizeof(raw),"admin:%s",pw);
 size_t pos=0,n=strlen(raw);
 for(size_t i=0;i<n;i+=3){unsigned a=(unsigned char)raw[i],b=i+1<n?(unsigned char)raw[i+1]:0,c=i+2<n?(unsigned char)raw[i+2]:0;
 if(pos+4>=sizeof(encoded))return 0;
 encoded[pos++]=b64[a>>2];encoded[pos++]=b64[((a&3)<<4)|(b>>4)];encoded[pos++]=i+1<n?b64[((b&15)<<2)|(c>>6)]:'=';encoded[pos++]=i+2<n?b64[c&63]:'=';
 }
 return (size_t)(end-header)==pos && !strncmp(header,encoded,pos);
}
static void unauthorized(int fd){const char *body="Admin login required";char h[512];int n=snprintf(h,sizeof(h),"HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"BusNova Admin\"\r\nContent-Type: text/plain\r\nCache-Control: no-store\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",strlen(body));send(fd,h,n,0);send(fd,body,(int)strlen(body),0);}
int main(){
#ifdef _WIN32
WSADATA w;WSAStartup(MAKEWORD(2,2),&w);
#endif
load();int s=socket(AF_INET,SOCK_STREAM,0),yes=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char*)&yes,sizeof(yes));struct sockaddr_in addr={0};addr.sin_family=AF_INET;const char *envport=getenv("PORT");int port=envport?atoi(envport):8080;if(port<1||port>65535)port=8080;
addr.sin_port=htons((unsigned short)port);addr.sin_addr.s_addr=htonl(INADDR_ANY);if(bind(s,(struct sockaddr*)&addr,sizeof(addr))<0||listen(s,16)<0){perror("server");return 1;}printf("BusNova listening on port %d\n",port);fflush(stdout);for(;;){int fd=accept(s,NULL,NULL);if(fd<0)continue;char req[BUF+1]={0};int n=recv(fd,req,BUF,0);if(n<=0){CLOSE(fd);continue;}req[n]=0;char method[12],url[4096];if(sscanf(req,"%11s %4095s",method,url)!=2){CLOSE(fd);continue;}char *q=strchr(url,'?');if(q)*q++=0;else q="";if(is_admin_route(url) && !auth_ok(req)){unauthorized(fd);CLOSE(fd);continue;}
if(!strcmp(url,"/api/booking/save") && num(q,"id")!=0 && !auth_ok(req)){sendresp(fd,403,"application/json","{\"error\":\"Admin login required for editing bookings\"}");CLOSE(fd);continue;}
if(!strncmp(url,"/api/",5))api(fd,url,q);else if(!strcmp(url,"/")||!strcmp(url,"/index.html")||!strcmp(url,"/admin")||!strcmp(url,"/admin.html")){const char *page=(!strcmp(url,"/admin")||!strcmp(url,"/admin.html"))?"admin.html":"index.html";FILE*f=fopen(page,"rb");if(!f)sendresp(fd,400,"text/plain","index.html missing");else{fseek(f,0,SEEK_END);long len=ftell(f);rewind(f);char*h=malloc(len+1);if(h){size_t got=fread(h,1,len,f);h[got]=0;sendresp(fd,200,"text/html; charset=utf-8",h);free(h);}fclose(f);}}else sendresp(fd,400,"text/plain","Not found");CLOSE(fd);}return 0;}
