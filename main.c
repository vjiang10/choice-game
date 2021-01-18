#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include "functions.h"

char currentaddress[256]; // To keep track of what node we're currently at.
int autosave = 0; // If 0, then autosave is off. If 1, then autosave is on.

// Function to save the game.
void saveGame() {
  printf("Saved Game!\n");
  int fd = open("savefile.txt", O_WRONLY);
  write(fd, currentaddress, sizeof(currentaddress));
  close(fd);
  printf("Input choice #: ");
}

// Function to autosave. Basically saveGame() but without the text prompts. (It happens in the background).
void autoSave() {
  int fd = open("savefile.txt", O_WRONLY);
  write(fd, currentaddress, sizeof(currentaddress));
  close(fd);
}

// Function to ask the player if they have installed ImageMagick.
void promptImageMagick() {
  printf("\nNote: WSL users may have to run this program through ssh (i.e. via PuTTY and enabling X11 forwarding) and install an X server (i.e. Xming)\n");
  printf("Linux/MacOS users usually have an inbuilt X server: able to run this program right away\n");
  printf("\nNote: make sure to install ImageMagick before running this program\n");
  printf("To install: $ sudo apt-get install imagemagick\n");
  printf("Have you installed ImageMagick? (y/n)\n");
  char promptResponse[10];
  fgets(promptResponse, sizeof(promptResponse), stdin);

  // If the player types anything other than 'y' or 'n'
  if (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
    while (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
      if (!strcmp(promptResponse, "help\n")) {
        help();
        printf("\nHave you installed ImageMagick? (y/n)\n");
      }
      else {
        printf("Invalid input. Type 'y' or 'n'.\n");}
        fgets(promptResponse, sizeof(promptResponse), stdin);
    }
  }

  // If the player types 'n'
  else if (promptResponse[0] == 'n') {
    printf("\nPlease install ImageMagick to run this program\n");
    printf("You can remove ImageMagick after using: $ sudo apt-get remove imageiagick\n");
    exit(0);
  }
}

// Function to prompt the player if they want to enable autosave. Enables autosave if desired.
void promptAutosave() {
  char promptResponse[256];
  fgets(promptResponse, sizeof(promptResponse), stdin);

  // If the player types anything other than 'y' or 'n'
  if (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
    while (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
      if (!strcmp(promptResponse, "help\n")) {
        help();
        printf("\nWould you like to enable autosave? (y/n)\n");
      }
      else {printf("Invalid input. Type 'y' or 'n'.\n");}
      fgets(promptResponse, sizeof(promptResponse), stdin);
    }
  }

  // If the player types 'y', enable autosave.
  if (promptResponse[0]=='y') {
    autosave = 1;
    printf("Autosave enabled.\n\n");
  }

  // If the player types 'n', disable autosave.
  if (promptResponse[0]=='n') {
    printf("Autosave disabled.\n\n");
  }
}

// Function to prompt the player if they want to load a savefile.
void promptLoadfile(char *buffer, char *buffer2) {
  char promptResponse[256];
  fgets(promptResponse, sizeof(promptResponse), stdin);

  // If the player types anything other than 'y' or 'n'
  if (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
    while (strcmp(promptResponse, "y\n")&&strcmp(promptResponse, "n\n")) {
      if (!strcmp(promptResponse, "help\n")) {
        help();
        printf("\nWould you like to load in a saved file? (y/n)\n");
      }
      else {printf("Invalid input. Type 'y' or 'n'.\n");}
      fgets(promptResponse, sizeof(promptResponse), stdin);
    }
  }

  char address[10];

  // If the player types 'y', attempt to load the savefile.
  if (promptResponse[0]=='y') {
    printf("\nLooking for save file...\n");
    int fd = open("savefile.txt", O_RDONLY);
    read(fd, address, sizeof(address));
    close(fd);

    // If savefile.txt doesn't start with E, then a save file exists.
    if (address[0]!='E') {
      struct stat sb;
      stat("./savefile.txt", &sb);
      printf("Found save file, last saved on: %s", ctime(&(sb.st_mtime)));
      printf("Loading from last saved checkpoint...\n");
    }

    // If savefile.txt starts with E, then no save file exists.
    else {
      printf("You have no save file. Loading from beginning of the game...\n");
    }
  }

  // If the player types 'n', then the program automatically comes here.
  address[0] = '0'; // If savefile.txt only had an 'E', then this replaces that 'E' with a '0'.
                    // If savefile.txt had a save file, then this wouldn't change anything.
  struct Node node = makeNode(address, buffer, buffer2); // Load in the game.

}

// print and summarize features
void help() {
  printf("------------------------------------------\n");
  printf("type \"quit\" or \"exit\" to quit the game\n");
  printf("type \"back\" to move back to last scene\n");
  printf("type \"save\" to save the game at current scene\n");
  printf("type \'Ctrl \\\' to save the game and quit \n");
  printf("--------------------------------------------\n");
}

