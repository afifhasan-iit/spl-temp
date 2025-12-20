// Strategy.cpp
#include "../include/Strategy.h"

using namespace std;

// Base Strategy
Strategy::Strategy(string strategyName) {
    name = strategyName;
}

string Strategy::getName() const {
    return name;
}

// RSI Strategy
RSIStrategy::RSIStrategy() : Strategy("RSI Strategy") {}

bool RSIStrategy::shouldBuy(Stock* stock, int day, bool currentlyHolding) {
    if (currentlyHolding) return false;
    
    double rsi = stock->getRSI(day);
    return (rsi > 0 && rsi < 30);  // Oversold
}

bool RSIStrategy::shouldSell(Stock* stock, int day, bool currentlyHolding) {
    if (!currentlyHolding) return false;
    
    double rsi = stock->getRSI(day);
    return (rsi > 70);  // Overbought
}

// Moving Average Crossover Strategy
MAStrategy::MAStrategy() : Strategy("Moving Average Crossover") {
    previousCrossAbove = false;
}

bool MAStrategy::shouldBuy(Stock* stock, int day, bool currentlyHolding) {
    if (currentlyHolding) return false;
    if (day < 50) return false;  // Need enough data for SMA50
    
    double sma20 = stock->getSMA20(day);
    double sma50 = stock->getSMA50(day);
    double prevSma20 = stock->getSMA20(day - 1);
    double prevSma50 = stock->getSMA50(day - 1);
    
    if (sma20 == 0 || sma50 == 0) return false;
    
    // Crossover: SMA20 was below SMA50, now above
    bool crossedAbove = (prevSma20 <= prevSma50) && (sma20 > sma50);
    
    return crossedAbove;
}

bool MAStrategy::shouldSell(Stock* stock, int day, bool currentlyHolding) {
    if (!currentlyHolding) return false;
    if (day < 50) return false;
    
    double sma20 = stock->getSMA20(day);
    double sma50 = stock->getSMA50(day);
    double prevSma20 = stock->getSMA20(day - 1);
    double prevSma50 = stock->getSMA50(day - 1);
    
    if (sma20 == 0 || sma50 == 0) return false;
    
    // Crossover: SMA20 was above SMA50, now below
    bool crossedBelow = (prevSma20 >= prevSma50) && (sma20 < sma50);
    
    return crossedBelow;
}

// Buy and Hold Strategy
BuyHoldStrategy::BuyHoldStrategy() : Strategy("Buy and Hold") {}

bool BuyHoldStrategy::shouldBuy(Stock* stock, int day, bool currentlyHolding) {
    // Buy only on first valid day (after indicators calculated)
    return (!currentlyHolding && day >= 50);
}

bool BuyHoldStrategy::shouldSell(Stock* stock, int day, bool currentlyHolding) {
    // Never sell
    return false;
}