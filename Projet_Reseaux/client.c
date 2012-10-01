#include "defs.h"
int port_TCP;
int port_UDP;
int sock_UDP;
int sock_TCP;
char *IP_server;
struct sockaddr_in serv_UDP, addr_UDP;
struct sockaddr_in serv_TCP, addr_TCP;
mail* mail_content;
list* inbox_list;
list* sendbox_list;
list* compose_list;
char* username_list;
struct sockaddr exp;
socklen_t exp_sz;
int CONNECTED;//If TCP connection is established with server



//client-side funcitons
void show_inbox(list* mail_list){
if(is_empty(mail_list)){printf("The inbox is empty\n");}
else{
    printf("*****************************LIST OF INBOX**************************\n");
        int i=1;
        list* p=mail_list;
        while(p!=NULL){
            mail *content_temp=p->data;
            printf("[Number]:%d [Source]:%s [Destination]:%s\n[Object]: %s [Date]:%s [Status]:%d\n", i, content_temp->src_nick, content_temp->dst_nick, content_temp->object, content_temp->date, content_temp->read);
            //print_mail(content_temp);
            p=p->next;
            i++;
        }
    printf("*************************************************************************\n");
    }
}

void show_sendbox(list *mail_list){
if(is_empty(mail_list)){printf("The sendbox is empty\n");}
else{
    printf("*****************************LIST OF SENDBOX**************************\n");
        int i=1;
        list* p=mail_list;
        while(p!=NULL){
            mail *content_temp=p->data;
            printf("[Number]:%d [Source]:%s [Destination]:%s\n[Object]: %s [Date]:%s\n", i, content_temp->src_nick, content_temp->dst_nick, content_temp->object, content_temp->date);
            //print_mail(content_temp);
            p=p->next;
            i++;
        }
    printf("*************************************************************************\n");
    }
}


void show_composebox(list* mail_list){
if(is_empty(mail_list)){printf("The composebox is empty\n");}
else{
    printf("*****************************LIST OF COMPOSEBOX**************************\n");
        int i=1;
        list* p=mail_list;
        while(p!=NULL){
            mail *content_temp=p->data;
            printf("[Number]:%d [Source]:%s [Destination]:%s\n[Object]: %s [Date]:%s", i, content_temp->src_nick, content_temp->dst_nick, content_temp->object, content_temp->date);
            //print_mail(content_temp);
            p=p->next;
            i++;
        }
    printf("*************************************************************************\n");
    }
}


//consult mails, just for inbox
list* consult_inbox(int i, list* l){
if(i<1){printf("Error of position\n");}
   else if(is_empty(l)){printf("The inbox is empty\n");}
    else{
            list* p=get_position(l, i);
            if(p==NULL){printf("We can not find the related mails\n");
            }
            else{
                printf("*****************************DETAIL OF MAIL**************************\n");
                mail* content_temp=p->data;
                content_temp->read=1;
                //printf("[Number]:%d [Source]:%s [Destination]:%s\n[Object]: %s [Date]:%s [Status]:%d\n\n", i, content_temp->src_nick, content_temp->dst_nick, content_temp->object, content_temp->date, content_temp->read);
                //printf("[Message]:%s\n\n", content_temp->message);
                print_mail(content_temp);
            }
            return l;
        }
        return NULL;
}


list* consult_sendbox(int i, list* l){
if(i<1){printf("Error of position\n");}
   else if(is_empty(l)){printf("The sendbox is empty\n");}
    else{
            list* p=get_position(l, i);
            if(p==NULL){printf("We can not find the related mails\n");
            }
            else{
                printf("*****************************DETAIL OF MAIL**************************\n");
                mail* content_temp=p->data;
                //printf("[Number]:%d [Source]:%s [Destination]:%s\n[Object]: %s [Date]:%s\n\n", i, content_temp->src_nick, content_temp->dst_nick, content_temp->object, content_temp->date);
                //printf("[Message]:%s\n\n", content_temp->message);
                print_mail(content_temp);
            }
            return l;
        }
        return NULL;
}


list* delete_mail(list* l, int i){
    l=delete_position(l, i);
    return l;
}


