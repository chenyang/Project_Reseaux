#include "defs.h"
#define SADDRIN struct sockaddr_in
#define SADDR struct sockaddr

/* Put your functions here */
const char version[6]= "1.1.0\0";

typedef struct {
  char nick[NICKLEN];
  char pass[NICKLEN];
  SADDR * addr;
  int tcp;
  list * mails;
  list * black;
} user;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Purge
void purge(char * str) {
	int i;
	int n=strlen(str);
	for (i=0; i<n; i++) {
		if (str[i]=='\n') {
			str[i]='\0';
			i=n;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenUDP
int openUDP(int port, SADDRIN * addr, int * problem, int debug) {
	int sock;
	int warning=0;

	// sockaddr_in
	addr->sin_family= AF_INET;
	addr->sin_port=htons(port);
	addr->sin_addr.s_addr=INADDR_ANY;
	
	// socket UDP()
	if (debug) {
		printf("Creating UDP socket:          ");
		fflush(stdout);
	}
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) > 0) {
		if (debug) printf("OK\n");
	}else{
		if (debug) perror("ERROR");
		else perror("socketUDP()");
		*problem=1;
	}
	
		// bind()
	if (debug) {
		printf("Running UDP server:           ");
		fflush(stdout);
	}
	if (bind(sock, (void *) addr, sizeof(SADDRIN)) == 0) {
		if (debug) printf("OK\n");
	}else{
		if (debug) perror("ERROR");
		else perror("bindUDP()");
		warning=1;
	}
	
	if (warning == 1 || *problem == 1) {
		close(sock);
		sock = 0;
	}
	return sock;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenTCP
int openTCP(int port, SADDRIN * addr, int * problem, int debug, int maxUser) {
	int sock;
	int warning=0;

	// sockaddr_in
	addr->sin_family= AF_INET;
	addr->sin_port=htons(port);
	addr->sin_addr.s_addr=INADDR_ANY;
	
	// socket TCP()
	if (debug) {
		printf("Creating TCP socket:          ");
		fflush(stdout);
	}
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) > 0) {
		if (debug) printf("OK\n");
	}else{
		if (debug) perror("ERROR");
		else perror("socketTCP()");
		*problem=1;
	}
	
	if (debug) {
		printf("Running TCP server:           ");
		fflush(stdout);
	}
	if (bind(sock, (void *) addr, sizeof(SADDRIN)) == 0) {
		if (listen(sock, maxUser) == 0) {
			if (debug) printf("OK\n");
		}else{
			if (debug) perror("ERR02");
			else perror("listenTCP()");
			*problem=1;
		}
	}else{
		if (debug) perror("ERR01");
		else perror("bindTCP()");
		warning=1;
	}
	
	if (warning == 1 || *problem == 1) {
		close(sock);
		sock = 0;
	}
	return sock;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FindUser
user * finduser(list * userlist, char * nick) {
	if (userlist!=NULL && userlist->data!=NULL) {
		user * u;
		u = (user *) userlist->data;
		if (!strcmp(u->nick,nick)) {
			return u;
		}else{
			return finduser(userlist->next, nick);
		}
	}else
		return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteUser
list * deleteuser(list * userlist, char * nick) {
	int i;
	list * l;
	for (i=1 ; (l = get_position(userlist, i)) != NULL && strcmp(((user *)l->data)->nick, nick) ; i++);
	if (l == NULL)
		return userlist;
	else
		return delete_position(userlist, i);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IsBlack
int isblack(list * black, char * nick) {
	if (black!=NULL && black->data!=NULL) {
		if (!strcmp((char *) black->data,nick)) {
			return 1;
		}else{
			return isblack(black->next, nick);
		}
	}else
		return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DeleteBlack
list * deleteblack(list * black, char * nick) {
	int i;
	list * l;
	for (i=1 ; (l = get_position(black, i)) != NULL && strcmp((char *)l->data, nick) ; i++);
	if (l == NULL)
		return black;
	else
		return delete_position(black, i);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GetNickList
void getnicklist(list * userlist, char * nicklist) {
	if (userlist!=NULL && userlist->data!=NULL) {
		strcat(nicklist,((user *)userlist->data)->nick);
		strcat(nicklist," ");
		getnicklist(userlist->next,nicklist);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Disconnect
void disconnect(user * u, unsigned short * nbUser) {
	if (u->tcp != 0) {
		close(u->tcp);
		u->tcp = 0;
		*nbUser = *nbUser - 1;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProcessKeyboard
list * processKeyboard(	list * userlist, char * command, int cmd_sz,
								int udp, int * sockUDP, SADDRIN * addrUDP,
								int tcp, int * sockTCP, SADDRIN * addrTCP, unsigned short * maxUser,
								int debug,
								char * retour, unsigned short * nbUser, int * ret_sz, int * problem)
{
	char tmp[cmd_sz];
	int n;

	get_field_by_number(command, tmp, 0, ' ');
	strlwr(tmp);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USERS
	if (!strcmp(tmp,"help")) {
		*ret_sz=50;
		strcpy(retour,"Commands: help, quit, whois, close, open, maxuser");
	} else if (!strcmp(tmp,"quit")) {
		*problem=1;
		*ret_sz=18;
		strcpy(retour,"Closing server...");

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WHOIS
	} else if (!strcmp(tmp,"whois")) {
		get_field_by_number(command, tmp, 1, ' ');
		user * u;
		u = finduser(userlist, tmp);
		if (u!=NULL) {
			n = get_size(u->mails);
			*ret_sz=38+NICKLEN;
			if (u->tcp != 0)
				sprintf(retour,"User: %s\nMails: %d\nIP: %s",u->nick,n,inet_ntoa(((SADDRIN *)u->addr)->sin_addr));
			else
				sprintf(retour,"User: %s\nMails: %d\nIP: Not connected",u->nick,n);
		} else {
			*ret_sz=16;
			strcpy(retour,"Unknown user...");
		}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLOSE
	} else if (!strcmp(tmp,"close")) {
		get_field_by_number(command, tmp, 1, ' ');
		if (!strcmp(tmp,"subscription")) {
			if (*sockUDP != 0) {
				close(*sockUDP);
				*sockUDP = 0;
				*ret_sz=26;
				strcpy(retour,"Subscriptions are closed.");
			} else {
				*ret_sz=34;
				strcpy(retour,"Subscriptions are already closed.");
			}
		} else if (!strcmp(tmp,"connection")) {
			if (*sockTCP != 0) {
				user * u = finduser(userlist, "TMP");
				if (u!= NULL) {
					if (u->tcp == 0) disconnect(u, nbUser);
					userlist = deleteuser(userlist, "TMP");
				}
				close(*sockTCP);
				*sockTCP = 0;
				*ret_sz=24;
				strcpy(retour,"Connections are closed.");
			} else {
				*ret_sz=32;
				strcpy(retour,"Connections are already closed.");
			}
		} else {
			*ret_sz=27;
			strcpy(retour,"subscription or connection");
		}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OPEN
	} else if (!strcmp(tmp,"open")) {
		get_field_by_number(command, tmp, 1, ' ');
		if (!strcmp(tmp,"subscription")) {
			if (*sockUDP == 0) {
				*sockUDP=openUDP(udp,addrUDP,problem,0);
				if (*sockUDP != 0) {
					*ret_sz=26;
					strcpy(retour,"Subscriptions are opened.");
				} else {
					*ret_sz=39;
					strcpy(retour,"WARNING! Subscriptions are not opened.");
				}
			} else {
				*ret_sz=34;
				strcpy(retour,"Subscriptions are already opened.");
			}
		} else if (!strcmp(tmp,"connection")) {
			if (*sockTCP == 0) {
				*sockTCP=openTCP(tcp,addrTCP,problem,0,*maxUser);
				if (*sockTCP != 0) {
					*ret_sz=24;
					strcpy(retour,"Connections are opened.");
				} else {
					*ret_sz=37;
					strcpy(retour,"WARNING! Connections are not opened.");
				}
			} else {
				*ret_sz=32;
				strcpy(retour,"Connections are already opened.");
			}
		} else {
			*ret_sz=27;
			strcpy(retour,"subscription or connection");
		}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAXUSER
	} else if (!strcmp(tmp,"maxuser")) {
		if (*sockTCP == 0) {
			get_field_by_number(command, tmp, 1, ' ');
			n = atoi(tmp);
			if( n != 0)
				*maxUser = n;
			*ret_sz=14;
			sprintf(retour,"Max user: %d",*maxUser);
		} else {
			*ret_sz=57;
			strcpy(retour,"You have to close connections before. 'close connection'");
		}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unknown command
	}else{
		*ret_sz=21;
		strcpy(retour,"ERROR:Uknown command");
	}
	
	return userlist;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProcessUDP
list * processUDP(int sockUDP, char * command, int cmd_sz, SADDR * exp, socklen_t exp_sz, list * userlist, unsigned short * nbUser, int * problem) {
	char tmp[cmd_sz];

	get_field_by_number(command, tmp, 0, ' ');
	strlwr(tmp);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USERS
	if (!strcmp(tmp,"users")) {
		char nicklist[MESLEN];
		strcpy(nicklist,"USERLIST:");
		getnicklist(userlist,nicklist);
		if (sendto(sockUDP, nicklist , MESLEN, 0, exp, exp_sz) < 0){
			perror("processUDP:USERS:sendTo()");
			*problem=1;
		}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SUBSCRIBE
	}else if (!strcmp(tmp,"subscribe")) {
		char nick[NICKLEN];
		char pass[NICKLEN];
		get_field_by_number(command, nick, 1, ' ');
		get_field_by_number(command, pass, 2, ' ');
		if (finduser(userlist,nick) == NULL) {
			user * newuser = (user *) malloc(sizeof(user));
			strcpy(newuser->nick,nick);
			strcpy(newuser->pass,pass);
			newuser->addr = exp;
			newuser->tcp = 0;
			newuser->mails = create_list();
			newuser->black = create_list();
			userlist = append_top(userlist,newuser);
			char msg[33]="SUBSCRIBE:You are now signed in.";
			if (sendto(sockUDP, msg , 33, 0, exp, exp_sz) < 0){
				perror("processUDP:Subscribe:sendTo()");
				*problem=1;
			}
		}else{
			char error[38]="ERROR:This nickname is not available.";
			if (sendto(sockUDP, error , 38, 0, exp, exp_sz) < 0){
				perror("processUDP:ERROR:nickname:sendTo()");
				*problem=1;
			}
		}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UNSUBSCRIBE
	}else if (!strcmp(tmp,"unsubscribe")) {
		user * u;
		char nick[NICKLEN];
		char pass[NICKLEN];
		get_field_by_number(command, nick, 1, ' ');
		get_field_by_number(command, pass, 2, ' ');
		if ((u = finduser(userlist,nick)) != NULL) {
			if (!strcmp(u->pass,pass)) {
				if (u->tcp != 0) {
					disconnect(u,nbUser);
				}
				userlist = deleteuser(userlist, nick);
				char msg[21]="UNSUBSCRIBE:Goodbye!";
				if (sendto(sockUDP, msg , 21, 0, exp, exp_sz) < 0){
					perror("processUDP:Unsubscribe:sendTo()");
					*problem=1;
				}
			}else{
				char error[26]="ERROR:Incorrect password!";
				if (sendto(sockUDP, error , 26, 0, exp, exp_sz) < 0){
					perror("processUDP:ERROR:incorrect:sendTo()");
					*problem=1;
				}
			}
		} else {
			char error[36]="ERROR:This nickname does not exist!";
			if (sendto(sockUDP, error , 36, 0, exp, exp_sz) < 0){
				perror("processUDP:ERROR:nickname2:sendTo()");
				*problem=1;
			}
		}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BLACKLIST
	}else if (!strcmp(tmp,"blacklist")) {
		user * u;
		char nick[NICKLEN];
		char pass[NICKLEN];
		char black[NICKLEN];
		get_field_by_number(command, nick, 1, ' ');
		get_field_by_number(command, pass, 2, ' ');
		get_field_by_number(command, black, 3, ' ');
		if ((u = finduser(userlist,nick)) != NULL) {
			if (!strcmp(u->pass,pass)) {
				if (finduser(userlist,black) != NULL) {
					if (!isblack(u->black, black)) {
						u->black = append_top(u->black, black);
						char msg[33+NICKLEN];
						sprintf(msg,"BLACKLIST:New blacklisted user: %s",black);
						if (sendto(sockUDP, msg , 33+NICKLEN, 0, exp, exp_sz) < 0){
							perror("processUDP:Blacklist:sendTo()");
							*problem=1;
						}
					}else{
						char msg[31]="BLACKLIST:Already blacklisted!";
						if (sendto(sockUDP, msg , 31, 0, exp, exp_sz) < 0){
							perror("processUDP:Blacklist2:sendTo()");
							*problem=1;
						}
					}
				}else{
					char error[36]="ERROR:This nickname does not exist!";
					if (sendto(sockUDP, error , 36, 0, exp, exp_sz) < 0){
						perror("processUDP:ERROR:nickname4:sendTo()");
						*problem=1;
					}
				}
			}else{
				char error[26]="ERROR:Incorrect password!";
				if (sendto(sockUDP, error , 26, 0, exp, exp_sz) < 0){
					perror("processUDP:ERROR:incorrect:sendTo()");
					*problem=1;
				}
			}
		} else {
			char error[36]="ERROR:This nickname does not exist!";
			if (sendto(sockUDP, error , 36, 0, exp, exp_sz) < 0){
				perror("processUDP:ERROR:nickname3:sendTo()");
				*problem=1;
			}
		}
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unknown command
	}else{
		char error[21]="ERROR:Uknown command";
		if (sendto(sockUDP, error , 21, 0, exp, exp_sz) < 0){
			perror("processUDP:ERROR:Uknown:sendTo()");
			*problem=1;
		}
	}
	
	return userlist;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ProcessTCP
list * processTCP(list * userlist, char * command, int cmd_sz, user * u, unsigned short * nbUser, int debug) {
	char tmp[cmd_sz];

	get_field_by_number(command, tmp, 0, ' ');
	strlwr(tmp);
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONNECT
	if (!strcmp(tmp,"connect")) {
		if (strcmp(u->nick,"TMP")) {
			char error[33]="ERROR:You are already connected!";
			send(u->tcp, (void *) error, 33, 0);
		}else{
			char nick[NICKLEN];
			get_field_by_number(command, nick, 1, ' ');
			get_field_by_number(command, tmp, 2, ' ');
			user * real = finduser(userlist, nick);
			if (real != NULL && !strcmp(tmp,real->pass)) {
				if (real->tcp > 0) {
					char error[76]="ERROR:Someone has connected to your account. You are not connected anymore.";
					send(real->tcp, (void *) error, 76, 0);
					disconnect(real, nbUser);
				}
				real->addr = u->addr;
				real->tcp = u->tcp;
				userlist = deleteuser(userlist, "TMP");
				char msg[31]="CONNECT:You are now connected!";
				send(real->tcp, (void *) msg, 31, 0);
			}else{
				char error[34]="ERROR:Wrong nickname or password!";
				send(u->tcp, (void *) error, 34, 0);
				disconnect(u,nbUser);
				userlist = deleteuser(userlist, "TMP");
			}
		}
	} else {
	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOT CONNECTED USER
		if (!strcmp(u->nick,"TMP")) {
			char error[52]="ERROR:Wrong Command. You are not connected anymore.";
			send(u->tcp, (void *) error, 52, 0);
			disconnect(u,nbUser);
			userlist = deleteuser(userlist, "TMP");
		} else {
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SENDMAIL
			if (!strcmp(tmp,"sendmail")) {
				char exp[NICKLEN];
				char dest[NICKLEN];
				int n;
				user * v;
				mail * email;
				char tmp2[MESLEN*2];
				get_field_by_number(command, exp, 1, ' ');
				get_field_by_number(command, dest, 2, ' ');
				if (finduser(userlist, dest) != NULL) {
					char msg[14]="MAIL:Waiting.";
					send(u->tcp, (void *) msg, 14, 0);
					if((n = recv(u->tcp, (void *) tmp2, MESLEN*2, 0)) > 0) {
						if (debug) printf("MAIL:%s",tmp2);
						email = (mail *) malloc(sizeof(mail));
						str_to_mail(tmp2, email);
						if (!strcmp(exp,email->src_nick)) {
							if (!strcmp(dest,email->dst_nick)) {
								v = finduser(userlist, dest);
								if (!isblack(v->black,exp)) {
									v->mails = append_top(v->mails, email);
									char msg2[35]="MAIL:Mail successfully registered!";
									send(u->tcp, (void *) msg2, 35, 0);
								} else {
									char msg2[34]="MAIL:You have been blacklisted...";
									send(u->tcp, (void *) msg2, 34, 0);
								}
							} else {
								char error[25]="ERROR:Wrong target user!";
								send(u->tcp, (void *) error, 25, 0);
							}
						} else {
							char error[25]="ERROR:Trying to rape me?";
							send(u->tcp, (void *) error, 25, 0);
						}
					} else {
						printf("\n\nTOTO\n\n");
					}
				} else {
					char error[25]="ERROR:Wrong target user!";
					send(u->tcp, (void *) error, 25, 0);
				}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CHECK
			} else if (!strcmp(tmp,"check")) {
				get_field_by_number(command, tmp, 1, ' ');
				if (!strcmp(tmp,u->nick)) {
					if (is_empty(u->mails)) {
						char msg[13]="MAIL:0";
						send(u->tcp, (void *) msg, 13, 0);
					} else {
						int n;
						n = get_size(u->mails);
						char msg[10];
						sprintf(msg, "MAIL:%d",n);
						send(u->tcp, (void *) msg, 10, 0);
					}
				} else {
					char error[25]="ERROR:Trying to rape me?";
					send(u->tcp, (void *) error, 25, 0);
				}
				
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GET
			} else if (!strcmp(tmp,"get")) {
				get_field_by_number(command, tmp, 1, ' ');
				if (!strcmp(tmp,u->nick)) {
					if (is_empty(u->mails)) {
						char error[15]="ERROR:No mail!";
						send(u->tcp, (void *) error, 15, 0);
					} else {
						mail * email;
						list * l;
						char * msg = (char *) malloc(sizeof(char)*MESLEN*2);
						char tmp2[MESLEN*2];
						l = get_position(u->mails,1);
						email = (mail *) l->data;
						mail_to_str(email, msg);
						sprintf(tmp2,"MAIL:%s",msg);
						tmp2[(MESLEN*2)-1]='\0';
						send(u->tcp, (void *) tmp2, MESLEN*2, 0);
						u->mails = delete_position(u->mails,1);
					}
				} else {
					char error[25]="ERROR:Trying to rape me?";
					send(u->tcp, (void *) error, 25, 0);
				}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Unknown command
			} else {
				char error[21]="ERROR:Wrong Command.";
				send(u->tcp, (void *) error, 21, 0);
			}
		}
	}
	return userlist;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Main Function */
int main(int argc, char **argv) {

	/*
	 * DECLARING
	 */
	unsigned short tcp;
	unsigned short udp;
	unsigned short maxUser;
	unsigned short nbUser;
	int sockTCP;
	int sockUDP;
	int maxfd;
	int n;
	list * userlist;
	list * l;
	user * u;
	char buff[MESLEN*2];
	SADDRIN addrTCP;
	SADDRIN addrUDP;
	SADDR exp;
	socklen_t exp_sz;
	fd_set set;
	int problem=0;
	int debug=0;

  if(argc > 4) {
  	printf("Erreur sur le nombre d'arguments\nLa commande a cette forme : ./serveur <pTCP> <pUDP> <maxusers>\n");
  	return -1;
  }else{
		if (argc >= 4)
			maxUser=atof(argv[3]);
		else
			maxUser=MAXUSERS;
		if (argc >= 3)
			udp=atoi(argv[2]);
		else
			udp=UDPPORT;
		if (argc >=2) {
			if (argc==2 && !strcmp(argv[1],"debug")) {
  			debug=1;
  			tcp=TCPPORT;
  		}else{
				tcp=atoi(argv[1]);
			}
		}else
			tcp=TCPPORT;
  }
  
  /*
   * BEGINING
   */
  
  printf("\n\nRunning Serveur's version: %s\n\n",version);
  printf("Port TCP: %d\nPort UDP: %d\nMax user number: %d\n\n",tcp,udp,maxUser);
	
	/*
	 * INITATING
	 */
	nbUser=0;
	userlist = create_list();
	
	/*
	 * LISTENING
	 */
	sockUDP=openUDP(udp, &addrUDP, &problem, debug);
	if (!problem) sockTCP=openTCP(tcp, &addrTCP, &problem, debug,maxUser);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	 * RECEIVING/SENDING
	 */
	if (!problem && !debug) printf(">");
	fflush(stdout);
	while (!problem) {
	
		FD_ZERO(&set);
		if (sockUDP != 0)
			FD_SET(sockUDP,&set);
		if (sockTCP != 0)
			FD_SET(sockTCP,&set);
		FD_SET(0,&set);
		
		maxfd=(sockUDP>sockTCP?sockUDP:sockTCP);
		maxfd++;
		
		l = userlist;
		while(l != NULL && l->data != NULL) {
			u = (user *) l->data;
			if (u->tcp != 0) {
				FD_SET(u->tcp, &set);
				if (maxfd <= u->tcp)
					maxfd = (u->tcp)+1;
			}
			l=l->next;
		}
		
		if (select(maxfd,&set,NULL,NULL,NULL) < 0) {
			if (debug) perror("Select procedure failed       ERROR");
			else perror("select()");
			problem=1;
		}else{
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// UDP Transactions
			if (sockUDP != 0 && FD_ISSET(sockUDP,&set)) {
				if (debug) {
					printf("Receiving UDP packet:         ");
					fflush(stdout);
				}
				exp_sz = sizeof(exp);
				if ((n = recvfrom(sockUDP, (void*) buff, OBJLEN, 0, &exp, &exp_sz)) >= 0) {
					if (n >= OBJLEN)
						buff[OBJLEN-1]='\0';
					else
						buff[n]='\0';
					purge(buff);
					if (debug) printf("OK:%s\n",buff);
					userlist = processUDP(sockUDP, buff, n, &exp, exp_sz, userlist, &nbUser, &problem);
				}else{
					if (debug) perror("ERROR");
					else perror("recvfrom()");
					problem=1;
				}
			}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TCP Transactions
			else if (sockTCP != 0 && FD_ISSET(sockTCP,&set)) {
				if (debug) {
					printf("Receiving TCP packet:         ");
					fflush(stdout);
				}
				exp_sz=sizeof(SADDR);
				if ((n = accept(sockTCP, &exp, &exp_sz)) > 0) {
					if (nbUser < maxUser && finduser(userlist, "TMP") == NULL) {
						nbUser++;
						strcpy(buff, "Please authenticate with: connect <nick> <pass>\nOtherwise, you will be disconnected.");
						if (send(n, (void *) buff, 85, 0) > 0) {
							u = (user *) malloc(sizeof(user));
							strcpy(u->nick,"TMP");
							u->addr = &exp;
							u->tcp = n;
							userlist = append_top(userlist, (void *) u);
							if (debug) printf("OK:NewUser %d\n",n);
						}else{
							if (debug) perror("ERROR");
							else perror("sendTCP()");
						}
						strcpy(buff, "NULL");
					}else{
						if (nbUser < maxUser)
							strcpy(buff, "ERROR:Please try again...");
						else
							strcpy(buff, "ERROR:User limit reached!");
						if (send(n, (void *) buff, 26, 0)) {
							if (debug) printf("OK:%s\n",buff);
						}else{
							if (debug) perror("ERROR");
							else perror("sendTCP()");
						}						
						close(n);
					}
				} else {
					if (debug) perror("ERROR");
					else perror("recvTCP()");
					problem = 1;
				}
			}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////	
// Keyboard Transaction
			else if (FD_ISSET(0,&set)) {
				if (debug) {
					printf("Processing:                   ");
					fflush(stdout);
				}
				get_text_from_stdin(buff, OBJLEN, NULL);
				n=0;
				userlist = processKeyboard(userlist, buff, OBJLEN, udp, &sockUDP, &addrUDP, tcp, &sockTCP, &addrTCP, &maxUser, debug, buff, &nbUser, &n, &problem);
				if (debug) printf("OK\n");
				if (n>0) printf("%s\n",buff);
				if (!problem && !debug) {
					printf(">");
					fflush(stdout);
				}
			}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client TCP transaction			
			else {
			
				if (debug) {
					printf("Receiving client TCP packet:  ");
					fflush(stdout);
				}
			
				l = userlist;
				if (l != NULL && l->data != NULL) {
					do {
						u = (user *) l->data;
						l = l->next;
					} while(l != NULL && l->data != NULL && !FD_ISSET(u->tcp, &set));
				}

				if (u->tcp != 0 && FD_ISSET(u->tcp, &set)) {
					if((n = recv(u->tcp, (void *) buff, MESLEN*2, 0)) == 0) {
					// This user is disconnected
						if (debug) printf("Connection closed:%s\n",u->nick);
						disconnect(u,&nbUser);
						if (!strcmp(u->nick,"TMP")) {
							userlist = deleteuser(userlist, "TMP");
						}
					} else {
						if (debug) {
							printf("RECV%d:%s",n,buff);
							fflush(stdout);
						}
						userlist = processTCP(userlist, buff, n, u, &nbUser, debug);
						if (debug) printf("/SEND:%s\n",buff);
					}
				}else {
					if (debug) printf("ERROR:Unknown error\n");
					else printf("FD_ISSET():Unknown error\n");
					problem=1;
				}
			}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


		}
	}
		
	
	/*
	 * CLOSING
	 */		
	 
	if (debug) printf("Closing servers:              ");
	if (sockUDP != 0) close(sockUDP);
	if (sockTCP != 0) close(sockTCP);
	if (debug) printf("OK\n");

	printf("\n\n\n");
	
	return 0;
}
