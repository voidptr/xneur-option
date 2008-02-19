typedef void (*sg_handler)(int);

char* copy_pchar(const char *source, int len = 0, bool setnull = false);
void error(const char *msg, ...);
void warning(const char *msg, ...);
void trap_signal(int sig, sg_handler handler);
void send_signal(int pid, int sig);
