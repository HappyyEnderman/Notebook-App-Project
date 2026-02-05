# Notebook-App-Project

A theoretical notebook app meant to run in command line interface and simulate a web based notes system

## To Use

- Download the files and a C compiler such as cygwin with gcc
- Compile notebook.c using your C compiler
- Run the created .exe file within the compiler terminal

## Functionality

Users create accounts which are stored on local files. There are various requirements for the email and password used to make sure the file system works, the password is secure, and the email is one that might reasonably exist.

Within accounts, users create notebooks to store their notes. These notes can be reordered and kept between program executions.

Notes are made within accounts and contain a title and content. The content may have multiple lines. When the program sorts notes alphabetically, the content isn't taken into consideration by the program.
