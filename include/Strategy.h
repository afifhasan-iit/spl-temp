// Strategy.h
#ifndef STRATEGY_H
#define STRATEGY_H

#include "Stock.h"
#include <string>

using namespace std;

// Base class for all trading strategies
class Strategy {
protected:
    string name;
    
public:
    Strategy(string strategyName);
    virtual ~Strategy() {}
    
    // Check if should buy on this day
    virtual bool shouldBuy(Stock* stock, int day, bool currentlyHolding) = 0;
    
    // Check if should sell on this day
    virtual bool shouldSell(Stock* stock, int day, bool currentlyHolding) = 0;
    
    string getName() const;
};

// RSI Strategy: Buy when RSI < 30, Sell when RSI > 70
class RSIStrategy : public Strategy { 
public:
    RSIStrategy();
    bool shouldBuy(Stock* stock, int day, bool currentlyHolding) override;
    bool shouldSell(Stock* stock, int day, bool currentlyHolding) override;
};

// Moving Average Crossover: Buy when SMA20 > SMA50, Sell when SMA20 < SMA50
class MAStrategy : public Strategy {
private:
    bool previousCrossAbove;
    
public:
    MAStrategy();
    bool shouldBuy(Stock* stock, int day, bool currentlyHolding) override;
    bool shouldSell(Stock* stock, int day, bool currentlyHolding) override;
};

// Buy and Hold: Buy on first day, never sell
class BuyHoldStrategy : public Strategy {
public:
    BuyHoldStrategy();
    bool shouldBuy(Stock* stock, int day, bool currentlyHolding) override;
    bool shouldSell(Stock* stock, int day, bool currentlyHolding) override;
};

#endif