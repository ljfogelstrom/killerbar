union sigval;
void notify (union sigval arg);
void handle_signal_std (int signo);

const char* hello_world(const char *fmt);
const char* datetime(const char *fmt);
const char* username(const char* fmt);
const char* run_command(const char* fmt);
const char* cpu_perc(const char* unused);
const char* disk_free(const char* path);
const char* disk_used(const char* path);
const char* disk_total(const char* path);
const char* temp(const char* file);
const char* cat(const char* file);
const char* ram_used(const char* unused);
const char* ram_total(const char* unused);
