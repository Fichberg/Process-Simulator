char *get_cmd(char *, char *);
int expand(char *);
int cmd_ls(char *);
int cmd_cd(char *, char *);
int cmd_exit(char *);
int cmd_ep(char *);
int cmd_show(char *);
int cmd_pwd(char *, char *);
void unrecognized(char *);
void filter(char *, int, char *);
