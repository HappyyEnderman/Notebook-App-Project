//Include Statements
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>


//Structures
/*A general structure for notes and notebooks
*Name is the title of the notebook or contents of the note
*Time_created is an integer time that can easily be compared
*/
typedef struct Writing{
	time_t time_created;
	char* name;
} Writing;

/*Structure to store the active user
*Login information (mostly unused)
*filepaths to the user directory and the open notebook
*arrays containing notes and notebooks
*number of notes and notebooks
*/
typedef struct User{
	char *email;
	char *password;
	char *filepath;
	Writing *notebooks;
	int notebook_count;
	char *notebook_path;
	Writing *notes;
	int note_count;
} User;




//Enums
//An enum to mark what layer of the structure the user is in
typedef enum Position{
	ENTRY,
	USER,
	NOTEBOOKS
}Position;
//Global Variables

//Prototypes
int directoryExists(char* directory_name, char* current_directory);

int checkUser(char* username, char* password);
int makeUser(char* username, char* password);
int deleteUser(User *user);
void logout(User *user);
User login(char* username, char* password);

void makeNotebook(char* notebook_name);
int loadNotebooks(User *user, FILE *settings);
void makeNote(char* note);

//Main function
int main(){
	//variables to control the interface
	int running = 1;
	int input = 0;
	Position position = ENTRY;
	User user = {0};
	//User Interface for the system
	printf("——Welcome to EasyNote——\n");
	while(running){
		switch(position){
			//Login Page Menu
			case ENTRY:
			printf("Please select a menu option:\n");
			printf("1. Log in\n");
			printf("2. Sign up\n");
			printf("3. Exit Program\n");
			scanf("%d", &input);
			
			switch(input){
				case 1: { //Braces are used to limit the scop of the email and password variables
					char *email = malloc(322 * sizeof(char)); // 322 allows for null terminator and to detect if it is over 320 characters
					char *password = malloc(30 * sizeof(char));
					printf("Enter email adress: ");
					scanf("%321s", email);
					printf("Enter password: ");
					scanf("%29s", password);
					//Attempts login
					if(checkUser(email, password)){
						user = login(email, password);
						//Checks if login was successful befor moving to user level
						if(strlen(user.email) != 0){
							position = USER;
						}
					}
					break;
				}
				case 2: {
					//Prompts for email and password_length
					char *email = malloc(322 * sizeof(char)); // 322 allows for null terminator and to detect if it is over 320 characters
					char *password = malloc(30 * sizeof(char));
					printf("Enter email adress: ");
					scanf("%321s", email);
					printf("Enter password: ");
					scanf("%321s", password);
					//Creates user account
					makeUser(email, password);
					free(email);
					free(password);
					//Changes interface level to user
					position = USER;
					break;
				}
				case 3:
					running = 0;
					break;
				default:
					printf("Input not recognized\n");
					break;
			}
			break;
			
			//Notebooks and Account Deletion Menu
			case USER:
				printf("Please select a menu option:\n");
				printf("1. UNDER CONSTRUCTION\n");
				printf("2. Logout\n");
				printf("3. Delete Account\n");
				scanf("%d", &input);
				switch(input){
				case 1:
					printf("UNDER CONSTRUCTION\n");
					break;
				
				case 2:
					logout(&user);
					position = ENTRY;
					break;
				case 3:
					deleteUser(&user);
					position = ENTRY;
					break;
				default:
					printf("Input not recognized\n");
					break;
			}
				break;
				
			//Create and Edit Notes Menu
			case NOTEBOOKS:
				printf("Under Construction: Returning to login screen\n");
				position = ENTRY;
				break;
				
			//This should be impossible to reach, but protects against edge cases with the user position
			default:
				printf("An unknown error has occurred. Returning to login screen. All unsaved progress will be lost.\n");
				position = ENTRY;
				break;
		}
	}
}

