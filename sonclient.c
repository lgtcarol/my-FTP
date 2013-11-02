void trvdir( char path[])
{
	DIR 			*dir;
	struct stat 		det;
	struct dirent 		*det1;
	char 			filenames[256][PATH_MAX];
	int 			i = 0;

//	printf("~~~~~~~~~~~~~~~~~~~~trvdir遍历目录并写文件~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	if((dir = opendir(path)) == NULL)
	{
		perror("opendir");
		return ;
	}
	chdir(path);
	while((det1 = readdir(dir)) != NULL )
	{
		if(strcmp(det1->d_name, ".") == 0 || strcmp(det1->d_name, "..") == 0 )
		      continue;
		strcpy(filenames[i++], det1->d_name);
		if(lstat(det1->d_name, &det) == -1)
		{
			perror("lstat");
			return ;
		}
		if(S_ISDIR(det.st_mode))
		     trvdir(det1->d_name);
	}
	display(filenames, i, path);
	chdir("..");
	closedir(dir);
}
/*将所有信息写入文件g_lsbuf中，避免来回写时出错*/
void display(char filenames[][PATH_MAX], int n, char path[])
{
	int 			i, k, j = 0;
	int 			linemax = 0, nlinemax;
	int 			fd;
	for(i = 0; i < n; i++)
	{
		if(strlen(filenames[i]) > linemax)
		      linemax = strlen(filenames[i]); 				//获取最长文件名的长度
	}
	nlinemax = LINEMAX / (linemax + 2);
	printf("%s\n", path);
	for(i = 0; i < n; i++)
	{
		printf("%s", filenames[i]);
		for(k = 0; k < linemax + 2 - strlen(filenames[i]); k++)
		      printf(" ");
		if((++j) % nlinemax == 0)
		      printf("\n");
	}
	printf("\n\n");
}

