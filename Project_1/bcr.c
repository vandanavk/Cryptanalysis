#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "gmp.h" //used for manipulating very large numbers

#define FACTOR_EXE       "./factor"

/* store the book cipher code */
int alpha[26];
int digits[10];

/* numbers in words required for book cipher code generation */ 
char str_to_digit[10][6] = { "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE" };

/* store each stage of decryption */
char* decrypt_book;

/* store the parameters for RSA */
mpz_t N, e, C, d, M, P, Q;


/*
 *
 * DECRYPTION
 *
 * Step 1:- Book cipher
 */

/*
 * Step 1.1:- Parse the book cipher and
 * generate the code
 */

/*
 * read the contents of a file.
 * this can be used for reading the encrypted string/
 * book cipher/ caesar key
 */
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
	fclose(fp);
	return buffer;	
}

/*
 * generate the code from the book cipher
 * for digits represented in words, store the code against
 * digits 0-9.
 * For words in the NATO phonetic alphabet, store
 * the code against the starting letter of the word
 */
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
 * Step 1.2:- Apply the book cipher code
 */


/*
 * apply book cipher code on the encrypted string.
 * This is the string that Alice and Bob see on the treasure map.
 */
void apply_book_cipher(char c)
{
	int len = strlen(decrypt_book);
	decrypt_book[len]=c;
	decrypt_book[len+1]='\0';
}

/*
 * To apply the book code, the encrypted string needs
 * to be split into multiple strings of length 2 each
 */
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
 * Step 2:- Caesar cipher
 */

/*
 * apply caesar cipher.
 * for a Caesar key XXYY where XX is the alphabet shift and
 * YY is the digit shift,
 * shift alphabets by XX to the right and digit by YY to
 * the left.
 */
void caesar(int alpha_shift, int digit_shift)
{
	for(int i=0;i<strlen(decrypt_book);i++)
	{
		if (isdigit(decrypt_book[i]))
		{
			decrypt_book[i] = (char)((int)(digit_shift+(decrypt_book[i]-'0'))%10)+'0';
		}
		else if (isalpha(decrypt_book[i]))
		{
			decrypt_book[i] = (char)((int)(alpha_shift+(decrypt_book[i]-'A'))%26)+'A';
		}
	}
}


/*
 * Step 3:- RSA
 */


/*
 * Step 3.1: Convert the hexadecimal result of Caesar to a
 * decimal number
 */

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
 * Step 3.2: Find N,e,C from the resultant decimal number
 */

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
	int length = strlen(decrypt_book);
	int j=length/2;
	int k=0,p=0,c=0;
	char* e_str=malloc(sizeof(char)*j);
	char* N_str=malloc(sizeof(char)*j);
	char* C_str=malloc(sizeof(char)*j);
 
	mpz_init(N);
	mpz_init(e);
	mpz_init(C);
	for (int i=0;i<length;i++)
	{
		if (j==0)
			break;
		p=0;c=0;
		for (k=0;k<j-((length%2)+1);k++) {
			N_str[k]=decrypt_book[k];
		}
		for (k=j;k<(length-j);k++) {
			e_str[p]=decrypt_book[k];
			p++;
		}
		mpz_set_str (e, e_str, 10);
		if (mpz_cmp_ui(e,9)>0 && mpz_probab_prime_p (e, 50)==2) {
			for (k=length-j;k<length;k++)
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
	printf("\nFrom the %d digit decimal number, split in such a way that we get N,"
		"e, C where e is prime and the length of N and C are equal\n",
		length);

	printf("we get N = %s,\n e = %s,\n C = %s\n", N_str, e_str, C_str);
}


/*
 * Step 3.3: Find P,Q from N
 */

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
	/*
	 * Execute the binary factor which uses GMP-ECM to find
	 * out the prime factors of a large number
	 */
	sprintf(command, "%s %s > %s", FACTOR_EXE, temp, "factor.txt");
	system(command);

	fseek(fp, 0, SEEK_END);
  	lSize = ftello(fp);
	rewind(fp);
	buffer = (char*) malloc (sizeof(char)*lSize);
	fread(buffer, 1, lSize, fp);

	/*
	 * Parse the output of "factor" to extract the prime
	 * factors.
	 */
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

	printf("Performing prime factorization on N to get P & Q\n");
	printf("P=%s,\n Q=%s\n", result[0], result[1]);

	free(temp);
	free(result[1]);
	free(result[0]);
	free(buffer);
	fclose(fp);
}

/*
 * Step 3.4: Find private key 'd' and M
 */


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
	char* str, *final_message, *tmp;
	int i,j, k=0;

	mpz_init(totient);
	mpz_sub_ui (P, P, 1);
	mpz_sub_ui (Q, Q, 1);
	mpz_mul (totient, P, Q);
	mpz_invert (d, e, totient);
	printf("\nFind d from e and P,Q, we get");
	gmp_printf ("\nd =  %Zd\n", d);

	/*
	 * Find M using C^d mod N
	 */
	mpz_powm_sec (M, C, d, N);
	printf ("\nFrom C, d and N, we get M as\n");
	gmp_printf ("\nM =  %Zd\n", M);

	str = malloc(sizeof(char) * mpz_sizeinbase(M, 10));
	mpz_get_str(str, 10, M);

	/*
	 * convert the large decimal number to ASCII
	 * by splitting into small strings of length 3 each
	 */
	memmove(str + 1, str, strlen(str) + 1);
	str[0]='0';
	printf("%s\n",str);

	tmp = malloc(sizeof(3));
	final_message = malloc(sizeof(char)*(strlen(str)/3));
	for(i=0;i<strlen(str)-2;i=i+3)
	{
		tmp[0]=str[i];
		tmp[1]=str[i+1];
		tmp[2]=str[i+2];

		final_message[k]=(char)atoi(tmp);
		k++;
	}
	printf("%s\n", final_message);

	free(final_message);
	free(tmp);
	free(str);
}

int main()
{
	char* final = read_book_key("final.txt");
	char* book = read_book_key("book_cipher1.txt");
	char* caesar_key = read_book_key("caesar.txt");
	char* token;
	int word_count =0, i=0, keys[2];

	/*
	 * Step 1: Book cipher
	 */
	printf("Alice and Bob see:\n%s\n", final);
	printf("Applying book cipher : %s\n", book);

	token = strtok(book, "-\n");
	while(token != NULL)
	{
		generate_book_code(token, ++word_count);
		token = strtok (NULL, "-");
	}

	decrypt_book = malloc(sizeof(char)*strlen(final));
	split_string_in_twos(final);
	printf("we get \n%s\n", decrypt_book);

	/*
	 * Step 2: Caesar cipher
	 */
	token = strtok(caesar_key, "-\n");
	while(token != NULL)
	{
		if (atoi(token)!=0) {
			keys[i]=atoi(token);
			i=i+1;
		}
		token = strtok(NULL,"-");
	}
	caesar(keys[0],10-keys[1]);

	printf("Applying caesar cipher: shift alphabets %d to the right and digits %d to the left\n",keys[0],keys[1]);
	printf("we get: \n%s\n", decrypt_book); 

	/*
	 * Step 3: RSA
	 */
	convert_hex_to_dec();
	printf("convert the result to decimal\n");
	printf("we get \n%s\n", decrypt_book);

	find_N_e_C();
	find_factors();
	find_M();

	free(decrypt_book);
	return 0;
}
