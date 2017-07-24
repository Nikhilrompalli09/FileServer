
// header		0-12
// bit vector   13-3500
// messages		3501-18000
// files		18001-30000
//blob			30001-100mb



#include<stdio.h>
#include<stdlib.h>
#include<string.h>
struct header
{
	int totMessgs;
	int totFiles;
};
struct message
{
	int fileId;
	int msgLen;
	int blobMsgAddr;
};
struct file
{
	int fileId;
	char fileName[30];
	char createdAt[30];
	int blobMsgAddr;
	int endAddress;
};
struct isEmpty
{
	char byt;
	int startAddr;
	int size;
};
int totMessgs = 0, totFiles = 0;
int isFilePresent(char* fileName,FILE* fp)
{
	fseek(fp, 18001, SEEK_SET);
	for (int i = 0; i < totFiles; i++)
	{
		struct file fileNode;
		fread(&fileNode, sizeof(fileNode), 1, fp);
		if (strcmp(fileNode.fileName, fileName)==0&&(fileNode.fileId!=0))
			return 1;
	}
	return 0;
}
void printComment(int cntCount,FILE* fp, int pos)
{
	fseek(fp, pos, SEEK_SET);
	char str[314];
	char* len = (char*)malloc(4 * sizeof(char));
	fread(len, sizeof(int), 1, fp);
	int l = *(int*)len;
	fread(str,l, 1, fp);
	str[l] = '\0';
	printf("\tComment %d : %s\n", cntCount,str);
}
void searchByFileId(int fileId,FILE* lob)
{
	int pos = 3501,cmtCount=0;
	for (int i = 0; i < totMessgs; i++)
	{
		fseek(lob, pos, SEEK_SET);
		struct message msgNode;
		fread(&msgNode, sizeof(msgNode), 1, lob);
		pos = ftell(lob);
		if (msgNode.fileId==fileId)
			printComment(++cmtCount,lob, msgNode.blobMsgAddr);
	}
	if (cmtCount == 0)
		printf("\tNo Comments!!!!!!!!!!!\n");
}
void displayImg(FILE* fp, int pos, int endPos)
{
	char ch;
	FILE* fp1 = fopen("new_img.jpg", "wb+");
	fseek(fp1, 0, SEEK_SET);
	fseek(fp, pos, SEEK_SET);
	while (pos <= endPos)
	{
		fread(&ch, sizeof(ch), 1, fp);
		fwrite(&ch, sizeof(ch), 1, fp1);
		pos++;
	}
	printf("File Downloaded...\n");
	fclose(fp1);

}
int fileNameToId(char* fileName, FILE* lob)
{
	int pos = 18001;
	for (int i = 0; i < totFiles; i++)
	{
		fseek(lob, pos, SEEK_SET);
		struct file fileNode;
		fread(&fileNode, sizeof(fileNode), 1, lob);
		pos = ftell(lob);
		if (strcmp(fileNode.fileName, fileName) == 0)
			return fileNode.fileId;
	}
	return 0;
}
void commentOnImage(struct message msgHead,FILE* lob)
{
	char fileName[30], msg[314];
	int fileId,pos;
	struct header head = { 0 };
	totMessgs++;
	printf("Enter file Name : ");
	scanf("%s", fileName);
	if (!isFilePresent(fileName,lob))
	{
		printf("File not Found!!!\n");
		return;
	}
	fileId = fileNameToId(fileName,lob);
	printf("Enter Message : ");
	getchar();
	gets(msg);
	msgHead.fileId = fileId;
	msgHead.msgLen = strlen(msg);
	fseek(lob, 0, SEEK_END);
	if (ftell(lob) < 30000)
		pos = 30001;
	else
		pos = ftell(lob);
	fseek(lob, pos, SEEK_SET);
	int len = strlen(msg);
	char* str = (char*)malloc((4 + len)*sizeof(char));
	memcpy(str, &len, sizeof(int));			//int will convert into 4 bytes of string
	memcpy(str + sizeof(int), msg, len);	//msg will be appened after int
	fwrite(str, sizeof(len) + len, 1, lob);
	msgHead.blobMsgAddr = pos;
	fseek(lob, 3501, SEEK_SET);
	fseek(lob, 3501 + sizeof(msgHead)*(totMessgs - 1), SEEK_SET);
	fwrite(&msgHead, sizeof(msgHead), 1, lob);
	fseek(lob, 0, SEEK_SET);
	head.totMessgs = totMessgs;
	head.totFiles = totFiles;
	fwrite(&head, sizeof(head), 1, lob);
	printf("You have Commented on Image...\n");
}
void showAllCommentsForParticularImage(FILE* lob)
{
	int pos = 18001;
	char fileName[30];
	printf("Enter File Name : ");
	scanf("%s", fileName);
	if (!isFilePresent(fileName,lob))
	{
		printf("File not Found!!!\n");
		return;
	}
	for (int i = 0; i < totFiles; i++)
	{
		fseek(lob, pos, SEEK_SET);
		struct file fileNode;
		fread(&fileNode, sizeof(fileNode), 1, lob);
		pos = ftell(lob);
		if (strcmp(fileName, fileNode.fileName) == 0)
		{
			printf("File Name : %s\n", fileNode.fileName);
			searchByFileId(fileNode.fileId, lob);
			return;
		}
	}
}
void showAllComments(FILE*lob)
{
	printf("----COMMENTS------");
	int pos = 18001;
	for (int i = 0; i < totFiles; i++)
	{
		fseek(lob, pos, SEEK_SET);
		struct file fileNode;
		fread(&fileNode, sizeof(fileNode), 1, lob);
		pos = ftell(lob);
		printf("File Name : %s\n", fileNode.fileName);
		searchByFileId(fileNode.fileId, lob);
	}
}

