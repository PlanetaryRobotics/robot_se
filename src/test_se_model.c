// TODO Change hardcoded N, M values. KR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "se_model.h"

// Initialize se
static void test_init(ekf_t* p_ekf)
{
    se_init(p_ekf);

    int i, j;
    for (i=0; i<N; ++i)
        for (j=0; i<N; ++i)
            p_ekf->F[i][j] = 0;

    for (i=0; i<N; ++i)
        p_ekf->A[i][i] = 1;    
}

static void print_state(float idx, const ekf_t* p_ekf, const char* prefix, int x_or_fx, int p_or_pp, double delta, FILE* p_fout)
{
    int i, j;
      
    fprintf(p_fout, "s_%s_%g,", prefix, idx); 
    for (i=0 ; i<15 ; i++)
      fprintf(p_fout, "%g,", x_or_fx ? p_ekf->x[i] : p_ekf->fx[i]);
    fprintf(p_fout, "\n");

    fprintf(p_fout, "p_%s_%g,", prefix, idx); 
    for (i=0 ; i<15 ; i++)
        for (j=0 ; j<15 ; j++)
	    fprintf(p_fout, "%g,", p_or_pp ? p_ekf->P[i][j]: p_ekf->Pp[i][j]);
    fprintf(p_fout, "\n");

    fprintf(p_fout, "q_%s_%g,", prefix, idx); 
    for (i=0 ; i<15 ; i++)
        for (j=0 ; j<15 ; j++)
	    fprintf(p_fout, "%g,", p_ekf->Q[i][j]);
    fprintf(p_fout, "\n");       

    fprintf(p_fout, "a_%s_%g,", prefix, idx); 
    for (i=0 ; i<15 ; i++)
        for (j=0 ; j<15 ; j++)
	    fprintf(p_fout,"%g,", p_ekf->A[i][j]);
    fprintf(p_fout, "\n");       

    fprintf(p_fout, "f_%s_%g,", prefix, idx); 
    for (i=0 ; i<15 ; i++)
        for (j=0 ; j<15 ; j++)
	    fprintf(p_fout, "%g,", p_ekf->F[i][j]);
    fprintf(p_fout, "\n");       

    fprintf(p_fout, "d_%s_%g,%g\n", prefix, idx, delta); 
}

static void print_msmt(float idx, const ekf_t* p_ekf, const char* prefix, double* msmt, FILE* p_fout)
{
    fprintf(p_fout, "m_%s_%g,", prefix, idx); 
    int i;
    for (i=0 ; i<15 ; i++)
        fprintf(p_fout, "%g,", msmt[i]);
    fprintf(p_fout, "\n");

    fprintf(p_fout, "ms_%s_%g,", prefix, idx); 
    for (i=0 ; i<15 ; i++)
        if (p_ekf->H[i][i] == 1)
	    fprintf(p_fout, "%d,", i);
    fprintf(p_fout, "\n");

    fprintf(p_fout, "r_%s_%g,", prefix, idx); 
    int j;
    for (i=0 ; i<15 ; i++)
        for (j=0 ; j<15 ; j++)
	  fprintf(p_fout, "%g,", p_ekf->R[i][j]);
    fprintf(p_fout, "\n");
}

static int read_tokens(char* line, double* p_data, float p_tok_info[2])
{
    int num = 0;
    const char delim[2] = ",";            
    p_tok_info[0] = atof(strtok(line, delim)); // type
    p_tok_info[1] = atof(strtok(NULL, delim)); // index   
    int valid_tok = 1;
    while (valid_tok) 
    {
        char* token = strtok(NULL, delim); 
        if (token == NULL)  
	    valid_tok = 0;
        else
  	    p_data[num++] = atof(token);
    }
    return num;
}

static void test_predict(ekf_t* p_ekf, const char* inp, const char* out)
{
    test_init(p_ekf);

    FILE* p_finp = fopen(inp, "r");        
    if (p_finp == NULL)
    {
        printf ("InputFile .. [BAD][file=%s]\n", inp);
        return;
    }
         
    FILE* p_fout = fopen(out, "w");        
    if (p_fout == NULL)
    {
        printf ("OutputFile .. [BAD][file=%s]\n", out);
        return;
    }

    char * line = NULL;
    size_t size = 0;
    int ret = 0;
    while ((ret = getline(&line, &size, p_finp)) != -1)
    {
        double s[230], p[230], q[230], d[10];
        float p_tok_info[2];
        
  	// Read s
        int num_s = read_tokens(line, s, p_tok_info);
        assert(num_s == 15);    
    
	// Populate x
        int i, j;
        int num = 0;
        for (i=0; i<15; ++i)
            p_ekf->x[i] = s[num++];
            
        // Read p
        getline(&line, &size, p_finp);
        int num_p = read_tokens(line, p, p_tok_info);
        assert(num_p == 225);    

        // Populate p
        num = 0;
        for (i=0; i<15; ++i)
            for (j=0; j<15; ++j)
                p_ekf->P[i][j] = p[num++];

        // Read q
        getline(&line, &size, p_finp);
        int num_q = read_tokens(line, q, p_tok_info);
        assert(num_q == 225);    

	// Populate q
	num = 0;
	for (i=0; i<15; ++i)
	    for (j=0; j<15; ++j)
                p_ekf->Q[i][j] = q[num++];

	// Read delta
        getline(&line, &size, p_finp);
        int num_d = read_tokens(line, d, p_tok_info);
        assert(num_d == 1);    

	// predict 
	print_state(p_tok_info[1], p_ekf, "k-1+", 1, 1, d[0], p_fout);
        se_predict(d[0], p_ekf);   
        print_state(p_tok_info[1], p_ekf, "k-", 0, 0, d[0], p_fout);
    }     
    fclose(p_finp);
    fclose(p_fout);
}

