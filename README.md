# Unix-Shell
A basic Shell written in C that can handle I/O redirection and pipes.<br>
<br>
### The shell can handle the commands like this example <br/>
`/bin/ls | /bin/sort | /bin/uniq | /usr/bin/wc -l 2>&1 1>output.txt`

### The following functionalities are supported : - <br>
Syntax              | Meaning
------------        | -------------
command             | executes the command and waits for the command to finish, prints error message if the command is invalid
command>filename    | redirect stdout to file filename.  If the file does not exist create one, otherwise, overwrite the existing file
command >> filename | If the filename already exists append the stdout output, otherwise, create a new file
1>filename          | redirect stdout to filename
2>filename          | redirect stderr to filename
2>&1                | redirect stderr to stdout
command<filename    | use file descriptor 0 (stdin) for filename.  If command tries to read from stdin, effectively it will read from filename.
\|                  | pipe command 
exit                | exit from the shell program
