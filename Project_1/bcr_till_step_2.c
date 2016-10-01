#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "gmp.h"

int alpha[26];
int digits[10];
char* decrypt_book;

char str_to_digit[10][6] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };

char* read_book_key(char* book_name)
{
	FILE* fp = fopen(book_name, "r");
	char* buffer;
	long lSize;
	if (NULL == fp)
	{
		return "-1";
	}
	fseek(fp, 0, SEEK_END);
  	lSize = ftello(fp);
	rewind(fp);
	buffer = (char*) malloc (sizeof(char)*lSize);
	fread(buffer, 1, lSize, fp);
	return buffer;	
}

void generate_book_code(char* word, int n)
{
	int i;
	for (i=0;i<10;i++)
	{
		if (strcmp(word, str_to_digit[i])==0)
		{
			digits[i] = n;
			return;
		}
	}
	alpha[(char)word[0]-'A'] = n;
}

void apply_book_cipher(char c)
{
	int len = strlen(decrypt_book);
	decrypt_book[len]=c;
	decrypt_book[len+1]='\0';
}

void split_string_in_twos(char* str)
{
	int i,j;
	char* tmp = malloc(sizeof(2));
	for(i=0;i<strlen(str)-1;i=i+2)
	{
		tmp[0]=str[i];
		tmp[1]=str[i+1];

		int t = atoi(tmp);
		for (j=0;j<10;j++)
		{
			if (digits[j]==t)
			{
				apply_book_cipher((char)j+'0');
				continue;				
			}
		}
		for (j=0;j<26;j++)
		{
			if (alpha[j]==t)
			{
				apply_book_cipher((char)('A'+(char)j));
				continue;
			}
		}
	}
}

void caesar()
{
	for(int i=0;i<strlen(decrypt_book);i++)
	{
		if (isdigit(decrypt_book[i]))
		{
			decrypt_book[i] = (char)((int)(3+(decrypt_book[i]-'0'))%10)+'0';
		}
		else if (isalpha(decrypt_book[i]))
		{
			decrypt_book[i] = (char)((int)(11+(decrypt_book[i]-'A'))%26)+'A';
		}
	}
}

void convert_hex_to_dec()
{
	mpz_t integ;
	mpz_init(integ);
	mpz_set_str (integ, decrypt_book, 16);
	//gmp_printf("%Zd\n", integ);
	mpz_get_str (decrypt_book, 10, integ);
}

int main()
{
	char* final = "02192404240811270719140602241104010404040830181419040718071412122419071917091206240627010706270414300408091817060212081914040614080908171814042718180830060408090714170617271709243024090912081917091427012404113007060419062409011109180619111802270619"; 
	char* book = read_book_key("book_cipher1.txt");
	char* token;
	int word_count =0;
	token = strtok(book, "-");
	while(token != NULL)
	{
		generate_book_code(token, ++word_count);
		token = strtok (NULL, "-");
	}
	decrypt_book = malloc(sizeof(char)*strlen(final));
	split_string_in_twos(final);
	printf("%s\t%ld\n",decrypt_book,strlen(decrypt_book));
	caesar();
	printf("%s\t%ld\n",decrypt_book,strlen(decrypt_book));
	convert_hex_to_dec();
	printf("%s\t%ld\n",decrypt_book,strlen(decrypt_book));

	int j=(strlen(decrypt_book)-1)/2;int k=0;
	mpz_t integ;
        mpz_init(integ);
 
	for (int i=0;i<strlen(decrypt_book);i++)
	{
		for (k=0;k<j;k++)
			printf("%c",decrypt_book[k]);
		printf("  ");
		for (k=j;k<(strlen(decrypt_book)-j);k++) {
			printf("%c",decrypt_book[k]);
		}
		printf("  ");
		for (k=strlen(decrypt_book)-j;k<strlen(decrypt_book);k++)
			printf("%c",decrypt_book[k]);
		printf("\n");
		j=j-1;
			
	}
	return 0;
}
