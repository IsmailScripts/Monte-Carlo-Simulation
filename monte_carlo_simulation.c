#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NUM_STEPS 252            // Number of steps (trading days in a year)
#define NUM_PATHS 100000000    // Total number of paths, adjust this for timing

// Function to generate standard normal random numbers
double rand_normal() {
    double u = ((double) rand() / RAND_MAX);
    double v = ((double) rand() / RAND_MAX);
    return sqrt(-2.0 * log(u)) * cos(2.0 * M_PI * v);
}

// Function to simulate a single path
double simulate_path(double S0, double mu, double sigma, double T, int steps) {
    double dt = T / steps;
    double S = S0;

    for (int i = 0; i < steps; i++) {
        double dWt = rand_normal() * sqrt(dt);
        S *= exp((mu - 0.5 * sigma * sigma) * dt + sigma * dWt);
    }

    return S;
}

int main() {
    srand(time(NULL));

    FILE *file = fopen("data.csv", "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    double total_log_return = 0.0;
    double total_log_return_squared = 0.0;
    int count = 0;
    double previous_close = 0.0;
    double initial_price = 0.0;
    char line[256];

    // Skip the header line
    fgets(line, sizeof(line), file);

    
   // Read each line of the CSV file
while (fgets(line, sizeof(line), file)) {
    double open_price, high_price, low_price, close_price, adj_close_price;
    int volume;
    
    // Use a format string to capture all necessary values, skipping the unwanted ones
    if (sscanf(line, "%*[^,],%lf,%lf,%lf,%lf,%lf,%d", 
                &open_price, &high_price, &low_price, &close_price, &adj_close_price, &volume) == 6) {
        if (count == 0) {
            initial_price = close_price;  // Set initial price from the first closing price
        } else {
            double log_return = log(close_price / previous_close);  // Calculate log return
            total_log_return += log_return;
            total_log_return_squared += log_return * log_return;
        }
        previous_close = close_price;  // Update the previous close price
        count++;
    }
}

    fclose(file);

    // Calculate drift (average log return) and volatility (std deviation of log returns)
    double avg_log_return = total_log_return / (count - 1);
    double variance_log_return = (total_log_return_squared / (count - 1)) - (avg_log_return * avg_log_return);
    double volatility = sqrt(variance_log_return) * sqrt(NUM_STEPS); // Annualized volatility
    double drift = avg_log_return * NUM_STEPS;  // Annualized drift

    double sum_final_prices = 0.0;
    double sum_final_prices_squared = 0.0;

    // Start timing
    clock_t start_time = clock();

    // Run the Monte Carlo simulation
    for (int i = 0; i < NUM_PATHS; i++) {
        double final_price = simulate_path(initial_price, drift, volatility, 1.0, NUM_STEPS);
        sum_final_prices += final_price;
        sum_final_prices_squared += final_price * final_price;
    }

    // End timing
    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Calculate mean and variance of final stock prices
    double mean_final_price = sum_final_prices / NUM_PATHS;
    double mean_final_price_squared = sum_final_prices_squared / NUM_PATHS;
    double variance_final_price = mean_final_price_squared - mean_final_price * mean_final_price;
    double std_dev_final_price = sqrt(variance_final_price);

    // Display results
    printf("Monte Carlo Simulation Result:\n");
    printf("Expected final stock price: %.2f\n", mean_final_price);
    printf("Standard deviation of final stock prices: %.2f\n", std_dev_final_price);
    printf("Total execution time: %.2f seconds\n", time_taken);
return 0;}