/*
*Checks if a directory exists within the current directory
*Returns 1 if it exists, 0 if it doesn't, and -1 if there was an error opening the directory
*/
int directoryExists(char* directory_name, char* current_directory){
	//Opens the current directory and verifies success
	DIR *directory = opendir(current_directory);
	if(directory == NULL){
		return -1;
	}
	else{
		//Reads the subdirectories into a dirent that will include the subdirectory names
		struct dirent *existing_dir = readdir(directory);
		while(existing_dir != NULL){
			//Checks if the name of a subdirectory matches the requested directory name
			if(strcmp(existing_dir->d_name, directory_name) == 0){
				closedir(directory);
				return 1;
				break;
			}
			existing_dir = readdir(directory);
		}
		closedir(directory);
	}
	return 0;
}

/*
*Verifies the user email and password meet requirements and gives all corresponding error messages
*Username is an email which must contain an '@' character with a '.' character after
*Username is between 5 and 320 characters
*Username must not have any odd characters (listed below)
*Password must have one uppercase letter and one lowercase letter
*Password must contain a number and special character
*Password must be between 7 and 28 characters
*Returns 1 on successful creation or 0 on a failed creation
*/
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
		
/*
*Creates the file structure for a new user if the username and password meets requirements
*File structure is a directory named with the user's email and a text file within the directory containing the username and password
*Notebooks will also be added to this text file, but not when the profile is made
*Email must not exist for this email already
*returns 1 on success or 0 on failure
*/
int makeUser(char* username, char* password){
	if(checkUser(username, password)){
		
		//Checks if email is a repeat
		int flag = directoryExists(username, "./Users");
		if(flag == -1){
			printf("Failed to check existing users. Please try again.\n");
			return 0;
		}
		else if(flag == 1){
			printf("An account already exists with that email.\n");
			return 0;
		}
		
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
			//Creates the user Notebooks subdirectory
			char *notebook_path = malloc((11 + strlen(path)) * sizeof(char));
			if(notebook_path == NULL){
				printf("Memory allocation failed.\n");
				return 0;
			}
			strcpy(notebook_path, path);
			strcat(notebook_path, "/Notebooks");
			if(mkdir(notebook_path, 0755) == -1){
				perror("Failed to create notebooks subdirectory");
				return 0;
			}
			
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
				perror("Failed to create user settings");
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
			perror("Failed to make user directory");
			return 0;
		}
	}
	//Return an error if the username and password do not meet conditions
	//No error message is printed as checkUser will give error messages
	return 0;
}

/*
*Removes all files under the user directory
*Removes the user directory
*Logs user out of account
*/
int deleteUser(User *user){
	//Creates a filepath to the notebooks directory
	char *notebook_path = malloc ((11 + strlen(user->filepath)) * sizeof(char));
	if(notebook_path == NULL){
		printf("Memory allocation failed.\n");
		return 0;
	}
	strcpy(notebook_path, user->filepath);
	strcat(notebook_path, "/Notebooks");
	//Opens the notebooks directory
	DIR *notebooks_folder = opendir(notebook_path);
	if(notebooks_folder == NULL){
		perror("Failed to open directory");
		free(notebook_path);
		return 0;
	}
	
	//Removes all files within the notebooks directory
	struct dirent *existing_files = readdir(notebooks_folder);
	while(existing_files != NULL){
		if(strcmp(existing_files->d_name, ".") != 0 && strcmp(existing_files->d_name, "..") != 0){
			char *filepath = malloc((strlen(existing_files->d_name) + strlen(notebook_path) + 2) * sizeof(char));
			if(filepath == NULL){
				printf("Memory allocation failed.\n");
				free(filepath);
				return 0;
			}
			strcpy(filepath, notebook_path);
			strcat(filepath, "/");
			strcat(filepath, existing_files->d_name);
			if(remove(filepath) != 0){
				free(filepath);
				perror("Failed to remove notebook");
			}
			free(filepath);
		}
		existing_files = readdir(notebooks_folder);
	}
	closedir(notebooks_folder);
	
	//Deletes the notebooks directory
	if(rmdir(notebook_path) != 0){
		perror("Failed to remove directory");
	}
	free(notebook_path);
	
	//Deletes user settings
	char *settings_path = malloc((strlen(user->filepath) + 18) * sizeof(char));
	strcpy(settings_path, user->filepath);
	strcat(settings_path, "/User_Settings.txt");
	if(remove(settings_path) != 0){
		perror("Failed to remove user settings");
		free(settings_path);
	}
	free(settings_path);
	//Deletes the user path
	if(rmdir(user->filepath) != 0){
		perror("Failed to remove user directory");
	}
	//logout(user);
	return 1;
}
		
