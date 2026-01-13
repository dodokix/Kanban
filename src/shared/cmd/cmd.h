#define CMD_BUFF_SIZE 64

/*
gestisce i comandi che possono essere mandati al server da client e tastiera
return: -1 errore, 0 se tutto ok
*/
int handle_command(int sock, const char* cmd);