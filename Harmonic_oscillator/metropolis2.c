#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

void geometry (int N, int* np, int* ns);
void initialize_lattice(int N, int iflag, double* field);
void update_metropolis (double* acc, double* rej, int N, double eta, double d_metro, double* field, int* np, int* ns);
double internal_energy(int N, double eta, double *field);
double y2_mean(int N, double *field);
double dy2_mean(int N, double *field);
double two_point_connected_function(double *field, int N, int k);


int main(){
	int iflag, measures, i_decorrel, i_term, k=0;
	double a, eta, d_metro, bh=5, omega=4, sum;
    double acc=0, rej=0, acc_over_rej;
    FILE *input_file, *field_out_file, *energy_file, *mean_y2_file, *mean_dy2_file, *C2_file;
    double *field, **C2;
    double U_n, y2mean, dy2mean, Ctau;  // U_n is the intern energy normalized over h/2pi * omega
    int *np, *ns;
    char field_out_filename[60], energy_filename[60], mean_y2_filename[60], mean_dy2_filename[60], C2_filename[60];
    
    srand(time(NULL));
    //N*a=bh
    //N*eta=bh*omega


    //// open input files to get some parameters ////

	input_file = fopen("input.txt", "r");
	if(input_file==NULL){
    	perror("Errore in apertura del file di input");
        exit(1);
    }

    //// read parameters from input file ////

    fscanf(input_file,"%d",&iflag);       // start as warm/cold/previous
    fscanf(input_file,"%d",&measures);    // number of measures
    fscanf(input_file,"%d",&i_decorrel);  // update time of physical quantities 
    fscanf(input_file,"%d",&i_term);      // thermalization steps

    fclose(input_file);


    //// cycle over different chain lenghts (N) ////

    for (int N=20; N<50; N+=10){
        
        // file with last field
        sprintf(field_out_filename, "./results/field/field_out_file_N_%d.txt", N);
        field_out_file = fopen(field_out_filename, "w");
        if(field_out_file==NULL){
            perror("Errore in apertura del file out");
            exit(1);
        }

        // file with y^2 mean values
        /*sprintf(mean_y2_filename, "./results/output/mean_y2/bh_%.0lf_omega_%.0lf/mean_y2_N_%d.txt", bh, omega, N);
        mean_y2_file = fopen(mean_y2_filename, "w");
        if(mean_y2_file==NULL){
            perror("Errore in apertura del file");
            exit(1);
        }*/

        // file with dy^2 mean values
        /*sprintf(mean_dy2_filename, "./results/output/mean_dy2/bh_%.0lf_omega_%.0lf/mean_dy2_N_%d.txt", bh, omega, N);
        mean_dy2_file = fopen(mean_dy2_filename, "w");
        if(mean_dy2_file==NULL){
            perror("Errore in apertura del file");
            exit(1);
        }*/

        // file with energy values
        /*sprintf(energy_filename, "./results/output/energy/bh_%.0lf_omega_%.0lf/energy_N_%d.txt", bh, omega, N);
        energy_file = fopen(energy_filename, "w");
        if(energy_file==NULL){
            perror("Errore in apertura del file");
            exit(1);
        }*/
        
        // file with C2 (two-point function)
        sprintf(C2_filename, "./results/output/C2/bh_%.0lf_omega_%.0lf/C2_N_%d.txt", bh, omega, N);
        C2_file = fopen(C2_filename, "w");
        if(C2_file==NULL){
            perror("Errore in apertura del file");
            exit(1);
        }
        

        field = calloc(N, sizeof(double));
        np = calloc(N, sizeof(int));
        ns = calloc(N, sizeof(int));

        //C2[i][j]: i run over measures, j run over tau (in [0,bh])
        // I will take, for each tau (j), the mean over measures (i)
        C2 = calloc(measures, sizeof(double*));
        for(int i=0; i<measures; i++){
            C2[i] = calloc(N, sizeof(double));
        }


        //// PARAMETERS SETTING ////

        eta = bh*omega / N;
        a = bh / N;
        d_metro = 2*sqrt(eta);
        printf("eta=%lf\n", eta);


        // initialize the field and set the boundary conditions
        initialize_lattice(N, iflag, field);
        geometry(N, np, ns);


        //// METROPOLIS ////

        // termalization
        for(int i=0; i<i_term; i++){
        	update_metropolis(&acc, &rej, N, eta, d_metro, field, np, ns);
        }
        
        for (int i=0; i<measures; i++){
            
            // take the measures after i_decorrel times
            for (int j=0; j<i_decorrel; j++){
        	   update_metropolis(&acc, &rej, N, eta, d_metro, field, np, ns);
            }

            // MEASURES //
            
            // internal energy
            /*U_n = internal_energy(N, eta, field);
            fprintf(energy_file, "%lf\n", U_n);*/

            // mean of y^2
            /*y2mean = y2_mean(N, field);
            fprintf(mean_y2_file, "%lf\n", y2mean);*/

            // mean of dy^2
            /*dy2mean = dy2_mean(N, field);
            fprintf(mean_dy2_file, "%lf\n", dy2mean);*/

            // two-point function for each measure
            for (int tau=0; tau<N; tau++){
                C2[i][tau] = two_point_connected_function(field, N, tau);
            }
            
        }
        
        // write over file C2
        for(int tau=0; tau<N; tau++){
            sum = 0;
            for(int i=0; i<measures; i++){
                sum += C2[i][tau];
            }
            sum = sum / measures;
            fprintf(C2_file, "%lf   %lf\n", sum, tau*a);
        }
        
        
        // compute the percentage acceptance
        acc_over_rej = acc/(acc+rej)*100;
        printf("percentage acceptance = %lf\n", acc_over_rej);
        acc=0;
        rej=0;


        //// write over file the last chain ////
        for (int i=0; i<N; i++){
        	fprintf(field_out_file, "%lf\n", field[i]);
        }


        // close all files
        fclose(field_out_file);
        //fclose(mean_y2_file);
        //fclose(mean_dy2_file);
        //fclose(energy_file);
        fclose(C2_file);
        
        // free the malloc
        free(field);
        free(np);
        free(ns);
        free(C2);

    }

	return 0;
}


