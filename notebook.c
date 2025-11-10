//Include Statements
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>


//Structures
typedef struct User{
	char *email;
	char *password;
} User;

typedef struct Notebook{
	char *name;
	struct Notebook* nextNotebook;
} Notebook;

typedef struct Note{
	char *name;
	struct Note* nextNote;
} Note;

//Global Variables
DIR *current_directory = NULL;

//Prototypes
int checkUser(char* username, char* password);
void makeUser(char* username, char* password);
void makeNotebook(char* notebook_name);
void makeNote(char* note);

//Main function
int main(){
	//sets the current directory to the working directory
	current_directory = opendir(".");
	if(!current_directory){
		printf("Error launching the active directory.\n");
		return 1;
	}
	char *username_test = malloc(15 * sizeof(char));
	strcpy(username_test, "user@gmail.com");
	char *password_test = malloc(9 * sizeof(char));
	strcpy(password_test, "PaSSW1!");
	printf("%d", checkUser(username_test, password_test));	
}

/**
*Verifies the user email and password meet requirements and gives all corresponding error messages
*Username is an email which must contain an '@' character
*Username must not be used by another account
*Password must have one uppercase letter and one lowercase letter
*Password must contain a number and special character
*Password must be between 7 and 28 characters
*Returns 1 on successful creation or 0 on a failed creation
*/
int checkUser(char* username, char* password){
	//Creates a flag that is set to 0 if there is ever an error in email or password
	int big_flag = 1;
	//Username and Password verification
	//Checks for an email domain
	if(strchr(username, '@') == NULL){
		printf("Email must contain an email domain (@domain.com).\n");
		big_flag = 0;
	}
	//Checks if email is a repeat
	/*doing this later*/
	
	//Checks if password contains an uppercase letter
	int small_flag = 0;
	for(int i = 0; i < strlen(password); i++){
		if(isupper((unsigned char)password[i])){
			small_flag = 1;
		}
	}
	if(!small_flag){
		printf("Password must contain at least one uppercase letter.\n");
		big_flag = 0;
	}
			
	//checks if the password contains a lowercase letter
	small_flag = 0;
	for(int i = 0; i < strlen(password); i++){
		if(islower((unsigned char)password[i])){
			small_flag = 1;
		}
	}
	if(!small_flag){
		printf("Password must contain at least one lowercase letter.\n");
		big_flag = 0;
	}
	
	//Checks if the password contains a number
	small_flag = 0;
	//Iterates through each numeric character (48-57)
	for(int i = 48; i <= 57; i++){
		if(strchr(password, (char)i) != NULL){
			small_flag = 1;
			break;
		}
	}
	//Gives an error if there was no numeric character
	if(!small_flag){
		printf("Password must contain at least one number.\n");
		big_flag = 0;
	}
	
	//Checks the password for a special character (33-47, 58-64, and 91-96)
	small_flag = 0;
	//Iterates through the first character group (33-47)
	for(int i = 33; i <= 47; i++){
		if(strchr(password, (char)i) != NULL){
			small_flag = 1;
			break;
		}
	}
	//Iterates through the second character group (58-64)
	if(!small_flag){
		for(int i = 58; i <= 64; i++){
			if(strchr(password, (char)i) != NULL){
				small_flag = 1;
				break;
			}
		}
	}
	//Iterates through the third character group (91-96)
	if(!small_flag){
		for(int i = 91; i <= 96; i++){
			if(strchr(password, (char)i) != NULL){
				small_flag = 1;
				break;
			}
		}
	}
	//Gives an error if there was no special character
	if(!small_flag){
		printf("Password must contain at least one special character.\n");
		big_flag = 0;
	}
	
	//Checks password length
	if(strlen(password) < 7 || strlen(password) > 28){
		printf("Password must be between 7 and 28 characters.\n");
		big_flag = 0;
	}
	return big_flag;
}
		

