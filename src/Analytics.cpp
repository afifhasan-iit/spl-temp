// Analytics.cpp
#include "../include/Analytics.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

// Calculate daily returns
vector<double> Analytics::calculateDailyReturns(const Stock* stock) {
    vector<double> returns;
    int dataSize = stock->getDataSize();
    
    for (int i = 1; i < dataSize; i++) {
        double today = stock->getClosePrice(i);
        double yesterday = stock->getClosePrice(i - 1);
        
        if (yesterday != 0) {
            double dailyReturn = ((today - yesterday) / yesterday) * 100.0;
            returns.push_back(dailyReturn);
        }
    }
    
    return returns;
}

// Calculate cumulative return
double Analytics::calculateCumulativeReturn(const Stock* stock) {
    int dataSize = stock->getDataSize();
    if (dataSize < 2) return 0.0;
    
    double firstClose = stock->getClosePrice(0);
    double lastClose = stock->getClosePrice(dataSize - 1);
    
    if (firstClose == 0) return 0.0;
    
    double cumulativeReturn = ((lastClose - firstClose) / firstClose) * 100.0;
    
    return cumulativeReturn;
}

// Calculate volatility (annualized)
double Analytics::calculateVolatility(const vector<double>& returns) {
    if (returns.size() < 2) return 0.0;
    
    double stdDev = calculateStdDev(returns);
    
    // Annualize: multiply by sqrt(252) trading days
    double annualizedVol = stdDev * sqrt(252);
    
    return annualizedVol;
}

// Calculate Sharpe Ratio
double Analytics::calculateSharpeRatio(const vector<double>& returns, double riskFreeRate) {
    if (returns.size() < 2) return 0.0;
    
    double avgReturn = calculateMean(returns);
    double vol = calculateVolatility(returns);
    
    if (vol == 0.0) return 0.0;
    
    // Annualize average return: multiply by 252
    double annualizedReturn = avgReturn * 252;
    
    // Sharpe = (Return - Risk Free Rate) / Volatility
    double sharpe = (annualizedReturn - riskFreeRate) / vol;
    
    return sharpe;
}

// Calculate Maximum Drawdown
double Analytics::calculateMaxDrawdown(const Stock* stock) {
    int dataSize = stock->getDataSize();
    if (dataSize < 2) return 0.0;
    
    double maxDrawdown = 0.0;
    double peak = stock->getClosePrice(0);
    
    for (int i = 1; i < dataSize; i++) {
        double currentPrice = stock->getClosePrice(i);
        
        // Update peak if current price is higher
        if (currentPrice > peak) {
            peak = currentPrice;
        }
        
        // Calculate drawdown from peak
        double drawdown = ((peak - currentPrice) / peak) * 100.0;
        
        // Update max drawdown
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }
    }
    
    return maxDrawdown;
}

// Display analytics report
void Analytics::displayAnalyticsReport(const Stock* stock) {
    cout << "\n=== Analytics Report for " << stock->getSymbol() << " ===" << endl;
    cout << fixed << setprecision(2);
    
    vector<double> returns = calculateDailyReturns(stock);
    
    cout << "\nPerformance Metrics:" << endl;
    cout << "-----------------------------------" << endl;
    cout << "Cumulative Return: " << calculateCumulativeReturn(stock) << "%" << endl;
    cout << "Volatility (Annualized): " << calculateVolatility(returns) << "%" << endl;
    cout << "Sharpe Ratio: " << calculateSharpeRatio(returns) << endl;
    cout << "Maximum Drawdown: " << calculateMaxDrawdown(stock) << "%" << endl;
    cout << "Total Days: " << stock->getDataSize() << endl;
}

// Helper: Calculate mean
double Analytics::calculateMean(const vector<double>& data) {
    if (data.empty()) return 0.0;
    
    double sum = 0.0;
    for (double val : data) {
        sum += val;
    }
    
    return sum / data.size();
}

// Helper: Calculate standard deviation
double Analytics::calculateStdDev(const vector<double>& data) {
    if (data.size() < 2) return 0.0;
    
    double mean = calculateMean(data);
    double variance = 0.0;
    
    for (double val : data) {
        double diff = val - mean;
        variance += diff * diff;
    }
    
    variance /= data.size();
    
    return sqrt(variance);
}