// function which makes the choice (takes in stdin int)
int makeChoice(int numChoice) {
    printf("Input choice #: ");
    char choice [256];
    fgets(choice, sizeof(choice), stdin);

    // If the player types 'restart'
    if (!strcmp(choice, "restart\n")) {
      printf("Are you sure you want to start from the beginning? (y/n)\n");
      fgets(choice, sizeof(choice), stdin);
      if (choice[0] == 'y') return 0;
      else {return makeChoice(numChoice);}
    }

    // If the player types 'help'
    if (!strcmp(choice, "help\n")) {
      help();
      return makeChoice(numChoice);
    }

    // If the player types 'quit'
    if (!strcmp(choice, "quit\n") || !strcmp(choice, "exit\n")) {
      printf("\nHope you had fun!\n");
      exit(0);
    }

    // If the player types 'save'
    if (!strcmp(choice, "save\n")) {
      while (!strcmp(choice, "save\n")) {
        saveGame();
        fgets(choice, sizeof(choice), stdin);
      }
    }

    // If the player types 'back'
    if (!strcmp(choice, "back\n")){
      if (!strcmp(currentaddress, "0")) {
        printf("Can't go back. You're at the beginning!\n");
        return makeChoice(numChoice);
      }
      else return atoi("10");
    }

    else {
      while (atoi(choice) < 1 || atoi(choice) > numChoice) {
          printf("Input invalid. Please try again: ");
          fgets(choice, sizeof(choice), stdin);
      }
    }

    return atoi(choice);
}

// returns whether there is a picture file at address
char reader0(char * address, char * buffer) {
  char *x = strstr(buffer, address);
  x+=strlen(address);
  return x[0];
}

// prints the text from story.txt at address
void reader(char * address, char * buffer){
    char add [256];
    strcpy(add, address);
    strcat(add, " ");
    char *x = strstr(buffer, add);
    x+=strlen(address);
    int i; for (i = 0; x[i] != '|'; x++); x++;
    int k; for (k = 0; x[k] != '|'; k++) printf("%c", x[k]);
    printf("\n");
}

// second reader; returns the number of choices at address
int reader2(char * address, char * buffer) {
    char *x = strstr(buffer, address);
    x+=strlen(address);
    int i; for (i = 0; x[i] != ':'; x++); x++;
    return atoi(x);
}

static void sighandler(int signo){
    if (signo == SIGQUIT) {
      printf("\n");
      saveGame();  // Ctrl + "\"
    }
}

char ** parse_args(char * line) {
  char **args = malloc(sizeof(char*)*100);
  char *p = line;

  int i;
  for (i=0; p!=NULL; i++) {
    args[i] = strsep(&p, " ");
  }
  args[i] = NULL;
  return args;
}

// function for displaying images
void display(char * address) {
  char line[256] = "display -resize 500x500 pics/";
  strcat(line, address); strcat(line, ".jpg");
  printf("the display command: %s", line);
  char **args = parse_args(line);
  execvp(args[0], args);
}

// makes node and links to next node recursively
struct Node makeNode(char str [256], char * buffer, char * buffer2) {
    struct Node node;
    strcpy(node.address, str);
    strcpy(currentaddress, node.address); // Sets the "currentaddress" (global String) to this node's address.
    if (autosave) autoSave();
    reader(node.address, buffer);
    if ('y' == reader0(str, buffer2)) {
      int f, status;
      f = fork();
      // child process displaying images
      if (!f) display(str);
      // parent process waiting for child process
      else {int childpid = waitpid(f, &status, 0);}
    }
    int len = strlen(str);
    // in the future, not using len but actually checking last char in string
    // if particular char, say T, terminate and initiate end game function
    if (len == 10) exit(0); // for now it's exit, but we can add a special function (endgame()) that ends the game
    char add[256], choice[10];
    strcpy(add, str);

    signal(SIGQUIT, sighandler);

    // reads from buffer2 the number of choices with address
    int numChoice = reader2(node.address, buffer2);
    sprintf(choice, "%d", makeChoice(numChoice));
    if (!strcmp(choice, "10")) add[strlen(add)-1] = 0;
    else {strcat(add, choice);}
    if (!strcmp(choice, "0")) makeNode("0", buffer, buffer2);
    else {makeNode(add, buffer, buffer2);}
}

int main() {
    // loads in story.txt into a buffer
    char buffer[5000];
    int fd = open("story.txt", O_RDONLY);
    read(fd, buffer, sizeof(buffer));
    close(fd);
    // loads numChoice.txt into another buffer
    char buffer2[256];
    int fd2 = open("numChoice.txt", O_RDONLY);
    read(fd2, buffer2, sizeof(buffer2));
    close(fd2);

    // Check if the player has installed ImageMagick:
    promptImageMagick();

    printf("\nNOTE: type in \"help\" anytime expand program features and utilities\n");
    while (1) {
      // Autosave Prompt:
      printf("\nWould you like to enable autosave? (y/n)\n");
      promptAutosave();

      // Loadfile Prompt: (If they don't want to load, the game starts at the beginning.)
      printf("Would you like to load in a saved file? (y/n)\n");
      promptLoadfile(buffer, buffer2);
    }

    return 0;
  }
