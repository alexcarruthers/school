void mksfs(int fresh);
void sfs_ls(void);
int sfs_open(char *name);
int sfs_close(int fileID);
int sfs_write(int fileID, char *buf, int length);
int sfs_read(int fileID, char *buf, int length);