int isComment(char* comment,int addr,int len, FILE* fp)
{
	char messg[100];
	fseek(fp, addr+4, SEEK_SET);
	fread(messg,len, 1,fp);
	messg[len] = '\0';
	if (strcmp(comment, messg) == 0)
		return 1;
	else
		return 0;
}
void deleteComment(FILE* lob)
{
	char fileName[30], comment[100];
	int pos;
	printf("Enter file Name: ");
	getchar();
	gets(fileName);
	if (!isFilePresent(fileName,lob))
	{
		printf("File doesnt exits!!!!!\n");
		return;
	}
	else
	{
		printf("Enter the comment to delete : ");
		gets(comment);
		pos = 3501;
		for (int i = 0; i < totMessgs; i++)
		{
			fseek(lob, pos, SEEK_SET);
			struct message msgNode;
			fread(&msgNode, sizeof(msgNode), 1, lob);
			pos = ftell(lob);
			if (isComment(comment,msgNode.blobMsgAddr,msgNode.msgLen,lob))
			{
				msgNode.fileId = 0;
				fseek(lob,pos-sizeof(msgNode), SEEK_SET);
				fwrite(&msgNode, sizeof(msgNode), 1, lob);
				printf("Comment is deleted Successfully.......\n");
			}
		}
	}
}
int checkEmpty(int size, FILE* fp)
{
	int pos = 13;
	for (int i = 0; i < totFiles; i++)
	{
		fseek(fp, pos, SEEK_SET);
		struct isEmpty checkNode;
		fread(&checkNode, sizeof(checkNode), 1, fp);
		if (checkNode.byt == '0'&&checkNode.size >= size)
			return checkNode.startAddr;
	}
	return 0;
}
void uploadImage(struct file fileHead,FILE* lob)
{
	char c, fileName[30], createDate[30];
	int pos,startAddr,size;
	struct header head = { 0 };
	totFiles++;
	struct isEmpty check = { 0 };
	printf("Enter file Name: ");
	scanf("%s", &fileName);
	FILE* fp = fopen(fileName, "rb");
	if (fp == NULL)
	{
		printf("File Not Exits In Present Directory!!!!\n");
		return;
	}
	printf("Enter date(dd/mm/yy) : ");
	scanf("%s", createDate);
	fileHead.fileId = totFiles;
	strcpy(fileHead.fileName, fileName);
	strcpy(fileHead.createdAt, createDate);
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	pos = checkEmpty(size, lob);
	if (!pos)
	{
		fseek(lob, 0, SEEK_END);
		if (ftell(lob) < 30000)
			pos = 30001;
		else
			pos = ftell(lob);
	}
	fileHead.blobMsgAddr = pos;
	fseek(lob, pos, SEEK_SET);
	fseek(fp, 0, SEEK_SET);
	startAddr = ftell(lob);
	while (!feof(fp))
	{
		fread(&c, sizeof(c), 1, fp);
		fwrite(&c, sizeof(c), 1, lob);
	}
	fileHead.endAddress = ftell(lob);
	fclose(fp);
	check.byt = '1';
	check.startAddr = startAddr;
	check.size = size;
	fseek(lob, 13 + sizeof(check)*(totFiles - 1), SEEK_SET);
	fwrite(&check, sizeof(check), 1, lob);
	fseek(lob, 18001 + sizeof(fileHead)*(totFiles - 1), SEEK_SET);
	fwrite(&fileHead, sizeof(fileHead), 1, lob);
	fseek(lob, 0, SEEK_SET);
	head.totMessgs = totMessgs;
	head.totFiles = totFiles;
	fwrite(&head, sizeof(head), 1, lob);
	printf("File Successfully Uploaded.....\n");
}
void downloadImage(struct file fileHead, FILE* lob)
{
	char fileName[30];
	int pos,fileId;
	printf("Enter file Name: ");
	scanf("%s", fileName);
	if (!isFilePresent(fileName,lob))
	{
		printf("File not Found!!!\n");
		return;
	}
	fileId = fileNameToId(fileName, lob);
	pos = 18001;
	if (fileId != 0)
	{
		fseek(lob, 18001 + sizeof(fileHead)*(fileId - 1), SEEK_SET);
		struct file fileNode;
		fread(&fileNode, sizeof(fileNode), 1, lob);
		pos = ftell(lob);
		if (fileNode.fileId != 0)
			displayImg(lob, fileNode.blobMsgAddr, fileNode.endAddress);
	}
}
void deleteImage(struct file fileHead, FILE* lob)
{
	char fileName[30];
	int fileId,pos;
	struct isEmpty check;
	printf("Enter File name : ");
	scanf("%s", &fileName);
	if (!isFilePresent(fileName,lob))
	{
		printf("File not Found!!!\n");
		return;
	}
	fileId = fileNameToId(fileName, lob);
	fseek(lob, 13 + sizeof(check)*(fileId - 1), SEEK_SET);
	fread(&check, sizeof(check), 1, lob);
	check.byt = '0';
	fseek(lob, -(signed)sizeof(check), SEEK_CUR);
	fwrite(&check, sizeof(check), 1, lob);
	pos = 18001	;
	fseek(lob, pos, SEEK_SET);
	for (int i = 0; i < totFiles; i++)
	{
			struct file fileNode;
			fread(&fileNode, sizeof(fileNode), 1, lob);
			if (fileNode.fileId == fileId)
			{
				fileNode.fileId = 0;
				fseek(lob, -(signed)sizeof(fileNode), SEEK_CUR);
				fwrite(&fileNode, sizeof(fileNode), 1, lob);
				printf("Image Deleted Successfully......\n");
				return;
			}
			
	}
	
}
void showAllImages(FILE* lob)
{
	int pos = 18001,count=0;
	fseek(lob, pos, SEEK_SET);
	printf("------FILES PRESENT IN FILE SYSTEM------\n");
	for (int i = 0; i < totFiles; i++)
	{
		struct file fileNode;
		fread(&fileNode, sizeof(fileNode), 1, lob);
		if (fileNode.fileId != 0)
			printf("\t%d . %s\n", ++count,fileNode.fileName);
	}
}
int main()
{
	FILE* lob;
	int ch;
	char input[10];
	struct message msgHead = { 0 };
	struct file fileHead = { 0 };
	lob = fopen("fileServer.disk", "rb+");
	if (lob == NULL)
	{
		printf("Error in File Open\n");
		return 0;
	}
	struct header head1 = { 0 };
	struct header head = { 0 };
	struct isEmpty check = { 0 };

	/*fseek(lob, 0, SEEK_SET);
	head.totUsers = totUsers;
	head.totMessgs = totMessgs;
	head.totFiles = totFiles;
	fwrite(&head, sizeof(head), 1, lob);
	puts("header written");*/

	fseek(lob, 0, SEEK_SET);
	fread(&head1, sizeof(head1), 1, lob);
	totMessgs = head1.totMessgs;
	totFiles = head1.totFiles;
	do
	{
		puts("\n---------------------------------------FILE SYSTEM------------------------------------------\n");
		printf("1. UPLOAD FILE\n2. DOWNLOAD FILE\n3. DELETE FILE\n4. SHOW ALL IMAGES\n5. ADD COMMENTS ON IMAGE\n6. READ ALL COMMENTS\n7. SHOW COMMENTS ON PARTICULAR FILE\n8. DELETE COMMENT\n\n");
		printf("Enter your choice : ");
		scanf("%d", &ch);
		switch (ch)
		{
		case 1:
		{
			uploadImage(fileHead, lob);
			break;
		}
		case 2:
		{
			downloadImage(fileHead, lob);
			break;

		}
		case 3:
		{
			deleteImage(fileHead, lob);
			break;

		}
		case 4:
		{
			showAllImages(lob);
			break;
		}
			case 5:
			{
				commentOnImage(msgHead, lob);
				break;
			}
			case 6:
			{
				showAllComments(lob);
				break;
			}
			case 7:
			{
				showAllCommentsForParticularImage(lob);
				break;
			}
			case 8:
				deleteComment(lob);

				break;
	}
		puts("\nDo you want to continue(yes/no)?");
		scanf("%s", input);
	} while (!strcmp(input, "yes"));
	fclose(lob);
	system("pause");
}
