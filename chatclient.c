// Taylor Martin

// Starter code from http://www.linuxhowtos.org/C_C++/socket.html 
// as suggested by Alex during class

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <curses.h>
#include <fcntl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>  

char *temp;
char temp2[256];

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

static void linehandler(char *line)
{
  if(line != NULL){
    temp = line;
  }
}

int main(int argc, char *argv[])
{
  int sockfd, stdinfd, portno, ret;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];
  fd_set rifds, rofds;
  rl_initialize();
  rl_prep_terminal(0);
  initscr();

  // handle the command line argument to use it as the username
  if(argc != 2) error("Must provide username");
  else strcpy(buffer, argv[1]);
  buffer[strlen(buffer)] = '\n';
 
  stdinfd = 0;
  portno = 49153;

  // create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  // get host
  server = gethostbyname("pilot");
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
	(char *)&serv_addr.sin_addr.s_addr,
	server->h_length);
  serv_addr.sin_port = htons(portno);
  
  // connect to host
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR connecting");
  
  // send username and reset buffer
  ret = write(sockfd,buffer,strlen(buffer));
  if (ret < 0) 
    error("ERROR sending username");
  bzero(buffer,256);
;
  FD_ZERO(&rifds);
  FD_SET(sockfd, &rifds);
  FD_SET(stdinfd, &rifds);
  
  while (1){
    // make a temporary fd_set to use with select()
    memcpy(&rofds, &rifds, sizeof(rofds));
    
    ret = select(sockfd+1, &rofds, NULL, NULL, NULL);
    if(ret==-1) error("select error");

    // install the line handler
    rl_callback_handler_install(NULL,linehandler);

    // check to see if there is a message ready to be read
    if(FD_ISSET(sockfd, &rofds)){
            
      // iterate through the message in progress if one exists
      nodelay(stdscr,TRUE);
      noecho();
      char g;
      g = getch();
      if(g!=ERR){
	ungetch(g);
	getstr(temp2);
      	refresh();
	/*
	move(getcury(stdscr)-2, 0);
	refresh();
	deleteln();
	clrtobot();
	refresh();
	*/
      }
      
      rl_callback_handler_remove();

      //read the current message from server
      ret = read(sockfd, buffer, sizeof(buffer));
      if(ret==-1) error("error receiving message");
      else if(ret==0) exit(0);
      else{
	// print the message from the server
	buffer[ret] = 0;
	printf("%s", buffer);
      }
      // if a message was cleared, re-type it below the message from server
      if(temp2[1]!=0){
	printf("\n");
	addstr(temp2);
	refresh();
	  
      }
    
      bzero(temp2,256);
      bzero(buffer,256);
    }
  
    // check to see if there is a message ready to be sent
    if(FD_ISSET(stdinfd, &rofds)){
      fgets(buffer,255,stdin);
      if (strlen(buffer) > 1){
	ret = write(sockfd, buffer, strlen(buffer));
      }
      bzero(buffer,256);
    }
  }
  close(sockfd);
  return 0;
}
