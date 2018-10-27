/*************************************************************************
	> File Name: test.c
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2018年10月25日 星期四 20时10分14秒
 ************************************************************************/

#include <string.h>
#include <stdio.h>
int main(void)
{
	char tt[100] = "GET / HTTP/1.1";
	char *szTemp = tt;

	char *szURL = strpbrk(szTemp, " \t");
	if (!szURL)
	{
		return -100;
	}
	*szURL++ = '\0';

	printf("szURL ==%s \n", szURL);

	char *szMethod = szTemp;
	if (strcasecmp(szMethod, "GET") == 0)
	{
		printf("The request method is GET\n");
	}
	else
	{
		return -100;
	}

	szURL += strspn(szURL, " \t");
	char *szVersion = strpbrk(szURL, " \t");
	if (!szVersion)
	{
		return -100;
	}
	*szVersion++ = '\0';
	szVersion += strspn(szVersion, " \t");
	if (strcasecmp(szVersion, "HTTP/1.1") != 0)
	{
		return -100;
	}

	if (strncasecmp(szURL, "http://", 7) == 0)
	{
		szURL += 7;
		szURL = strchr(szURL, '/');
	}

	if (!szURL || szURL[0] != '/')
	{
		return -100;
	}

	//URLDecode( szURL );
	printf("The request URL is: %s\n", szURL);
}
