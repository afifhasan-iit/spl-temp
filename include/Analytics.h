// Analytics.h
#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <vector>
#include "Stock.h"

using namespace std;

class Analytics {
public:
    // Calculate daily returns for a stock
    static vector<double> calculateDailyReturns(const Stock* stock);
    
    // Calculate cumulative return
    static double calculateCumulativeReturn(const Stock* stock);
    
    // Calculate volatility (annualized standard deviation)
    static double calculateVolatility(const vector<double>& returns);
    
    // Calculate Sharpe Ratio
    static double calculateSharpeRatio(const vector<double>& returns, double riskFreeRate = 0.02);
    
    // Calculate Maximum Drawdown
    static double calculateMaxDrawdown(const Stock* stock);
    
    // Display analytics report for a stock
    static void displayAnalyticsReport(const Stock* stock);
    
private:
    // Helper: Calculate mean of a vector
    static double calculateMean(const vector<double>& data);
    
    // Helper: Calculate standard deviation
    static double calculateStdDev(const vector<double>& data);
};

#endif