// Backtester.h
#ifndef BACKTESTER_H
#define BACKTESTER_H

#include "Stock.h"
#include "Strategy.h"
#include <vector>
#include <string>

using namespace std;

struct Trade {
    int day;
    string type;      // "BUY" or "SELL"
    double price;
    int shares;
};

class Backtester {
private:
    Stock* stock;
    Strategy* strategy;
    double startingCash;
    
    // Results
    double cash;
    int shares;
    vector<Trade> trades;
    double finalValue;
    double totalReturn;
    int numTrades;
    int winningTrades;
    double maxDrawdown;
    
public:
    Backtester(Stock* s, Strategy* strat, double initialCash = 10000.0);
    
    // Run the backtest
    void run();
    
    // Display results
    void displayResults() const;
    
    // Getters
    double getTotalReturn() const;
    double getFinalValue() const;
};

#endif