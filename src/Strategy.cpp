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

// MACD Strategy
MACDStrategy::MACDStrategy() : Strategy("MACD Strategy") {}

bool MACDStrategy::shouldBuy(Stock* stock, int day, bool currentlyHolding) {
    if (currentlyHolding) return false;
    if (day < 35) return false;  // Need enough data for MACD
    
    double macd = stock->getMACD(day);
    double signal = stock->getMACDSignal(day);
    double prevMacd = stock->getMACD(day - 1);
    double prevSignal = stock->getMACDSignal(day - 1);
    
    if (macd == 0 || signal == 0) return false;
    
    // Buy when MACD crosses above Signal
    bool crossedAbove = (prevMacd <= prevSignal) && (macd > signal);
    
    return crossedAbove;
}

bool MACDStrategy::shouldSell(Stock* stock, int day, bool currentlyHolding) {
    if (!currentlyHolding) return false;
    if (day < 35) return false;
    
    double macd = stock->getMACD(day);
    double signal = stock->getMACDSignal(day);
    double prevMacd = stock->getMACD(day - 1);
    double prevSignal = stock->getMACDSignal(day - 1);
    
    if (macd == 0 || signal == 0) return false;
    
    // Sell when MACD crosses below Signal
    bool crossedBelow = (prevMacd >= prevSignal) && (macd < signal);
    
    return crossedBelow;
}

// Momentum Strategy
MomentumStrategy::MomentumStrategy() : Strategy("Momentum Strategy") {}

bool MomentumStrategy::shouldBuy(Stock* stock, int day, bool currentlyHolding) {
    if (currentlyHolding) return false;
    if (day < 10) return false;  // Need data for momentum
    
    double momentum = stock->getMomentum(day);
    
    // Buy when momentum is strongly positive (> 5%)
    return (momentum > 5.0);
}

bool MomentumStrategy::shouldSell(Stock* stock, int day, bool currentlyHolding) {
    if (!currentlyHolding) return false;
    
    double momentum = stock->getMomentum(day);
    
    // Sell when momentum turns negative (< -5%)
    return (momentum < -5.0);
}