void compose(){
    socklen_t sz=sizeof(serv_TCP);
    char message_recu[518];
    char connection[100];
    mail_content=(mail*)malloc(sizeof(mail));
    char src[NICKLEN], dst[NICKLEN], obj[OBJLEN] ,date[30], mes[MESLEN];
    printf("You chose <Composition of A mail>. \nNow type your nickname\n");
    scanf("%s",src);
    //check registration??
    printf("Please input your dest nickname\n");
    scanf("%s", dst);
    //check valability
    printf("Please input the date time. Format:DD/MM/YYYY\n");
    scanf("%s", date);
    printf("Please Input the mail title\n");
    scanf("%s", obj);
    printf("Type your message, type '0' for termination\n");
    fflush(stdin);
    int a;
    int i=0;
    while ((a = getchar()) != '0'){
    mes[i]=a;
    i++;
    }
    mes[i]='\0';

    printf("\n******************************\n");
    printf("Your Message will be send is:\n");
    printf("%s", mes);
    printf("\n******************************\n");

    strcpy(mail_content->message, mes);
    strcpy(mail_content->src_nick, src);
    strcpy(mail_content->dst_nick, dst);
    strcpy(mail_content->date, date);
    strcpy(mail_content->object, obj);
    mail_content->read=0;

    char command[10];
    while(1){
        printf("Ready to send to Server? Type Y or N\n");
        scanf("%s", command);
        if(strcmp("Y", command)==0)
            {
            if(CONNECTED){//if the user is TCP connected to server
             sendMail(mail_content);
             sendbox_list=append_top(sendbox_list, mail_content);
             printf("Your message has been stored in the Send box\nYou can type <SENDBOX> to check\n\n");
             break;
            }else{connection_tcp();continue;}
            }else{
                compose_list=append_top(compose_list, mail_content);
                 printf("Your message has been stored in the Compose box\nYou can type <COMPOSEBOX> to chenck\n\n");
                 break;
            }
    }
}

void getUsernameList()
{
    char message[50]="users";
    char username_list[255];
    char dest[255];
    if(sendto(sock_UDP, (void *) message, sizeof(message), 0, (struct sockaddr*)&serv_UDP, sizeof(addr_UDP))>0){
        printf("send command <get userlist> ok\n");
    }//send to server UDP
    exp_sz=sizeof(exp);
    recvfrom(sock_UDP,(void *)username_list, sizeof(username_list), 0, &exp, &exp_sz);
    int i=0;
    printf("The userlist received is :%s\n", username_list);

    get_field_by_number(username_list, dest, 0, ':');

    //formatted output
    if(strcmp("USERLIST", dest)==0){
        get_field_by_number(username_list, dest,  1, ':');
        strcpy(username_list, dest);
        while(strlen(dest)>0){
        get_field_by_number(username_list, dest, i, ' ');
        printf("%s\n", dest);
        i++;
    }
    }
}

void subscribe()
{
    char dest[255];
    char command[150];
    get_text_from_stdin(command, 150, "Please subscribe with format: subscribe <name> <password>\n");
    if(sendto(sock_UDP, command, 150, 0, (struct sockaddr*)&serv_UDP, sizeof(addr_UDP))>0)
        printf("Subscribe command sent OK\n");//send to server UDP
    if(recvfrom(sock_UDP,(void *)&dest, sizeof(dest), 0, &exp, &exp_sz)>0)//if server replied
        printf("\n>message received from server: %s\n",dest);
}

void unsubscribe()
{
    char dest[255];
    char command[150];
    get_text_from_stdin(command, 150, "Please type the name you want to delete from server: unsubscribe <nom> <password>\n");
    if(sendto(sock_UDP, command, 150, 0, (struct sockaddr*)&serv_UDP, sizeof(addr_UDP))>0)
        printf("Unsubscribe command sent ok\n");//send to server UDP
     if(recvfrom(sock_UDP,(void *)&dest, sizeof(dest), 0, &exp, &exp_sz)>0)//if server replied
        printf("\n>message received from server: %s\n",dest);
}

void blacklist(){
    char command[150];
    char dest[255];
    get_text_from_stdin(command, 150, "Please type the name you want to blacklist from server: blacklist <name> <password> <unwanted>\n");
    if(sendto(sock_UDP, command, sizeof(command), 0, (struct sockaddr*)&serv_UDP, sizeof(addr_UDP))>0)
    printf("Blacklist command sent ok\n");
    if(recvfrom(sock_UDP,(void *)&dest, sizeof(dest), 0, &exp, &exp_sz)>0)//if server replied
        printf("\n>message received from server: %s\n",dest);
}


void sendMail(mail* mail_content){
    char message_recu[255];
    if(CONNECTED){
        //we send the command of "SENDMAIL" to server
        char command[60]="SENDMAIL ";
        strcat(command, mail_content->src_nick);
        strcat(command, " ");
        strcat(command, mail_content->dst_nick);

        if(send(sock_TCP,command, 60, 0)>0){
         printf("SENDMAIL command sent correctly\n");
         if(recv(sock_TCP, message_recu, 255, 0)>0)//see what server replied
            printf(">message received from server: %s\n", message_recu);
        }else{
            printf("SENDMAIL command failed\n");
        }
        //sencondly,we send the mail
        char mail_str[500];
        mail_to_str(mail_content, mail_str);
        printf("mail_str is :\n%s, and sizeof it is %d\n", mail_str, sizeof(mail_str));
        int n;
        if((n =send(sock_TCP,(void *)mail_str, 500, 0))>0){
         if(recv(sock_TCP, message_recu, 255, 0)>0)//see what server replied
         printf(">message received from server: %s\n", message_recu);
        }else{
            printf("Mail sent failed\n");
        }
    }//end of if connected
    else{connection_tcp();}

}

