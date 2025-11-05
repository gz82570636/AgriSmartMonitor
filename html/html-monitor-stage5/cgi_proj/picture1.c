#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <dirent.h>
#include <iconv.h>
#include <sys/types.h>
#include <sys/wait.h>


int cgiMain()
{
	DIR *dir;
	struct dirent *dirp;

	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<HTML>\n");
	fprintf(cgiOut, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
	fprintf(cgiOut, " <html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
	fprintf(cgiOut, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />");
	fprintf(cgiOut, "<link rel=\"stylesheet\" href=\"../images/style.css\" type=\"text/css\" />	<title>history Picture</title></head>");
	fprintf(cgiOut, "<body>");
	fprintf(cgiOut, "<H1 align=\"left\">history Picture:<H1>");
	fprintf(cgiOut, "<a href=\"/main1.html\">回主页</a>");

	if((dir = opendir("/www/pice/")) == NULL)
	{
		perror("fail to opendir");
		return -1;
	}

	if(dir != NULL)
	{
		while((dirp = readdir(dir)) != NULL)
		{
			if(dirp->d_name[0] == '.') continue;
			fprintf(cgiOut, "<div align=\"center\">");
			fprintf(cgiOut,"%s",dirp->d_name);
			fprintf(cgiOut, "</div>");
			fprintf(cgiOut, "<div align=\"center\"><img src=\"/pice/%s\" width=\"320\" height=\"240\" />",dirp->d_name);
			fprintf(cgiOut, "</div>");
		}
	}

	fprintf(cgiOut, "</BODY></HTML>");
	

	return 0;
}
