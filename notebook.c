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
    char* content;   
} Writing;

/*Structure to store the active user
*Login information (mostly unused)
*filepaths to the user directory and the open notebook
*arrays containing notes and notebooks
*number of notes and notebooks
*Note: notes_path, notes, and note_count are only set if a notebook is openned
*/
typedef struct User{
    char *email;
    char *password;
    char *filepath;
    Writing *notebooks;
    int notebook_count;
    char *notes_path;
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
//General Functions
int directoryExists(char* directory_name, char* current_directory);

//User Management Functions
int checkUser(char* username, char* password);
int makeUser(char* username, char* password, User* user);
int deleteUser(User *user);
User login(char* username, char* password);
void logout(User *user);

//Notebook Management Functions
int makeNotebook(char* notebook_name, User* user);
int loadNotebooks(User *user, char *filepath);
int makeNote(char* note_title, char* content, User* user);
int loadNotes(User *user, const char *notebook_filename);
int saveNotebook(User *user);
int editNote(User *user, int index, const char *new_title, const char *new_content);
int deleteNote(User *user, int index);

//Main function
int main(){
	// Variables to control the interface
	int running = 1;
	int input = 0;
	Position position = ENTRY;
	User user = (User){0}; // = {0} is used to ensure memory can be freed later without causing problems; pointers are set to NULL and ints are set to 0;
	//User Interface for the system
	printf("——Welcome to EasyNote——\n");
	while(running){
		switch(position){
			// LOGIN MENU
			case ENTRY:
			printf("Please select a menu option:\n");
			printf("1. Log in\n");
			printf("2. Sign up\n");
			printf("3. Exit Program\n");
			scanf("%d", &input);
			
			switch(input){
				case 1: { //Braces are used to limit the scop of the email and password variables
					char *email = malloc(130 * sizeof(char)); // 130 allows for null terminator and to detect if it is over 128 characters
					char *password = malloc(30 * sizeof(char));
					printf("Enter email adress: ");
					scanf("%129s", email);
					printf("Enter password: ");
					scanf("%29s", password);
					//Attempts login
					if(checkUser(email, password)){
						user = login(email, password);
						//Checks if login was successful befor moving to user level
						if(user.email != NULL){
							position = USER;
						}
					}
					break;
				}
				case 2: {
					//Prompts for email and password_length
					char *email = malloc(130 * sizeof(char)); // 130 allows for null terminator and to detect if it is over 128 characters
					char *password = malloc(30 * sizeof(char));
					printf("Enter email adress: ");
					scanf("%129s", email);
					printf("Enter password: ");
					scanf("%29s", password);
					//Creates user account
					if(makeUser(email, password, &user)){
						//Changes interface level to user
						position = USER;
					}
					free(email);
					free(password);
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
			
			// USER MENU
			case USER:
				printf("Please select a menu option:\n");
				printf("1. Create Notebook\n");
				printf("2. Load Notebook\n");
				printf("3. Logout\n");
				printf("4. Delete Account\n");
				scanf("%d", &input);
				
				switch(input){
				case 1:{
					//Scans notebook name
					char *notebook_name = malloc(66 * sizeof(char));
					printf("Enter Notebook Name: ");
					scanf("%65s", notebook_name);
					makeNotebook(notebook_name, &user);
					free(notebook_name);
					break;
				}
				case 2: {
					if(user.notebook_count == 0){
						printf("No notebooks available.\n");
						break;
					}
					printf("Your notebooks:\n");
					for(int i=0;i<user.notebook_count;i++){
						printf("%d. %s (created %ld)\n", i+1, user.notebooks[i].name, (long)user.notebooks[i].time_created);
					}
					int pick;
					printf("Enter the number of the notebook you want to open: ");
					scanf("%d", &pick);
					if (pick < 1 || pick > user.notebook_count){
						printf("Invalid selection\n"); 
						break;
					}
					// build notebook filename path
					char notebook_file[1024];
					snprintf(notebook_file, sizeof(notebook_file), "%s/Notebooks/%s.txt", user.filepath, user.notebooks[pick-1].name);
					if(!loadNotes(&user, notebook_file)){
						printf("Failed to load notebook\n");
						break;
					}
					position = NOTEBOOKS;
					break;
				}
				case 3:
					logout(&user);
					position = ENTRY;
					break;
				case 4:{
					//Confirms account deletion
					char confirmation;
					printf("Are you sure you want to delete your account (Y/N): ");
					scanf(" %c", &confirmation);
					if(toupper(confirmation) == 'Y'){
						//Deletes the account
						if(deleteUser(&user)){
							printf("Account deletion successful\n");
							position = ENTRY;
						}
						else{
							printf("Account deletion partially or entirely failed\n");
						}
					}
					else{
						printf("Account deletion cancelled\n");
					}
					break;
				}
				default:
					printf("Input not recognized\n");
					break;
			}
				break;
				
			// NOTEBOOK MENU
			case NOTEBOOKS:
				printf("\nNotebook Menu\n");
				printf("1. View Notes\n");
				printf("2. Create Note\n");
				printf("3. Exit Notebook\n");
				scanf("%d", &input);

				switch(input){
					case 1:
						if(user.note_count == 0){
							printf("No notes available. Create a note to view notes.\n");
						} else {
							for(int i=0;i<user.note_count;i++){
								printf("%d. %s\n", i+1, user.notes[i].name);
							}
						}
						break;

					case 2: {
						char title[256];
						char content[5000];
						content[0] = '\0';

						printf("Enter note title: ");
						scanf("%255s", title);

						printf("Enter note content. Enter a single '.' on a line to finish:\n");

						getchar();
						// An infinite loop that reads line from the user until they enter "."
						while (1) {
							char line[512];
							if (!fgets(line, sizeof(line), stdin)) {
								break;
							}
							if (strcmp(line, ".\n") == 0 || strcmp(line, ".\r\n") == 0 || strcmp(line, ".") == 0)
								break;

							strcat(content, line);
						}

						if(!makeNote(title, content, &user)){
							printf("Failed to make note.\n");
						}
						break;
					}

					case 3:
						free(user.notes);
						user.notes = NULL;
						user.note_count = 0;
						position = USER;
						break;

					default:
						printf("Invalid input.\n");
				}
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
*Username is between 5 and 128 characters
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
	
	//Checks for valid email length (5-128 characters)
	if(username_length < 5 || username_length > 128){
		printf("Email must be between 5 and 128 characters.\n");
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
			
	//Checks if the password contains a lowercase letter
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
int makeUser(char* username, char* password, User* user){
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
			//sets the user variables. Skips notebook related variables because 
			*user = login(username, password);
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
	User output = {0};
	
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
		output.email = malloc(129 * sizeof(char));
		if(output.email == NULL){
			logout(&output);
			printf("Failed to allocate memory.\n");
			return output;
		}
		fscanf(user_settings, " Email: %128s", output.email);
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
		if(loadNotebooks(&output, output.filepath) == 1){
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
	free(user->notes_path);
	free(user->notes);
	*user = (User){0};
}

/*
*Notebook Title may not exceed 64 characters
*Checks if the notebook already exists. If not, conitnues to next steps.
*Makes a notebook in memory
*Appends the notebook to Notebook_List.txt
*Creates a .txt file with the notebook's name as the name of the .txt file
*Returns 1 on success and 0 on failure
*/
int makeNotebook(char* notebook_name, User* user){
	if(strlen(notebook_name) > 64){
		printf("Notebook name cannot exceed 64 characters");
		return 0;
	}
	//Makes a directory path to the notebooks folder for checking if the notebook name is taken
	char *notebook_path = malloc((strlen(user->filepath) + strlen("/Notebooks") + 1) * sizeof(char));
	if(notebook_path == NULL){
		printf("Memory allocation failed.\n");
		return 0;
	}
	strcpy(notebook_path, user->filepath);
	strcat(notebook_path, "/Notebooks/");
	//Adds a .txt to the notebook name
	char *new_notebook_name = malloc((strlen(notebook_name) + 5) * sizeof(char));
	if(new_notebook_name == NULL){
		printf("Memory allocation failed.\n");
		free(notebook_path);
		return 0;
	}
	strcpy(new_notebook_name, notebook_name);
	strcat(new_notebook_name, ".txt");
	//Returns an error if the notebook name already exists
	if(directoryExists(new_notebook_name, notebook_path)){
		printf("You cannot have two notebooks with the same name.\n");
		free(notebook_path);
		return 0;
	}
	
	
	//Appends the notebook and time of creation to Notebook_List.txt
	//Creates the path to the notebook list
	char *list_path = malloc((strlen(user->filepath) + strlen("/Notebook_List.txt") + 1) * sizeof(char));
	if(list_path == NULL){
		printf("Memory allocation failed.\n");
		free(notebook_path);
		return 0;
	}
	strcpy(list_path, user->filepath);
	strcat(list_path, "/Notebook_List.txt");
	FILE *list = fopen(list_path, "a");
	if(list == NULL){
		printf("banana");
		perror("Failed to open file");
		free(notebook_path);
		free(list_path);
		return 0;
	}
	
	//Updates the user with new notebook
	//Increases the number of notebooks
	user->notebook_count++;
	user->notebooks = realloc(user->notebooks, user->notebook_count * sizeof(Writing));
	if(user->notebooks == NULL){
		printf("Failed to make new notebook. Reloading from drive.\n");
		loadNotebooks(user, user->filepath);
		free(notebook_path);
		free(list_path);
		fclose(list);
		return 0;
	}
	
	//Sets the notebook name in memory
	user->notebooks[user->notebook_count-1].name = malloc((strlen(notebook_name) + 1) * sizeof(char));
	if(user->notebooks[user->notebook_count-1].name == NULL){
		printf("Failed to make new notebook. Reloading Notebooks from drive.\n");
		loadNotebooks(user, user->filepath);
		free(notebook_path);
		free(list_path);
		fclose(list);
		return 0;
	}
	strcpy(user->notebooks[user->notebook_count-1].name, notebook_name);
	//Sets the notebook creation time in memory
	user->notebooks[user->notebook_count-1].time_created = time(NULL);
	
	//Appends the name and creation time to the notebook list, followed by an empty line
	fprintf(list, "Name: %s\n", notebook_name);
	fprintf(list, "Time Created: %ld\n", user->notebooks[user->notebook_count-1].time_created);
	fprintf(list, "\n");
	fclose(list);
	free(list_path);
	
	//Creates the notebook text file to store notes
	notebook_path = realloc(notebook_path, (strlen(notebook_path) + strlen(new_notebook_name) + 2) * sizeof(char));
	if(notebook_path == NULL){
		printf("Memory allocation failed.\n");
		free(new_notebook_name);
		return 0;
	}
	strcat(notebook_path, "/");
	strcat(notebook_path, new_notebook_name);
	FILE *notebook = fopen(notebook_path, "a");
	free(new_notebook_name);
	free(notebook_path);
	if(notebook == NULL){
		perror("Failed to create file");
		return 0;
	}
	fclose(notebook);
	printf("Notebook created successfully\n");
	return 1;
}
/*
*UNIMPLEMENTED
*Loads the notebook names from the user settings 
*Notebooks are put into a currently uncreated struct within users to efficiently store notes and such
*Current thought is that it will only be called in login, so you can count on the first 2 lines having been read already
*Returns 1 on success and 0 on failure
*IMPORTANT: If read from directories, order will not be kept, so it is important to read from the user settings list instead
*/
int loadNotebooks(User *user, char* filepath){
	// If the notebook doesn't exist, return 0.
    if (user == NULL || filepath == NULL) {
		return 0;
	}

    // Create the path to notebook list.
    char *list_path = malloc(strlen(filepath) + strlen("/Notebook_List.txt") + 1);
    if(!list_path) {
		return 0;
	}
    strcpy(list_path, filepath);
    strcat(list_path, "/Notebook_List.txt");

	// Open notebook list.
    FILE *f = fopen(list_path, "r");
    if(!f){
        // If the file for the notebook doesn't exist, then it's empty.
        user->notebook_count = 0;
        user->notebooks = NULL;
        free(list_path);
        return 1;
    }

    // Clear notebooks that are currently loaded.
    for(int i = 0; i < user->notebook_count; i++) {
        free(user->notebooks[i].name);
    }
	
    free(user->notebooks);
    user->notebooks = NULL;
    user->notebook_count = 0;
	
	// Set up variables for reading the notebook list.
    char *line = NULL;
    size_t len = 0;
    size_t read;

    char name_buffer[256] = {0};
    long time_value = 0;

	// Read the notebook list file.
    while((read = getline(&line, &len, f)) != -1){
        if(strncmp(line, "Name:", 5) == 0) {
            // If the name is found, parse the notebook name.
            const char *p = line + 5;
			
            while(*p == ' ') { 
				p++;
			}
			
            strncpy(name_buffer, p, sizeof(name_buffer) - 1);
            name_buffer[sizeof(name_buffer)-1] = '\0';
			
            // Read the time the notebook was created. If there is no time, then the time is sent to the current time.
            if((read = getline(&line, &len, f)) == -1) {
				break;
			}
			
            if(strncmp(line, "Time Created:", 13) == 0){
                const char *tptr = line + 13;
                while(*tptr == ' ') tptr++;
                time_value = atol(tptr);
            } else {
                time_value = time(NULL);
            }
			
            // Store the notebook in memory.
            user->notebook_count++;
            user->notebooks = realloc(user->notebooks, user->notebook_count * sizeof(Writing));
			
            if(!user->notebooks){
                perror("Failed to reallocate.");
                fclose(f);
                if(line) free(line);
                free(list_path);
                return 0;
            }
			
            int index = user->notebook_count - 1;
            user->notebooks[index].time_created = (time_t)time_value;
            user->notebooks[index].name = malloc(strlen(name_buffer)+1);
            strcpy(user->notebooks[index].name, name_buffer);
            user->notebooks[index].content = NULL; // The content of a notebook is set to null because only notes will have content.
        }
    }

    if(line) {
		free(line);
	}
	
    fclose(f);
    free(list_path);
    return 1;
}

int loadNotes(User *user, const char *notebook_filename){
    if(!user || !notebook_filename) {
		return 0;
	}
	// Open the notebook file.
    FILE *f = fopen(notebook_filename, "r");
    if(!f) {
        // If there is no notebook file, then the notebook is empty.
        user->note_count = 0;
        user->notes = NULL;
        // Create the path for notes.
        free(user->notes_path);
        user->notes_path = malloc(strlen(notebook_filename) + 1);
        strcpy(user->notes_path, notebook_filename);
        return 1;
    }

    // Clear existing notes.
    if (user->notes) {
        for(int i = 0; i < user->note_count; i++){
            free(user->notes[i].name);
            free(user->notes[i].content);
        }
        free(user->notes);
        user->notes = NULL;
        user->note_count = 0;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char title[257];
    long t_created = 0;
    // Create a temporary buffer that stores notes content.
    size_t content_buf_size = 4096;
    char *content_buf = malloc(content_buf_size);
    if(!content_buf){
        fclose(f);
        return 0;
    }

    while((read = getline(&line, &len, f)) != -1){
		// Scan to see if there are any new notes.
        if(strncmp(line, "Title:", 6) == 0){
			chomp(line); // This is a function used to remove trailing newlines.
            const char *p = line + 6;
            while(*p == ' ') {
				p++;
			}
            strncpy(title, p, sizeof(title)-1);
            title[sizeof(title)-1] = '\0';

            // Read the time created.
            if((read = getline(&line, &len, f)) == -1) {
				break;
			}
            chomp(line);
            if(strncmp(line, "Time Created:", 13) == 0){
                const char *tptr = line + 13;
                while(*tptr == ' ') + {
					tptr++;
				}
                t_created = atol(tptr);
            } else {
				t_created = time(NULL);
			}

            // Parse the note content.
            if((read = getline(&line, &len, f)) == -1) break;
            chomp(line);
            // Read the content of the note until it reaches "ENDNOTE".
            content_buf[0] = '\0';
            size_t content_len = 0;
            while((read = getline(&line, &len, f)) != -1){
                chomp(line);
                if(strcmp(line, "ENDNOTE") == 0) {
					break;
				}
                size_t line_len = strlen(line);
				// This line expands the buffer if necessary.
                if(content_len + line_len + 2 > content_buf_size){
                    content_buf_size *= 2;
                    content_buf = realloc(content_buf, content_buf_size);
                    if(!content_buf){
                        fclose(f);
                        if(line) {
							free(line);
						}
                        return 0;
                    }
                }
				// Add a new line to each line.
                memcpy(content_buf + content_len, line, line_len);
                content_len += line_len;
                content_buf[content_len++] = '\n';
                content_buf[content_len] = '\0';
            }

            // Store the note in the memory.
            user->note_count++;
            user->notes = realloc(user->notes, user->note_count * sizeof(Writing));
            int idx = user->note_count - 1;
            user->notes[idx].time_created = (time_t)t_created;
            user->notes[idx].name = malloc(strlen(title) + 1);
            strcpy(user->notes[idx].name, title);
            user->notes[idx].content = malloc(content_len + 1);
            memcpy(user->notes[idx].content, content_buf, content_len+1);
        }
    }

    free(content_buf);
    if(line) {
		free(line);
	}
    fclose(f);

    // Store the path for notes.
    free(user->notes_path);
    user->notes_path = malloc(strlen(notebook_filename) + 1);
    strcpy(user->notes_path, notebook_filename);

    return 1;
}

int saveNotebook(User *user){
    if(!user || !user->notes_path) return 0;
    FILE *f = fopen(user->notes_path, "w");
    if(!f) return 0;
    for(int i=0;i<user->note_count;i++){
		// All of the code for saving the notes content.
        fprintf(f, "Title: %s\n", user->notes[i].name ? user->notes[i].name : "");
        fprintf(f, "Time Created: %ld\n", (long)user->notes[i].time_created);
        fprintf(f, "Content:\n");
        if(user->notes[i].content) fputs(user->notes[i].content, f);
        if(user->notes[i].content && user->notes[i].content[strlen(user->notes[i].content)-1] != '\n') fputc('\n', f);
        fprintf(f, "ENDNOTE\n");
    }
    fclose(f);
    return 1;
}

int makeNote(char* note_title, char* content, User* user){
    // Check if it's possible to make the note.
    if(!user || !user->notes_path || !note_title) {
        return 0;
    }

    // Add the note to memory.
    user->note_count++;
    user->notes = realloc(user->notes, user->note_count * sizeof(Writing));
    if(!user->notes) {
        return 0;
    }

    int idx = user->note_count - 1;

    // Allocate and copy the title.
    user->notes[idx].name = malloc(strlen(note_title) + 1);
    strcpy(user->notes[idx].name, note_title);

    // Set creation time.
    user->notes[idx].time_created = time(NULL);

    // Allocate and copy content.
    if(content) {
        user->notes[idx].content = malloc(strlen(content) + 1);
        strcpy(user->notes[idx].content, content);
    } else {
        user->notes[idx].content = malloc(1);
        user->notes[idx].content[0] = '\0';
    }

    // Save notebook after adding note.
    if(!saveNotebook(user)) {
        return 0;
    }

    return 1;
}

int editNote(User *user, int index, const char *new_title, const char *new_content){
    if(!user || index < 0 || index >= user->note_count) {
        return 0;
    }

    // Edit the title.
    free(user->notes[index].name);
    user->notes[index].name = malloc(strlen(new_title)+1);
    strcpy(user->notes[index].name, new_title);
	// Edit the content.
    free(user->notes[index].content);
    user->notes[index].content = malloc(strlen(new_content)+1);
    strcpy(user->notes[index].content, new_content);

    return saveNotebook(user);
}

int deleteNote(User *user, int index){
    if(!user || index < 0 || index >= user->note_count) {
        return 0;
    }
	// Free the notes memory
    free(user->notes[index].name);
    free(user->notes[index].content);

    // Reposition the remaining notes.
    for(int i = index; i < user->note_count - 1; i++){
        user->notes[i] = user->notes[i+1];
    }

    // Update count.
    user->note_count--;
    if(user->note_count == 0){
        free(user->notes);
        user->notes = NULL;
    } else {
        user->notes = realloc(user->notes, user->note_count * sizeof(Writing));
    }

    return saveNotebook(user);
}