list* receiveMailList(list* list)
{
    socklen_t sz=sizeof(serv_TCP);
    char connection[100];
    char message_recu[518];
    char mailstr[512];
    char command[60];
    mail* mail_content;
    //int num_of_mail;

    //we send the command
    if(CONNECTED)
    {
        printf("You are connected :)\n");
        get_text_from_stdin(command, 100,  "Get the maillist from server as the following format: get <nick> or check <nick> \n");
        //if check command
        if(command[0]=='c'){
            if(send(sock_TCP,command, sizeof(command), 0)>0){//if we sent correctly the command "CHECK"
                if(recv(sock_TCP, message_recu, 255, 0)>0){//if server replyed, i.e MAIL:X
                 printf(">message received from server: %s\n", message_recu);
               }else{printf("No response from server\n");}
            }else{printf("command can not be sent to server\n");}
        }


        //if get command
        if(command[0]=='g'){
            int wanted, i=0;
            printf("Input how many new messages you want to receive in INBOX\n");
            scanf("%d",&wanted);
            for(i=0; i<wanted; i++){//For each message the user want to receive
                    if(send(sock_TCP,command, sizeof(command), 0)>0){//if we sent correctly the command "GET"
                         //printf("GET command sent correctly\n");
                         if(recv(sock_TCP, message_recu, 512, 0)>0){//if server replyed, i.e MAIL:xxxx|xxxxx|xxxxx|xxxxx
                            //printf(">message received from server: %s\n", message_recu);
                            if(message_recu[0]!='E')//if there is message in server's inbox
                            {
                                get_field_by_number(message_recu, mailstr, 1, ':');
                                mail_content = (mail *)malloc(sizeof(mail));
                                str_to_mail(mailstr, mail_content);
                                list=append_top(list, mail_content);
                            }
                        }else{printf("No response from server\n");}
                    }else{printf("command GET sent failed\n");}
                }//end of for
            }
    }//End of IF CONNECTED
    else{connection_tcp();}

    return list;

}



void connection_tcp()
{
    close(sock_TCP);
    sock_TCP=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    addr_TCP.sin_port=htons(++port_TCP);
    if(bind(sock_TCP, (void* )&addr_TCP, sizeof(struct sockaddr_in))==0){
    printf("bind TCP OK\n");
    }else{
    perror("bind TCP fail or it already exists\n");
    }

    socklen_t sz=sizeof(serv_TCP);
    char connection[100];
    char message_recu[518];
     if(!CONNECTED){
            printf("You are not connected :(\n");
             if(connect(sock_TCP, (struct sockaddr_in *)&serv_TCP,sz)<0){//if connecting to server is not abled
                perror("connection TCP sending to server failed\n");
            }else{
                recv(sock_TCP, message_recu, 255, 0);//Se what server replied
                get_text_from_stdin(connection, 100,  "Please connect to the server with form: connect <nick> <pass>\n");
                 if(send(sock_TCP,connection, sizeof(connection), 0)>0){//if send "connection" command succeeded
                     if(recv(sock_TCP, message_recu, 255, 0)>0){//if server replyed
                         printf(">message received from server: %s\n", message_recu);
                         if(message_recu[0]=='C'){//if server replyed successfully connected
                            printf("connection TCP with server established\n");
                            CONNECTED=1;
                       }else{CONNECTED=0;}
                     }
                    }else{ printf("CONNECTION command sent failed\n");}

            }
        }
}

