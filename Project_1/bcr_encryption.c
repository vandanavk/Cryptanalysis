#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gmp.h>

mpz_t M,N,e,d,C;
char *M_str = "FROMDGONORTH37WEST23DIG5";
char* caesar;

int alpha[26];
int digits[10];

char str_to_digit[10][6] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };


// read the book cipher given in the challenge
char* read_book_key()
{
        FILE* fp = fopen("book_cipher1.txt", "r");
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



char* ascii_to_numbers()
{
   char *output = malloc(sizeof(char) * (strlen(M_str) * 4 + 1));
   char *output_end = output;

   *output_end = '\0';
   for (; *M_str; ++M_str) {
      output_end += sprintf(output_end, "0%u", *M_str);
   }

  printf("%s\n",output);
return output;
}

void apply_caesar(char* str)
{
	for(int i=0;i<strlen(str);i++)
        {
                if (isdigit(str[i]))
                {
                        caesar[i] = (char)((int)(7+(str[i]-'0'))%10)+'0';
                }
                else if (isalpha(str[i]))
                {
                        caesar[i] = (char)((int)(9+(str[i]-'A'))%26)+'A';
                }
        }
}

void apply_book()
{
	char* final = malloc(10);
	char* temp = malloc(200);
	//sprintf(final,"0%u\n",digits[(char)caesar[0]-'0']);
	for (int i=0;i<strlen(caesar);i++)
	{
		if (isdigit(caesar[i]))
		{
			if (digits[(char)caesar[i]-'0']<10)
				sprintf(final,"0%u",digits[(char)caesar[i]-'0']);
			else
				sprintf(final,"%u",digits[(char)caesar[i]-'0']);

		strcat(temp,final);
		}
		else if (isalpha(caesar[i]))
		{
			if (alpha[(char)caesar[i]-'A']<10)
				sprintf(final,"0%u",alpha[(char)caesar[i]-'A']);
			else
				sprintf(final,"%u",alpha[(char)caesar[i]-'A']);

		strcat(temp,final);
		}
	}
	printf("%s\n",temp);
}
int main()
{
	char* book = read_book_key();
	char* token;
	int word_count =0;
        token = strtok(book, "-");
        while(token != NULL)
        {
                generate_book_code(token, ++word_count);
                token = strtok (NULL, "-");
        }
	char* N_str = "501281908486219621910086584233925309600136539640088201223414043112175611";
	char* e_str = "65537";
	char* C_str = malloc(80);
	char* RSA_key = malloc(150);
	caesar = malloc(120);
	char* temp = ascii_to_numbers();
	//find C=M^e mod N
	mpz_init(C);
	mpz_init(M); mpz_set_str(M,temp,10);
	mpz_init(e); mpz_set_str(e, e_str,10);
	mpz_init(N); mpz_set_str(N,N_str,10);
	mpz_set_str(M,M_str,10);
	mpz_powm_sec (C, M, e, N);
	gmp_printf ("%Zd\n", C);
	mpz_get_str(C_str,10,C);
	strcat(RSA_key,N_str);
	strcat(RSA_key,e_str);
	strcat(RSA_key,C_str);
	printf("%s\n", RSA_key);

	char* dec;
        mpz_t integ;
        mpz_init(integ);
        mpz_set_str (integ, RSA_key, 10);
        mpz_get_str (dec, 16, integ);
	printf("%s\n", dec);
	apply_caesar(dec);
	printf("%s\n",caesar);
	apply_book();
	return 0;
}
