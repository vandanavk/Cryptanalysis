#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "gmp.h" //used for manipulating very large numbers
#include "ecm.h"

#ifdef _MSC_VER
#define FACTOR_EXE       "factor"
#else
#define FACTOR_EXE       "./factor"
#endif

int alpha[26];
int digits[10];
char* decrypt_book;
mpz_t N, e, C, d, M, P, Q;

char str_to_digit[10][6] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };


// read the book cipher given in the challenge
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

// generate the code from the book cipher
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

/*
 * apply book cipher code on the string which was generated after
 * 3 steps of encryption
 */
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

/*
 * apply caesar cipher.
 * move 11 letters to the right and 3 digits to the right (7 digits to the left)
 */
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

/*
 * the result of caesar cipher is a large hex number which needs to be
 * converted to decimal
 */
void convert_hex_to_dec()
{
	mpz_t integ;
	mpz_init(integ);
	mpz_set_str (integ, decrypt_book, 16);
	mpz_get_str (decrypt_book, 10, integ);
}

/*
 * from the large decimal number, find out N, e, C
 * since the RSA key is not given.
 * the length or number of bits of N would be equal to
 * that of C.
 * Therefore, split the string till you find a prime number for e.
 * The digits to the left of e would be N.
 * the digits to the right of e would be C.
 */
void find_N_e_C()
{
	int j=(strlen(decrypt_book))/2;
	int k=0,p=0,c=0;
	char* e_str=malloc(sizeof(char)*j);
	char* N_str=malloc(sizeof(char)*j);
	char* C_str=malloc(sizeof(char)*j);
 
	mpz_init(N);
	mpz_init(e);
	mpz_init(C);
	for (int i=0;i<strlen(decrypt_book);i++)
	{
		if (j==0)
			break;
		p=0;c=0;
		for (k=0;k<j-2;k++) {
			N_str[k]=decrypt_book[k];
		}
		for (k=j;k<(strlen(decrypt_book)-j);k++) {
			e_str[p]=decrypt_book[k];
			p++;
		}
		mpz_set_str (e, e_str, 10);
		if (mpz_cmp_ui(e,9)>0 && mpz_probab_prime_p (e, 50)==2) {
			for (k=strlen(decrypt_book)-j;k<strlen(decrypt_book);k++)
			{
				C_str[c]=decrypt_book[k];
				c++;
			}
			break;
		}
		j=j-1;
	}
	mpz_set_str (N, N_str, 10);
	mpz_set_str (C, C_str, 10);
}

/*
 * To find d such that ed = 1 mod (P-1)(Q-1)
 * we need to find P and Q which are prime factors of N.
 * For this, use a binary called "factor"
 * which in turn calls the GMP ECM library to compute
 * the factors
 */
void find_factors()
{
	FILE* fp = fopen("factor.txt","r");
	char* buffer, *find_pq, *tok, *result[2];
	long lSize;
	int m=0;
	char* temp=malloc(sizeof(char)*(strlen(decrypt_book)/2));
	char command[100];

	mpz_get_str (temp, 10, N);
	sprintf(command, "%s %s > %s", FACTOR_EXE, temp, "factor.txt");
	system(command);

	fseek(fp, 0, SEEK_END);
  	lSize = ftello(fp);
	rewind(fp);
	buffer = (char*) malloc (sizeof(char)*lSize);
	fread(buffer, 1, lSize, fp);
	find_pq = strstr(buffer, "is:");
	find_pq=find_pq+3;
	tok = strtok(find_pq," ");
	while( tok != NULL ) 
   	{
		if (atoi(tok)!=0) {
			result[m] = malloc(sizeof(char)*(strlen(decrypt_book)/2));
			strcpy(result[m], tok);
			m=m+1;
		}
      		tok = strtok(NULL, " ");
   	}
	mpz_init(P);
	mpz_init(Q);
	mpz_set_str(P,result[0],10);
	mpz_set_str(Q,result[1],10);
}

/*
 * Find d by e^-1 mod totient(N)
 * Once we have N, d, e, C, find M
 * by M=C^d mod N.
 * According to the challenge split M into
 * 3 digit parts and convert each part to ASCII to
 * find the message
 */
void find_M()
{
	mpz_t totient;
	char* str = malloc(100);
	char* final_message = malloc(35);

	mpz_init(totient);
	mpz_sub_ui (P, P, 1);
	mpz_sub_ui (Q, Q, 1);
	mpz_mul (totient, P, Q);
	mpz_invert (d, e, totient);

	mpz_powm_sec (M, C, d, N);
	
	mpz_get_str(str, 10, M);
	memmove(str + 1, str, strlen(str) + 1);
	str[0]='0';
	printf("%s\n",str);

	int i,j, k=0;
	char* tmp = malloc(sizeof(3));
	for(i=0;i<strlen(str)-2;i=i+3)
	{
		tmp[0]=str[i];
		tmp[1]=str[i+1];
		tmp[2]=str[i+2];

		final_message[k]=(char)atoi(tmp);
		k++;
	}
	printf("%s\n", final_message);
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
	caesar();
	convert_hex_to_dec();
	find_N_e_C();
	find_factors();
	find_M();

	return 0;
}