/*---------------------------------------- Main Function -------------------------------------------*/
int main(int argc, char **argv) {
  if(argc != 4) {
    printf("Erreur sur le nombre d'arguments\nLa commande a cette forme : ./client <IPserv> <TCPserv> <UDPserv>\n");
    return -1;
  }
  /* Put your code here */


//initialisation of TCP/IP
IP_server=argv[1];
port_TCP=atoi(argv[2]);
port_UDP=atoi(argv[3]);
printf("Ici c'est la cote client\nIP_server:%s, \nport_TCP: %d, \nport_UDP: %d\n", IP_server, port_TCP, port_UDP);
sock_UDP=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
sock_TCP=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(sock_UDP>0){printf("socket_UDP success\n");}else{printf("socket_UDP fail\n");}
if(sock_TCP>0){printf("socket_TCP success\n");}else{printf("socket_TCP fail\n");}
//set addr_UDP
addr_UDP.sin_family=AF_INET;
addr_UDP.sin_port=htons(port_UDP);
addr_UDP.sin_addr.s_addr=INADDR_ANY;
//set serv_UDP
serv_UDP.sin_family=AF_INET;
serv_UDP.sin_port=htons(port_UDP);
inet_pton(AF_INET, IP_server, &serv_UDP.sin_addr);

//set addr_TCP
addr_TCP.sin_family=AF_INET;
addr_TCP.sin_port=htons(port_TCP);
addr_TCP.sin_addr.s_addr=INADDR_ANY;

//set serv TCP
serv_TCP.sin_family=AF_INET;
serv_TCP.sin_port=htons(port_TCP);
inet_pton(AF_INET, IP_server, &serv_TCP.sin_addr);

if(bind(sock_UDP, (void* )&addr_UDP, sizeof(struct sockaddr_in))==0){
    printf("bind UDP OK\n");
}else{
    printf("bind UDP fail or it already exists\n");
}


if(bind(sock_TCP, (void* )&addr_TCP, sizeof(struct sockaddr_in))==0){
    printf("bind TCP OK\n");
}else{
    printf("bind TCP fail or it already exists\n");
}


/*---MAIN FUNCTIONNALITY--*/
//initialize inbox sendbox and composebox list;
inbox_list=create_list();
sendbox_list=create_list();
compose_list=create_list();
int connection=0;

printf("\n\n$$$$$$$$$$$$$$$$$$$$$$$<MAIL SYSTEM CLIENT>$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
printf("\n\n--List of COMMANDS--\n");
printf("<INBOX>--<SENDBOX>--<COMPOSEBOX>--<CONNECTION>--<COMPOSE>--<USERLIST>\n");
printf("<CONSULT-l-i>--<DELETE-l-i>--<SUBSCRIBE>--<UNSUBSCRIBE>--<BLACKLIST>--<QUIT>\n");

while(1){

 char command[100];
 char nom[50];
 char *tok;
 char box[25], num[25];
 int number;
 gets(command);//not scanf, since scanf will seperate the space" "
 if(strcmp(command, "INBOX")==0){
     inbox_list=receiveMailList(inbox_list);
     printf("The mail in your inbox are: \n");
     show_inbox(inbox_list);
}
 else if(strcmp(command, "SUBSCRIBE")==0){subscribe();}
 else if(strcmp(command, "UNSUBSCRIBE")==0){unsubscribe();}
 else if(strcmp(command, "BLACKLIST")==0){blacklist();}
 else if(strcmp(command, "SENDBOX")==0){show_sendbox(sendbox_list);}
 else if(strcmp(command, "USERLIST")==0){getUsernameList();}
 else if(strcmp(command, "COMPOSEBOX")==0){show_composebox(compose_list);}
else if(strcmp(command, "CONNECTION")==0){connection_tcp();}
 else if((command[0]=='C')&&(command[1]=='O')&&(command[2]=='N')){//if consult command
        tok = strtok (command," ,.-");//We pass by the first argument 'CONSULT'
        tok = strtok (NULL, " ,.-"); strcpy(box, tok);
        tok = strtok (NULL, " ,.-"); strcpy(num, tok);
        number=atoi(num);
        printf("You have chosen <%s>, mail <%d>\n", box, number);
        if(strcmp(box, "INBOX")==0){
            inbox_list=consult_inbox(number, inbox_list);
        }else if(strcmp(box, "SENDBOX")==0){
            sendbox_list=consult_sendbox(number, sendbox_list);
        }else{
            printf("We can not locate the type of mailbox that you reffered\n");
        }
 }
 else if((command[0]=='D') && (command[1]=='E') && (command[2]=='L')){//if delete command
        tok = strtok (command," ,.-");//We pass by the first argument 'CONSULT'
        tok = strtok (NULL, " ,.-"); strcpy(box, tok);
        tok = strtok (NULL, " ,.-"); strcpy(num, tok);
        number=atoi(num);
        printf("You have chosen <%s>, mail <%d>\n", box, number);
        if(strcmp(box, "INBOX")==0){
            inbox_list=delete_mail(inbox_list, number);
        }else if(strcmp(box, "SENDBOX")==0){
            sendbox_list=delete_mail(sendbox_list, number);
        }else{
            printf("We can not locate the type of mailbox that you reffered\n");
        }
     }
 else if(strcmp(command, "COMPOSE")==0){compose();}
 else if(strcmp(command, "QUIT")==0){close(sock_UDP);close(sock_TCP);exit(0);}
 else{printf("Wrong command, try again\n");}
}

/*----------------------------------------------------------------------------------------*/


return 0;//end of Main
}


