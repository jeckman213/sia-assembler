#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *ltrim(char *s)
{
	while (*s == ' ' || *s == '\t') s++;
	return s;
}

char getRegister(char *text)
{
	if (*text == 'r' || *text=='R') text++;
	return atoi(text);
}

// Helper function for 3R format operations
int ThreeRegisterHelper(unsigned char* bytes)
{
	bytes[0] |= getRegister(strtok(NULL, " "));
	bytes[1] = getRegister(strtok(NULL, " ")) << 4 | getRegister(strtok(NULL, " "));
	return 2;
}

// Helper function for branch commands
int branchHelper(unsigned char* bytes)
{
	// Grabs offset from line
	int offset = atoi(strtok(NULL, " "));

	// Shifts Right 16 bits to just get top 4 bits of offset
	bytes[1] |= offset >> 16;

	// Left shift 4 to get rid of top 4 bits and then right shift 12 to get rid of bottom 12 bits
	bytes[2] = offset << 4 >> 12;

	// Left shift 8 to get rid of top 8 bits and then right shift 12
	bytes[3] = offset << 12 >> 12;

	// 0000 0000 0000 0000 1010 >> 16 -> 0000 0000 0000 0000 0000
	// 0000 0000 0000 0000 1010 << 4 -> 0000 0000 0000 1010 0000 >> 12 -> 0000 0000 0000 0000 0000
	// 0000 0000 0000 0000 1010 << 12 -> 0000 1010 0000 0000 0000 >> 12 -> 0000 0000 0000 0000 1010

	return 4;
}

int jumpHelper(unsigned char* bytes)
{
	// Gets jump address from line
	int jumpAddress = atoi(strtok(NULL, " "));

	bytes[0] |= jumpAddress >> 24;

	bytes[1] = jumpAddress << 4 >> 20;

	bytes[2] = jumpAddress << 12 >> 20;

	bytes[3] = jumpAddress << 20 >> 20;

	return 4;
}

int assembleLine(char *text, unsigned char* bytes)
{
	text = ltrim(text);
	char *keyWord = strtok(text," ");
	if (strcmp("add",keyWord) == 0)
	{
		bytes[0] = 0x10;
		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("subtract", keyWord) == 0)
	{
		bytes[0] = 0x50;
		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("and", keyWord) == 0)
	{
		bytes[0] = 0x20;
		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("or", keyWord) == 0)
	{
		bytes[0] = 0x60;
		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("multiply", keyWord) == 0)
	{
		bytes[0] = 0x40;
		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("divide", keyWord) == 0)
	{
		bytes[0] = 0x30;
		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("halt", keyWord) == 0)
	{
		bytes[0] = 0x00;
		bytes[1] = 0x00;
		return 2;
	}
	else if (strcmp("addimmediate", keyWord) == 0)
	{
		bytes[0] = 0x90;
		bytes[0] |= getRegister(strtok(NULL, " "));
		bytes[1] = getRegister(strtok(NULL, " "));
		return 2;
	}
	else if (strcmp("branchifequal", keyWord) == 0)
	{
		bytes[0] = 0xA0;
		bytes[0] |= getRegister(strtok(NULL, " "));
		bytes[1] = getRegister(strtok(NULL, " ")) << 4;

		return branchHelper(bytes);
	}
	else if (strcmp("branchifless", keyWord) == 0) 
	{
		bytes[0] = 0xB0;
		bytes[0] |= getRegister(strtok(NULL, " "));
		bytes[1] = getRegister(strtok(NULL, " ")) << 4;

		return branchHelper(bytes);
	}
	else if (strcmp("interrupt", keyWord) == 0)
	{
		bytes[0] = 0x80;
		
		// Gets interrupt number after keyWord
		int interrupt = atoi(strtok(NULL, " "));

		// 1010 0101 0011 >> 8 -> 0000 0000 1010
		bytes[0] |= interrupt >> 8;

		// 1010 0101 0011 << 4 -> 0101 0011 0000 >> 4 -> 0000 0101 0011
		bytes[1] = interrupt << 4 >> 4;

		return 2;
	}
	else if (strcmp("call", keyWord) == 0)
	{
		bytes[0] = 0xD0;

		return jumpHelper(bytes);
	}
	else if (strcmp("jump", keyWord) == 0)
	{
		bytes[0] = 0xC0;

		return jumpHelper(bytes);
	}
	else if (strcmp("store", keyWord) == 0)
	{
		bytes[0] = 0xF0;

		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("load", keyWord) == 0)
	{
		bytes[0] = 0xE0;

		return ThreeRegisterHelper(bytes);
	}
	else if (strcmp("push", keyWord) == 0)
	{
		bytes[0] = ( 0x70 | getRegister(strtok(NULL, " ")));
		bytes[1] = 0x10;

		return 2;
	}
	else if (strcmp("pop", keyWord) == 0)
	{
		bytes[0] = ( 0x70 | getRegister(strtok(NULL, " ")));
		bytes[1] = 0x20;

		return 2;
	}
	else if (strcmp("return", keyWord) == 0)
	{
		bytes[0] = 0x70;
		bytes[1] = 0x00;

		return 2;
	}
}

int main(int argc, char **argv)
{
	FILE *src = fopen(argv[1],"r");
	FILE *dst = fopen(argv[2],"wb");
	while (!feof(src)) {
		unsigned char bytes[4];
		char line[1000];
		if (NULL != fgets(line, 1000, src)) {
			printf("read: %s\n",line);
			int byteCount = assembleLine(line,bytes);

			int i = 0;
			for (i = 0; i < byteCount; i++)
			{
				printf("Byte %d: %u\n", i, bytes[i]);
			}

			fwrite(bytes,byteCount,1,dst);
		}
	}
	fclose(src);
	fclose(dst);
	return 0;
}