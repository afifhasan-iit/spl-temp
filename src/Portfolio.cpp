// Portfolio.cpp
#include "../include/Portfolio.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

// Constructor
Portfolio::Portfolio(string portfolioName) {
    name = portfolioName;
    cashBalance = 0.0;
}

// Buy stock
void Portfolio::buyStock(string symbol, int quantity, double price, string date) {
    double totalCost = quantity * price;
    
    // Check if enough cash
    if (totalCost > cashBalance) {
        cout << "Error: Not enough cash. Need $" << totalCost 
             << " but have $" << cashBalance << endl;
        return;
    }
    
    // Deduct cash
    cashBalance -= totalCost;
    
    // Add or update holding
    if (holdings.find(symbol) != holdings.end()) {
        // Already own this stock - update average cost
        Holding& h = holdings[symbol];
        double totalValue = (h.quantity * h.avgCost) + totalCost;
        h.quantity += quantity;
        h.avgCost = totalValue / h.quantity;
    } else {
        // New stock
        Holding h;
        h.symbol = symbol;
        h.quantity = quantity;
        h.avgCost = price;
        h.purchaseDate = date;
        holdings[symbol] = h;
    }
    
    // Record transaction
    string transaction = "BUY " + to_string(quantity) + " " + symbol + 
                        " @ $" + to_string(price) + " on " + date;
    transactions.push_back(transaction);
    
    cout << "✓ Bought " << quantity << " shares of " << symbol << endl;
}

// Sell stock
void Portfolio::sellStock(string symbol, int quantity, double price, string date) {
    // Check if we own this stock
    if (holdings.find(symbol) == holdings.end()) {
        cout << "Error: You don't own " << symbol << endl;
        return;
    }
    
    Holding& h = holdings[symbol];
    
    // Check if enough quantity
    if (h.quantity < quantity) {
        cout << "Error: You only have " << h.quantity << " shares of " << symbol << endl;
        return;
    }
    
    // Add cash from sale
    double revenue = quantity * price;
    cashBalance += revenue;
    
    // Update holding
    h.quantity -= quantity;
    
    // Remove if quantity is 0
    if (h.quantity == 0) {
        holdings.erase(symbol);
    }
    
    // Record transaction
    string transaction = "SELL " + to_string(quantity) + " " + symbol + 
                        " @ $" + to_string(price) + " on " + date;
    transactions.push_back(transaction);
    
    cout << "✓ Sold " << quantity << " shares of " << symbol << endl;
}

// Getters
string Portfolio::getName() const {
    return name;
}

double Portfolio::getCashBalance() const {
    return cashBalance;
}

void Portfolio::addCash(double amount) {
    cashBalance += amount;
    cout << "✓ Added $" << amount << " to portfolio" << endl;
}

// Display holdings
void Portfolio::displayHoldings() const {
    cout << "\n=== Holdings in '" << name << "' ===" << endl;
    
    if (holdings.empty()) {
        cout << "No holdings yet." << endl;
        return;
    }
    
    cout << fixed << setprecision(2);
    cout << "\nSymbol\tQuantity\tAvg Cost\tTotal Cost" << endl;
    cout << "------------------------------------------------" << endl;
    
    for (const auto& pair : holdings) {
        const Holding& h = pair.second;
        double totalCost = h.quantity * h.avgCost;
        
        cout << h.symbol << "\t" 
             << h.quantity << "\t\t$"
             << h.avgCost << "\t\t$"
             << totalCost << endl;
    }
    
    cout << "\nCash Balance: $" << cashBalance << endl;
}

// Display transactions
void Portfolio::displayTransactions() const {
    cout << "\n=== Transaction History ===" << endl;
    
    if (transactions.empty()) {
        cout << "No transactions yet." << endl;
        return;
    }
    
    for (int i = 0; i < transactions.size(); i++) {
        cout << i + 1 << ". " << transactions[i] << endl;
    }
}

// Display summary with current values
void Portfolio::displaySummary(const map<string, Stock*>& stockData) const {
    cout << "\n=== Portfolio Summary: '" << name << "' ===" << endl;
    
    if (holdings.empty()) {
        cout << "No holdings yet." << endl;
        cout << "Cash Balance: $" << fixed << setprecision(2) << cashBalance << endl;
        return;
    }
    
    cout << fixed << setprecision(2);
    cout << "\nSymbol\tQty\tAvg Cost\tCurrent\t\tProfit/Loss" << endl;
    cout << "------------------------------------------------------------" << endl;
    
    double totalValue = 0.0;
    double totalCost = 0.0;
    
    for (const auto& pair : holdings) {
        const Holding& h = pair.second;
        double costBasis = h.quantity * h.avgCost;
        totalCost += costBasis;
        
        // Get current price if stock data available
        if (stockData.find(h.symbol) != stockData.end()) {
            Stock* stock = stockData.at(h.symbol);
            int lastIndex = stock->getDataSize() - 1;
            double currentPrice = stock->getClosePrice(lastIndex);
            double currentValue = h.quantity * currentPrice;
            double profitLoss = currentValue - costBasis;
            double profitLossPct = (profitLoss / costBasis) * 100.0;
            
            totalValue += currentValue;
            
            cout << h.symbol << "\t" << h.quantity << "\t$" << h.avgCost 
                 << "\t\t$" << currentPrice << "\t\t";
            
            if (profitLoss >= 0) {
                cout << "+$" << profitLoss << " (+" << profitLossPct << "%)";
            } else {
                cout << "-$" << abs(profitLoss) << " (" << profitLossPct << "%)";
            }
            cout << endl;
        } else {
            cout << h.symbol << "\t" << h.quantity << "\t$" << h.avgCost 
                 << "\t\tN/A\t\tN/A (Load stock data)" << endl;
            totalValue += costBasis;  // Use cost basis if no current price
        }
    }
    
    cout << "\n-----------------------------------------------------------" << endl;
    cout << "Total Cost Basis: $" << totalCost << endl;
    cout << "Current Holdings Value: $" << totalValue << endl;
    cout << "Cash Balance: $" << cashBalance << endl;
    cout << "Total Portfolio Value: $" << (totalValue + cashBalance) << endl;
    
    double totalProfitLoss = (totalValue + cashBalance) - (totalCost + cashBalance);
    double totalProfitLossPct = (totalProfitLoss / totalCost) * 100.0;
    
    cout << "Total Profit/Loss: ";
    if (totalProfitLoss >= 0) {
        cout << "+$" << totalProfitLoss << " (+" << totalProfitLossPct << "%)";
    } else {
        cout << "-$" << abs(totalProfitLoss) << " (" << totalProfitLossPct << "%)";
    }
    cout << endl;
}