/*
*Attempts to log the user into an existing account
*Returns a User struct with the user information
*Loads the notebooks associated with the user (NOT DONE YET)
*/
User login(char* username, char* password){
	User output;
	//Checks if the user account exists as a directory
	if(directoryExists(username, "./Users") == 1){
		//Create the path to the user's settings file and open it
		char *filepath = malloc((27 + strlen(username))*sizeof(char));
		strcpy(filepath, "./Users/");
		strcat(filepath, username);
		
		//Sets the user path to the account's directory
		output.filepath = malloc((strlen(filepath) + 1) * sizeof(char));
		if(output.filepath == NULL){
			free(filepath);
			printf("Failed to allocate memory");
		}
		strcpy(output.filepath, filepath);
		//Finishes setting the filepath to user settings
		strcat(filepath, "/User_Settings.txt");
		FILE *user_settings = fopen(filepath, "r");
		free(filepath);
		//Ensures no error occurs in opening the file
		if(user_settings == NULL){
			logout(&output);
			printf("An error occurred when checking account's password.\n");;
			//Returns a null user. This needs checked outside the function.
			return output;
		}
		
		//Reads the username and password of the account
		/*IMPORTANT: The space at the start of the inputs from fscanf allows the program to skip whitespace.
		Without it, the newline would be read immediately when checking password, giving it no value.*/
		output.email = malloc(321 * sizeof(char));
		if(output.email == NULL){
			logout(&output);
			printf("Failed to allocate memory.\n");
			return output;
		}
		fscanf(user_settings, " Email: %320s", output.email);
		output.password = malloc(29 * sizeof(char));
		if(output.password == NULL){
			logout(&output);
			printf("Failed to allocate memory.\n");
			return output;
		}
		fscanf(user_settings, " Password: %28s", output.password);
		
		//Reconfirms email matches and checks the password
		//Clears previous data and returns an error if either the username or password don't match
		if(!(strcmp(output.password, password) == 0 && strcmp(output.email, username) == 0)){
			logout(&output);
			fclose(user_settings);
			printf("Email or password does not match.\n");
			return output;
		}
		//Reads the notebooks from user settings and stores them
		if(loadNotebooks(&output, user_settings) == 1){
			fclose(user_settings);
			return output;
		}
		//Returns an error if loadNotebooks fails
		else{
			logout(&output);
			printf("Failed to load notebooks.\n");
			return output;
		}
	}
	//Returns an error if no directory exists
	//A real system would want to use a more general error message for user privacy, but here it is helpful for identifying where things fail, so it has been kept
	printf("No account exists with that email.\n");
	return output;
}

//Clears all contents of the User argument
void logout(User *user){
	free(user->email);
	free(user->password);
	free(user->filepath);
	free(user->notebooks);
	free(user->notebook_path);
	free(user->notes);
	user->notebook_count = 0;
	user->note_count = 0;
}

/*
*UNIMPLEMENTED
*Loads the notebook names from the user settings 
*Notebooks are put into a currently uncreated struct within users to efficiently store notes and such
*Current thought is that it will only be called in login, so you can count on the first 2 lines having been read already
*Returns 1 on success and 0 on failure
*IMPORTANT: If read from directories, order will not be kept, so it is important to read from the user settings list instead
*/
int loadNotebooks(User *user, FILE *settings){
	return 1;
}