//-------------------------------------------------//
// FUNCTIONS DEFINITION
//-------------------------------------------------//

// take into account the boundary conditions
void geometry (int N, int *np, int *ns){

    for(int i=1; i<N; i++){
        np[i] = i-1;
    }
    np[0] = N-1;

    for(int i=0; i<N-1; i++){
        ns[i] = i+1;
    }
    ns[N-1] = 0;

    return;
}

//-------------------------------------------------//

// initialize the chain as cold (all zeros), warm (random), or previous
void initialize_lattice(int N, int iflag, double *field) {
    double x;
    FILE *data_out;
    
    // cold start
    if (iflag == 0){
        for (int i=0; i<N; i++){
            field[i] = 0.0;
        }
    }

    // warm start
    else if (iflag == 1){
        for (int i=0; i<N; i++){
            x = rand();
            field[i] = x/RAND_MAX - 0.5;
        }
    }

    // previous-chain start
    else {
        data_out = fopen("out_file.txt","r");
        for(int i=0; i<N; i++){
            fscanf(data_out,"%lf",&field[i]);
        }
        fclose(data_out);
    }

return ;
}

//-------------------------------------------------//

//make the metropolis step one time for each step of the chain (field)
void update_metropolis (double *acc, double *rej, int N, double eta, double d_metro, double *field, int *np, int *ns){
    int p_i, s_i;
    double field_P, c1, c2, x, r, dS;

    c1 = 1/eta;
    c2 = eta/2 + 1/eta;

    for (int i=0; i<N; i++){
        x = rand();
        x = x/RAND_MAX;
        p_i = np[i];
        s_i = ns[i];

        field_P = field[i] + d_metro*(2*x-1);

        dS = c1*(field_P-field[i])*(field[s_i]+field[p_i]) + c2*(pow(field[i],2)-pow(field_P,2));
        r = exp(dS);

        x = rand();
        x = x/RAND_MAX;

        if (x<r){
            *acc += 1;
            field[i] = field_P;
        }
        else{
            *rej += 1;
        }
    }

    return;
}

//-------------------------------------------------//

// compute the internal energy
double internal_energy(int N, double eta, double *field){
    double mean_dy2=0, mean_y2=0;

    for (int j=0; j<N-1; j++){
        mean_dy2 = pow((field[j+1]-field[j]),2);
        mean_y2 = pow(field[j],2);
    }

    mean_dy2 = 1./N * mean_dy2;
    mean_y2 = 1./N * (mean_y2 + pow(field[N-1],2));

    return -1./(2*pow(eta,2)) * mean_dy2 + 1./2 * mean_y2 + eta/2;
}

//-------------------------------------------------//

// compute the mean of y^2
double y2_mean(int N, double *field){
    double mean_y2=0;

    for (int i=0; i<N; i++){
        mean_y2 += pow(field[i],2);
    }

    return mean_y2 * 1./N;
}

//-------------------------------------------------//

// compute the mean of delta_y^2
double dy2_mean(int N, double *field){
    double mean_dy2=0;

    for (int j=0; j<N-1; j++){
        mean_dy2 = mean_dy2 + pow((field[j+1]-field[j]),2);
    }

    return mean_dy2 * 1./N;
}

//-------------------------------------------------//

// compute the two-point connected function
double two_point_connected_function(double *field, int N, int k){
    double C2=0, sum=0;
    int j=0;

    while (j+k < N){
        C2 += field[j+k]*field[j];
        j += 1;
    }

    for(int i=0; i<N; i++){
        sum += field[i];
    }

    return C2/j - sum/N;
}