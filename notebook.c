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
int makeUser(char* username, char* password);
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
	char *username_test = malloc(40 * sizeof(char));
	strcpy(username_test, "notascam@rnicrosoft.com");
	char *password_test = malloc(40 * sizeof(char));
	strcpy(password_test, "Get5cammed!");
	printf("%d\n", checkUser(username_test, password_test));
	printf("%d", makeUser(username_test, password_test));
	free(password_test);
	free(username_test);
}

/**
*Verifies the user email and password meet requirements and gives all corresponding error messages
*Username is an email which must contain an '@' character with a '.' character after
*Username is between 5 and 320 characters
*Username must not have any odd characters (listed below)
*Username must not be used by another account
*Password must have one uppercase letter and one lowercase letter
*Password must contain a number and special character
*Password must be between 7 and 28 characters
*Returns 1 on successful creation or 0 on a failed creation
*/
//folders cannot contain \/*?"<>|
int checkUser(char* username, char* password){
	//Creates a flag that is set to 0 if there is ever an error in email or password
	int big_flag = 1;
	int username_length = strlen(username);
	int password_length = strlen(password);
	//Username and Password verification
	//Checks for an email domain
	if(strchr(username, '@') == NULL){
		printf("Email must contain a valid email domain (@domain.com).\n");
		big_flag = 0;
	}
	//ensures there is a . after the @. Runs separate to avoid checking a NULL string
	else if(strchr(strchr(username, '@'), '.') == NULL){
		printf("Email must contain a valid email domain (@domain.com).\n");
		big_flag = 0;
	}
	
	//Checks for valid email length (5-320 characters)
	if(username_length < 5 || username_length > 320){
		printf("Email must be between 5 and 320 characters.\n");
		big_flag = 0;
	}
	
	//Checks if the username contains an unusable character for a directory name (\, /, *, ?, ", <, >, |, :)
	//Also checks for other odd characters that may cause issues (', ,, ;, (, ), [, ], [space])
	for(int i = 0; i < username_length; i++){
		if(strchr("\\/*?\"<>|:',;()[] ", username[i])){
			printf("Email must not contain unusual characters.\n");
			big_flag = 0;
			break;
		}
	}
		
	//Checks if email is a repeat
	//Opens the Users directory and verifies success
	DIR *users = opendir("./Users");
	if(users == NULL){
		printf("ERROR: Failed to check other users.\n");
		big_flag = 0;
	}
	else{
		//Reads the subdirectories of Users into a dirent that will include the subdirectory names
		struct dirent *existing_usernames = readdir(users);
		while(existing_usernames != NULL){
			//Checks if the name of the subdirectory matches the username
			//Subdirectories will be named with the username, so this checks that there is no overlap
			if(strcmp(existing_usernames->d_name, username) == 0){
				printf("An account already exists with this email.\n");
				big_flag = 0;
				break;
			}
			existing_usernames = readdir(users);
		}
		closedir(users);
	}
	
			
	
	
	//Checks if password contains an uppercase letter
	int small_flag = 0;
	for(int i = 0; i < password_length; i++){
		if(isupper((unsigned char)password[i])){
			small_flag = 1;
			break;
		}
	}
	if(!small_flag){
		printf("Password must contain at least one uppercase letter.\n");
		big_flag = 0;
	}
			
	//checks if the password contains a lowercase letter
	small_flag = 0;
	for(int i = 0; i < password_length; i++){
		if(islower((unsigned char)password[i])){
			small_flag = 1;
			break;
		}
	}
	if(!small_flag){
		printf("Password must contain at least one lowercase letter.\n");
		big_flag = 0;
	}
	
	//Checks if the password contains a number
	small_flag = 0;
	//Iterates through each numeric character (48-57)
	for(int i = 0; i < password_length; i++){
		if(isdigit((unsigned char)password[i])){
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
	if(password_length < 7 || password_length > 28){
		printf("Password must be between 7 and 28 characters.\n");
		big_flag = 0;
	}
	return big_flag;
}
		
/**
*Creates the file structure for a new user if the username and password meets requirements
*File structure is a directory named with the user's email and a text file within the directory containing the username and password
*Notebooks will also be added to this text file, but not when the profile is made
*returns 1 on success or 0 on failure
*/
int makeUser(char* username, char* password){
	if(checkUser(username, password)){
		//Puts the file path for the user directory into a string
		char *path = malloc((9 + strlen(username)) * sizeof(char));
		if(path == NULL){
			printf("Memory allocation failed.\n");
			return 0;
		}
		strcpy(path, "./Users/");
		strcat(path, username);
		//Makes a directory and checks for success
		/*
		0755 is used to give only the owner write access
		Group members and others can only read and execute files
		If code doesn't work, try changing to 0777 so all profiles have access, or making sure you are the file's owner
		*/
		if(mkdir(path, 0755) == 0){
			//Make the user profile file
			//changes path to point to a user profile path
			path = realloc(path, (strlen(path) + 18) * sizeof(char));
			if(path == NULL){
				printf("Memory allocation failed.\n");
				return 0;
			}
			strcat(path, "/User_Settings.txt");
			//fopen creates a file if it does not exist
			FILE *user_settings = fopen(path, "w");
			if(user_settings == NULL){
				printf("Failed to create user settings.\n");
				return 0;
			}
			fprintf(user_settings, "Email: %s\n", username);
			fprintf(user_settings, "Password: %s\n", password);
			fclose(user_settings);
			free(path);
			return 1;
		}
		//Return an error if mkdir fails
		else{
			printf("An unexpected error occured creating the user directory.\n");
			return 0;
		}
	}
	//Return an error if the username and password do not meet conditions
	//No error message is printed as checkUser will give error messages
	return 0;
}
		