static void test_update(ekf_t* p_ekf, const char* inp, const char* out)
{
    test_init(p_ekf);

    FILE* p_finp = fopen(inp, "r");        
    if (p_finp == NULL)
    {
        printf ("InputFile .. [BAD][file=%s]\n", inp);
        return;
    }

    FILE* p_fout = fopen(out, "w");        
    if (p_fout == NULL)
    {
        printf ("OutputFile .. [BAD][file=%s]\n", out);
        return;
    }
             
    char * line = NULL;
    size_t size = 0;
    int ret = 0;
    int i, j;
    while ((ret = getline(&line, &size, p_finp)) != -1)
    {
        int num = 0;  
        double s[230], p[230], q[230], d[1], z[15], zu_temp[15], r[15];
        float p_tok_info[2];
        int zu[15];
      
	// Read s
        int num_s = read_tokens(line, s, p_tok_info);
        assert(num_s == 15);    

	// Populate fx
        num = 0;
        for (i=0; i<15; ++i)
            p_ekf->fx[i] = s[num++];
            
        // Read p
        getline(&line, &size, p_finp);
        int num_p = read_tokens(line, p, p_tok_info);
        assert(num_p == 225);    

        // Populate p
        num = 0;
        for (i=0; i<15; ++i)
            for (j=0; j<15; ++j)
                p_ekf->Pp[i][j] = p[num++];

        // Read q
        getline(&line, &size, p_finp);
        int num_q = read_tokens(line, q, p_tok_info);
        assert(num_q == 225);    

	// Populate q
	num = 0;
	for (i=0; i<15; ++i)
	    for (j=0; j<15; ++j)
                p_ekf->Q[i][j] = q[num++];

	// Read delta
        getline(&line, &size, p_finp);
        int num_d = read_tokens(line, d, p_tok_info);
        assert(num_d == 1);    

  	// Read z
        getline(&line, &size, p_finp);
        int num_z = read_tokens(line, z, p_tok_info);
        assert(num_z == 15);    

  	// Read zu
        getline(&line, &size, p_finp);
        int num_zu = read_tokens(line, zu_temp, p_tok_info);
        assert(num_zu < 15);    
        for (i=0 ; i<num_zu ; i++)
	    zu[i] = (int)zu_temp[i];

        // Read r
        getline(&line, &size, p_finp);
        int num_r = read_tokens(line, r, p_tok_info);
        assert(num_r == 225);    

	// Print state before measurement
	print_state(p_tok_info[1], p_ekf, "k-", 0, 0, d[0], p_fout);
        se_update(z, zu, num_zu, r, p_ekf, (int)p_tok_info[1]);   
        print_msmt(p_tok_info[1], p_ekf, "k-", z, p_fout);
        print_state(p_tok_info[1], p_ekf, "k+", 1, 1, d[0], p_fout);
        print_msmt(p_tok_info[1], p_ekf, "k+", z, p_fout);
    }     
    fclose(p_finp);
    fclose(p_fout);
}

static void test_predict_update(ekf_t* p_ekf, const char* inp, const char* out)
{
    test_init(p_ekf);
    FILE* p_finp = fopen(inp, "r");
    if (p_finp == NULL)
    {
        printf("InputFile .. [BAD][val=%s]", inp);
        return;
    }         

    FILE* p_fout = fopen(out, "w");
    if (p_fout == NULL)
    {
        printf("OutputFile .. [BAD][val=%s]", out);
        return;
    }         

    char * line = NULL;
    size_t size = 0;
    int ret = 0;
    while ((ret = getline(&line, &size, p_finp)) != -1)
    {
        double prev[230], next[230], zu_temp[15];
        int zu[15];
        float p_tok_info[2];

        int num_t = read_tokens(line, prev, p_tok_info);
	assert (num_t <= 15);
        switch ((int)p_tok_info[0])
	{
	case 0: // predict
  	    print_state(p_tok_info[1], p_ekf, "k-1+", 1, 1, prev[0], p_fout);
            se_predict(prev[0], p_ekf);   
            print_state(p_tok_info[1], p_ekf, "k-", 0, 0, prev[0], p_fout);
	    break;
        case 1: // update
            getline(&line, &size, p_finp);
            int num_zu = read_tokens(line, zu_temp, p_tok_info);
            assert(num_zu < 15);    
            int i = 0;
            for (; i<num_zu ; i++)
	        zu[i] = (int)zu_temp[i];

	    // Read R
  	    getline(&line, &size, p_finp);
            int num_r = read_tokens(line, next, p_tok_info);
	    assert (num_r == 225);

            se_update(prev, zu, num_zu, next,  p_ekf, (int)p_tok_info[1]);   
            print_msmt(p_tok_info[1], p_ekf, "k-", prev, p_fout);
            print_state(p_tok_info[1], p_ekf, "k+", 0, 0, prev[0], p_fout);
            break;
        default: // error
  	    printf ("Input .. [BAD]");      
	} 
    }
}


int main()
{
    printf("Generating predict test output ... ");
    fflush(stdout);
    ekf_t ekf_predict;
    test_predict(&ekf_predict, "./data/out_predict_test.txt", "./data/out_se_predict.txt");  
    printf("[OK]\n");

    printf("Generating update test output ... ");
    fflush(stdout);
    ekf_t ekf_update;
    test_update(&ekf_update, "./data/out_update_test.txt", "./data/out_se_update.txt");  
    printf("[OK]\n");

    printf("Generating predict-update test output ... "); 
    fflush(stdout);
    ekf_t ekf;
    test_predict_update(&ekf, "./data/out_compact_test.txt", "./data/out_se.txt");  
    printf("[OK]\n");

    return 0;
}
