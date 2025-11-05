#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   

char name[64];   
char pass[64];  
  
char* getcgidata(FILE* fp, char* requestmethod)   
{   
	char* input;   
	int len;   
	int size = 1024;   
	int i = 0;   
	  
	if (!strcmp(requestmethod, "GET"))   {   
		 input = getenv("QUERY_STRING");   
		 return input;   
	}   
	else if (!strcmp(requestmethod, "POST"))   {   
		 len = atoi(getenv("CONTENT_LENGTH"));   
		 input = (char*)malloc(sizeof(char)*(size + 1));   
		   
		 if (len == 0)   {   
			input[0] = '\0';   
			return input;   
		 }   
		   
		 while(1)   
		 {   
			input[i] = (char)fgetc(fp);   
			if (i == size)   {   
				 input[i+1] = '\0';   
				 return input;   
			}   										  
			--len;   
			if (feof(fp) || (!(len)))   {   
				 i++;   
				 input[i] = '\0';   
				 return input;   
			}   
			i++;   
						  
		 }   
	}   
	return NULL;  
}	

void unencode_for_name_pass(char *input)
{
	int i = 0;   
	int j = 0;   
	
		// ���ǻ�ȡ��input�ַ������������µ���ʽ   
		// Username="admin"&Password="aaaaa"   
		// ����"Username="��"&Password="���ǹ̶���   
		// ��"admin"��"aaaaa"���Ǳ仯�ģ�Ҳ������Ҫ��ȡ��   
		  
		// ǰ��9���ַ���UserName=   
		// ��"UserName="��"&"֮���������Ҫȡ�������û���   	
	for ( i = 9; i < (int)strlen(input); i++ )  {   
		 if (input[i] == '&')   {   
			name[j] = '\0';   
			break;   
		 }                                       
		 name[j++] = input[i];   
	}   
   
	// ǰ��9���ַ� + "&Password="10���ַ� + Username���ַ���   
	// �����ǲ�Ҫ�ģ���ʡ�Ե���������   
	for ( i = 19 + strlen(name), j = 0; i < (int)strlen(input); i++ ){   
		 pass[j++] = input[i];   
	}   
	pass[j] = '\0';   	
	
	//printf("Your Username is %s<br> Your Password is %s<br> \n", name, pass);   
	
	printf("Content-type: text/html\n\n");   //���߱���������html�﷨������
	if((strcmp(name,"chen") == 0)&&(strcmp(pass,"123") == 0))
	{
		printf("<script language='javascript'>document.location = 'http://192.168.1.200/choose.html'</script>"); //�Զ���ת�����ҳ��	
	}
	else{
		printf("�û������������<br><br>");  
		//exit(-1);
	}
}

int main()   
{   
	char *input;   
	char *req_method;   
 
	printf("Content-type: text/html\n\n");   //���߱���������html�﷨������
	printf("The following is query reuslt:<br><br>");   

	req_method = getenv("REQUEST_METHOD");   
	input = getcgidata(stdin, req_method);   //��ȡURL ���������
	
	unencode_for_name_pass(input);   //���룬���ж��û���������,�����ȷ����ת��ѡ��ֿ���棬������ʾ����	
	return 0;   
}   
   
			
