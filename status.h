union sigval;
void notify (union sigval arg);
void handle_signal_std (int signo);

const char* hello_world(const char *fmt);
const char* datetime(const char *fmt);
const char* username(const char* fmt);
const char* run_command(const char* fmt);