// Display detailed summary with analytics
void Portfolio::displayDetailedSummary(const map<string, Stock*>& stockData) const {
    displaySummary(stockData);
    
    if (holdings.empty() || stockData.empty()) return;
    
    cout << "\n=== Holdings Breakdown ===" << endl;
    
    double totalValue = cashBalance;
    for (const auto& pair : holdings) {
        const Holding& h = pair.second;
        if (stockData.find(h.symbol) != stockData.end()) {
            Stock* stock = stockData.at(h.symbol);
            int lastIndex = stock->getDataSize() - 1;
            double currentPrice = stock->getClosePrice(lastIndex);
            totalValue += h.quantity * currentPrice;
        } else {
            totalValue += h.quantity * h.avgCost;
        }
    }
    
    for (const auto& pair : holdings) {
        const Holding& h = pair.second;
        double holdingValue = h.quantity * h.avgCost;
        
        if (stockData.find(h.symbol) != stockData.end()) {
            Stock* stock = stockData.at(h.symbol);
            int lastIndex = stock->getDataSize() - 1;
            double currentPrice = stock->getClosePrice(lastIndex);
            holdingValue = h.quantity * currentPrice;
        }
        
        double allocation = (holdingValue / totalValue) * 100.0;
        
        cout << h.symbol << ": " << allocation << "% of portfolio" << endl;
    }
    
    double cashAllocation = (cashBalance / totalValue) * 100.0;
    cout << "Cash: " << cashAllocation << "% of portfolio" << endl;
}

// Check functions
bool Portfolio::hasStock(string symbol) const {
    return holdings.find(symbol) != holdings.end();
}

int Portfolio::getQuantity(string symbol) const {
    if (holdings.find(symbol) != holdings.end()) {
        return holdings.at(symbol).quantity;
    }
    return 0;
}

// Save/Load functions
bool Portfolio::saveToFile(string filename) const {
    ofstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error: Could not save portfolio to " << filename << endl;
        return false;
    }
    
    // Save portfolio name
    file << "PORTFOLIO_NAME," << name << endl;
    
    // Save cash balance
    file << "CASH," << cashBalance << endl;
    
    // Save holdings
    file << "HOLDINGS" << endl;
    for (const auto& pair : holdings) {
        const Holding& h = pair.second;
        file << h.symbol << "," << h.quantity << "," 
             << h.avgCost << "," << h.purchaseDate << endl;
    }
    
    // Save transactions
    file << "TRANSACTIONS" << endl;
    for (const string& trans : transactions) {
        file << trans << endl;
    }
    
    file.close();
    return true;
}

bool Portfolio::loadFromFile(string filename) {
    ifstream file(filename);
    
    if (!file.is_open()) {
        cout << "Error: Could not load portfolio from " << filename << endl;
        return false;
    }
    
    string line;
    string section = "";
    
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        // Check for section headers
        if (line == "HOLDINGS") {
            section = "HOLDINGS";
            continue;
        } else if (line == "TRANSACTIONS") {
            section = "TRANSACTIONS";
            continue;
        }
        
        // Parse based on section
        if (section == "") {
            // Parse header info
            size_t commaPos = line.find(',');
            if (commaPos != string::npos) {
                string key = line.substr(0, commaPos);
                string value = line.substr(commaPos + 1);
                
                if (key == "PORTFOLIO_NAME") {
                    name = value;
                } else if (key == "CASH") {
                    cashBalance = stod(value);
                }
            }
        } else if (section == "HOLDINGS") {
            // Parse holdings: symbol,quantity,avgCost,date
            stringstream ss(line);
            string symbol, qtyStr, costStr, date;
            
            getline(ss, symbol, ',');
            getline(ss, qtyStr, ',');
            getline(ss, costStr, ',');
            getline(ss, date, ',');
            
            Holding h;
            h.symbol = symbol;
            h.quantity = stoi(qtyStr);
            h.avgCost = stod(costStr);
            h.purchaseDate = date;
            
            holdings[symbol] = h;
            
        } else if (section == "TRANSACTIONS") {
            // Store transaction as-is
            transactions.push_back(line);
        }
    }
    
    file.close();
    return true;
}