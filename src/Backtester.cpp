// Backtester.cpp
#include "../include/Backtester.h"
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

Backtester::Backtester(Stock* s, Strategy* strat, double initialCash) {
    stock = s;
    strategy = strat;
    startingCash = initialCash;
    cash = initialCash;
    shares = 0;
    finalValue = 0.0;
    totalReturn = 0.0;
    numTrades = 0;
    winningTrades = 0;
    maxDrawdown = 0.0;
}

void Backtester::run() {
    int dataSize = stock->getDataSize();
    bool holding = false;
    double buyPrice = 0.0;
    double peak = startingCash;
    
    cout << "\nRunning backtest for: " << strategy->getName() << endl;
    cout << "Starting cash: $" << startingCash << endl;
    cout << "Stock: " << stock->getSymbol() << endl;
    cout << "Processing..." << endl;
    
    // Go through each day
    for (int day = 0; day < dataSize; day++) {
        double currentPrice = stock->getClosePrice(day);
        
        // Check buy signal
        if (strategy->shouldBuy(stock, day, holding)) {
            // Calculate how many shares we can buy
            int sharesToBuy = cash / currentPrice;
            
            if (sharesToBuy > 0) {
                double cost = sharesToBuy * currentPrice;
                cash -= cost;
                shares = sharesToBuy;
                holding = true;
                buyPrice = currentPrice;
                
                Trade t;
                t.day = day;
                t.type = "BUY";
                t.price = currentPrice;
                t.shares = sharesToBuy;
                trades.push_back(t);
                numTrades++;
            }
        }
        // Check sell signal
        else if (strategy->shouldSell(stock, day, holding)) {
            if (shares > 0) {
                double revenue = shares * currentPrice;
                cash += revenue;
                
                // Check if winning trade
                if (currentPrice > buyPrice) {
                    winningTrades++;
                }
                
                Trade t;
                t.day = day;
                t.type = "SELL";
                t.price = currentPrice;
                t.shares = shares;
                trades.push_back(t);
                numTrades++;
                
                shares = 0;
                holding = false;
            }
        }
        
        // Calculate current portfolio value
        double portfolioValue = cash + (shares * currentPrice);
        
        // Track max drawdown
        if (portfolioValue > peak) {
            peak = portfolioValue;
        }
        double drawdown = ((peak - portfolioValue) / peak) * 100.0;
        if (drawdown > maxDrawdown) {
            maxDrawdown = drawdown;
        }
    }
    
    // If still holding at end, sell at last price
    if (shares > 0) {
        double lastPrice = stock->getClosePrice(dataSize - 1);
        cash += shares * lastPrice;
        shares = 0;
    }
    
    // Calculate final results
    finalValue = cash;
    totalReturn = ((finalValue - startingCash) / startingCash) * 100.0;
    
    cout << "✓ Backtest complete!" << endl;
}

void Backtester::displayResults() const {
    cout << "\n========================================" << endl;
    cout << "    BACKTEST RESULTS" << endl;
    cout << "========================================" << endl;
    cout << "Strategy: " << strategy->getName() << endl;
    cout << "Stock: " << stock->getSymbol() << endl;
    cout << "----------------------------------------" << endl;
    cout << fixed << setprecision(2);
    
    cout << "\nPerformance:" << endl;
    cout << "  Starting Capital: $" << startingCash << endl;
    cout << "  Final Value: $" << finalValue << endl;
    cout << "  Total Return: " << totalReturn << "%" << endl;
    cout << "  Max Drawdown: " << maxDrawdown << "%" << endl;
    
    cout << "\nTrading Activity:" << endl;
    cout << "  Total Trades: " << numTrades << endl;
    
    if (numTrades > 0) {
        int completedTrades = numTrades / 2;  // Buy + Sell = 1 complete trade
        if (completedTrades > 0) {
            double winRate = (static_cast<double>(winningTrades) / completedTrades) * 100.0;
            cout << "  Winning Trades: " << winningTrades << " / " << completedTrades << endl;
            cout << "  Win Rate: " << winRate << "%" << endl;
        }
    }
    
    // Show recent trades
    if (trades.size() > 0) {
        cout << "\nRecent Trades (last 5):" << endl;
        int start = max(0, (int)trades.size() - 5);
        for (int i = start; i < trades.size(); i++) {
            cout << "  Day " << trades[i].day << ": " 
                 << trades[i].type << " " 
                 << trades[i].shares << " shares @ $" 
                 << trades[i].price << endl;
        }
    }
    
    cout << "========================================" << endl;
}

double Backtester::getTotalReturn() const {
    return totalReturn;
}

double Backtester::getFinalValue() const {
    return finalValue;
}