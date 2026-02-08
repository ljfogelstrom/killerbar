struct Blocks block[] = {
    /* function         argument				    format          interval , initial value      */
    { datetime,         "%F %T",				    "| %s",         TIMER(1, 1)	    },
    { run_command,      "setvolume",				    "| vol: %s",    TIMER(0, 3)	    },
    { temp,		"/sys/class/thermal/thermal_zone2/temp",    "%sC",	    TIMER(10, 1)    },
    { cpu_perc,         NULL,					    "%s",           TIMER(10, 5)    },
    { ram_total,        NULL,					    "%s |",	    TIMER(20, 2)    },
    { ram_used,         NULL,					    "MEM: %s /",    TIMER(20, 2)    },
};

/* optional: delimiter for each field */
static char* delimiter = " ";
