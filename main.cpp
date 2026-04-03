// main.cpp
#include <iostream>
#include <vector>
#include <map>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include "include/Stock.h"
#include "include/Portfolio.h"
#include "include/Analytics.h"
#include "include/Strategy.h"
#include "include/Backtester.h"
#include "include/DataFetcher.h"

using namespace std;
namespace fs = std::filesystem;

// Clear screen function
void clearScreen() {
    // #ifdef _WIN32
    //     system("cls");
    // #else
    //     system("clear");
    // #endif
}

// Pause and wait for user
void pauseScreen() {
    // cout << "\nPress Enter to continue...";
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');
    // cin.get();
}

// Auto-pause for N seconds
void autoPause(int seconds) {
    this_thread::sleep_for(chrono::seconds(seconds));
}

// Helper function: Load stock if not already loaded
bool loadStockIfNeeded(string symbol, map<string, Stock*>& stocks) {
    // Normalize symbol to uppercase
    for (char& c : symbol) c = toupper(c);
    
    // Already loaded?
    if (stocks.find(symbol) != stocks.end()) {
        return true;
    }
    
    // Check if CSV file exists
    string filename = "data/" + symbol + ".csv";
    
    if (!fs::exists(filename)) {
        cout << "✗ Error: " << symbol << " not found in data/ folder" << endl;
        return false;
    }
    
    // Check if CSV is Yahoo format (7 columns with Adj Close) — migrate to Stooq
    ifstream checkFile(filename);
    if (checkFile.is_open()) {
        string header;
        getline(checkFile, header);
        checkFile.close();
        if (header.find("Adj Close") != string::npos) {
            cout << "⚠ " << symbol << " is in old Yahoo format. Re-downloading from Stooq..." << endl;
            if (!DataFetcher::updateStock(symbol, true)) {
                cout << "✗ Failed to re-download " << symbol << ". Skipping." << endl;
                return false;
            }
        }
    }

    // Load the stock
    cout << "Loading " << symbol << "..." << endl;
    Stock* newStock = new Stock(symbol, symbol);
    
    if (newStock->loadFromCSV(filename)) {
        stocks[symbol] = newStock;
        cout << "✓ " << symbol << " loaded successfully!" << endl;
        return true;
    } else {
        cout << "✗ Failed to load " << symbol << endl;
        delete newStock;
        return false;
    }
}

// Helper function: Get date range from user
pair<int, int> getDateRange(Stock* stock) {
    int dataSize = stock->getDataSize();
    
    cout << "\n=== Select Time Period ===" << endl;
    cout << "1. Last 30 days" << endl;
    cout << "2. Last 90 days (3 months)" << endl;
    cout << "3. Last 180 days (6 months)" << endl;
    cout << "4. Last 365 days (1 year)" << endl;
    cout << "5. All time" << endl;
    cout << "6. Custom range" << endl;
    cout << "Enter choice: ";
    
    int choice;
    cin >> choice;
    
    int startDay = 0;
    int endDay = dataSize - 1;
    
    if (choice == 1) {
        startDay = max(0, endDay - 30);
    } else if (choice == 2) {
        startDay = max(0, endDay - 90);
    } else if (choice == 3) {
        startDay = max(0, endDay - 180);
    } else if (choice == 4) {
        startDay = max(0, endDay - 365);
    } else if (choice == 5) {
        startDay = 0;
    } else if (choice == 6) {
        cout << "Enter start day (0 to " << endDay << "): ";
        cin >> startDay;
        cout << "Enter end day (" << startDay << " to " << endDay << "): ";
        cin >> endDay;
        
        // Validate
        if (startDay < 0) startDay = 0;
        if (endDay >= dataSize) endDay = dataSize - 1;
        if (startDay > endDay) startDay = endDay;
    } else {
        cout << "Invalid choice. Using all time." << endl;
    }
    
    int numDays = endDay - startDay + 1;
    cout << "✓ Analyzing " << numDays << " days of data" << endl;
    
    return make_pair(startDay, endDay);
}

void displayMainMenu() {
    cout << "\n======================================" << endl;
    cout << "      QuantLab - Main Menu" << endl;
    cout << "======================================" << endl;
    cout << "1. Manage Portfolios" << endl;
    cout << "2. Load Stock Data" << endl;
    cout << "3. View Stock Info" << endl;
    cout << "4. View Indicators" << endl;
    cout << "5. View Analytics" << endl;
    cout << "6. Backtest Strategy" << endl;
    cout << "7. Update Stock Data" << endl;
    cout << "8. Exit" << endl;
    cout << "======================================" << endl;
    cout << "Enter choice: ";
}

void displayPortfolioMenu() {
    cout << "\n======================================" << endl;
    cout << "      Portfolio Management" << endl;
    cout << "======================================" << endl;
    cout << "1. Create new portfolio" << endl;
    cout << "2. View all portfolios" << endl;
    cout << "3. Select portfolio" << endl;
    cout << "4. Back to main menu" << endl;
    cout << "======================================" << endl;
    cout << "Enter choice: ";
}

void displaySelectedPortfolioMenu(string portfolioName) {
    cout << "\n======================================" << endl;
    cout << "  Portfolio: " << portfolioName << endl;
    cout << "======================================" << endl;
    cout << "1. Add cash" << endl;
    cout << "2. Buy stock" << endl;
    cout << "3. Sell stock" << endl;
    cout << "4. View holdings" << endl;
    cout << "5. View transactions" << endl;
    cout << "6. View summary" << endl;
    cout << "7. View performance analytics" << endl;
    cout << "8. Back" << endl;
    cout << "======================================" << endl;
    cout << "Enter choice: ";
}

int main() {
    map<string, Stock*> stocks;           // symbol -> Stock object
    vector<Portfolio*> portfolios;         // All portfolios
    
    cout << "\n*** Welcome to QuantLab ***\n" << endl;
    
    // Auto-load portfolios from portfolios/ folder
    cout << "Loading saved portfolios..." << endl;
    try {
        if (fs::exists("portfolios") && fs::is_directory("portfolios")) {
            for (const auto& entry : fs::directory_iterator("portfolios")) {
                if (entry.path().extension() == ".csv") {
                    Portfolio* p = new Portfolio("");
                    if (p->loadFromFile(entry.path().string())) {
                        portfolios.push_back(p);
                        cout << "✓ Loaded: " << p->getName() << endl;
                    } else {
                        delete p;
                    }
                }
            }
        }
    } catch (const exception& e) {
        cout << "Note: No saved portfolios found (this is normal for first run)" << endl;
    }
    
    if (portfolios.size() > 0) {
        cout << "\n" << portfolios.size() << " portfolio(s) loaded successfully!" << endl;
    }
    
    // Auto-load stocks from watchlist
    cout << "\nLoading watchlist stocks..." << endl;
    ifstream watchlistFile("watchlist.txt");
    
    if (watchlistFile.is_open()) {
        string symbol;
        int loadedCount = 0;
        
        while (getline(watchlistFile, symbol)) {
            // Trim whitespace
            symbol.erase(0, symbol.find_first_not_of(" \t\r\n"));
            symbol.erase(symbol.find_last_not_of(" \t\r\n") + 1);
            
            if (symbol.empty()) continue;
            
            if (loadStockIfNeeded(symbol, stocks)) {
                loadedCount++;
            }
        }
        
        watchlistFile.close();
        
        if (loadedCount > 0) {
            cout << "\n✓ " << loadedCount << " stock(s) loaded from watchlist!" << endl;
        }
    } else {
        cout << "Note: No watchlist.txt found. You can create one with stock symbols (one per line)." << endl;
    }
    
    // Auto-update watchlist stock data
    cout << "\nChecking for outdated stock data..." << endl;
    DataFetcher::updateWatchlist("watchlist.txt");
    // Reload any stocks that were updated
    for (auto& pair : stocks) {
        string filename = "data/" + pair.first + ".csv";
        if (fs::exists(filename)) {
            pair.second->loadFromCSV(filename);
        }
    }
    
    while (true) {
        clearScreen();
        displayMainMenu();
        int choice;
        cin >> choice;
        
        if (choice == 1) {
            // ===== PORTFOLIO MANAGEMENT =====
            while (true) {
                clearScreen();
                displayPortfolioMenu();
                int portfolioChoice;
                cin >> portfolioChoice;
                
                if (portfolioChoice == 1) {
                    // Create new portfolio
                    string name;
                    cout << "\nEnter portfolio name: ";
                    cin.ignore();
                    getline(cin, name);
                    
                    Portfolio* newPortfolio = new Portfolio(name);
                    portfolios.push_back(newPortfolio);
                    
                    // Save to file
                    string filename = "portfolios/" + name + ".csv";
                    // Replace spaces with underscores for filename
                    for (char& c : filename) {
                        if (c == ' ') c = '_';
                    }
                    newPortfolio->saveToFile(filename);
                    
                    cout << "✓ Portfolio '" << name << "' created and saved!" << endl;
                    
                } else if (portfolioChoice == 2) {
                    // View all portfolios
                    if (portfolios.empty()) {
                        cout << "\nNo portfolios yet. Create one first!" << endl;
                    } else {
                        cout << "\n=== Your Portfolios ===" << endl;
                        for (int i = 0; i < portfolios.size(); i++) {
                            cout << i + 1 << ". " << portfolios[i]->getName() 
                                 << " (Cash: $" << portfolios[i]->getCashBalance() << ")" << endl;
                        }
                    }
                    
                } else if (portfolioChoice == 3) {
                    // Select portfolio
                    if (portfolios.empty()) {
                        cout << "\nNo portfolios yet. Create one first!" << endl;
                        continue;
                    }
                    
                    cout << "\n=== Select Portfolio ===" << endl;
                    for (int i = 0; i < portfolios.size(); i++) {
                        cout << i + 1 << ". " << portfolios[i]->getName() << endl;
                    }
                    
                    int select;
                    cout << "Enter number: ";
                    cin >> select;
                    
                    if (select < 1 || select > portfolios.size()) {
                        cout << "Invalid selection." << endl;
                        continue;
                    }
                    
                    Portfolio* currentPortfolio = portfolios[select - 1];
                    
                    // Selected portfolio menu
                    while (true) {
                        clearScreen();
                        displaySelectedPortfolioMenu(currentPortfolio->getName());
                        int action;
                        cin >> action;
                        
                        if (action == 1) {
                            // Add cash
                            double amount;
                            cout << "Enter amount to add: $";
                            cin >> amount;
                            currentPortfolio->addCash(amount);
                            
                            // Save portfolio
                            string filename = "portfolios/" + currentPortfolio->getName() + ".csv";
                            for (char& c : filename) {
                                if (c == ' ') c = '_';
                            }
                            currentPortfolio->saveToFile(filename);
                            
                            pauseScreen();
                            
                        } else if (action == 2) {
                            // Buy stock
                            string symbol, date;
                            int quantity;
                            double price;
                            
                            cout << "Enter stock symbol: ";
                            cin >> symbol;
                            
                            // Validate and auto-load stock if needed
                            if (!loadStockIfNeeded(symbol, stocks)) {
                                cout << "Cannot buy " << symbol << ". Stock not available." << endl;
                                continue;
                            }
                            
                            cout << "Enter quantity: ";
                            cin >> quantity;
                            cout << "Enter price per share: $";
                            cin >> price;
                            cout << "Enter date (YYYY-MM-DD): ";
                            cin >> date;
                            
                            currentPortfolio->buyStock(symbol, quantity, price, date);
                            
                            // Save portfolio
                            string filename = "portfolios/" + currentPortfolio->getName() + ".csv";
                            for (char& c : filename) {
                                if (c == ' ') c = '_';
                            }
                            currentPortfolio->saveToFile(filename);
                            
                            pauseScreen();
                            
                        } else if (action == 3) {
                            // Sell stock
                            string symbol, date;
                            int quantity;
                            double price;
                            
                            cout << "Enter stock symbol: ";
                            cin >> symbol;
                            
                            // Validate stock (should already be loaded if they own it)
                            if (!loadStockIfNeeded(symbol, stocks)) {
                                cout << "Cannot sell " << symbol << ". Stock not available." << endl;
                                continue;
                            }
                            
                            cout << "Enter quantity: ";
                            cin >> quantity;
                            cout << "Enter price per share: $";
                            cin >> price;
                            cout << "Enter date (YYYY-MM-DD): ";
                            cin >> date;
                            
                            currentPortfolio->sellStock(symbol, quantity, price, date);
                            
                            // Save portfolio
                            string filename = "portfolios/" + currentPortfolio->getName() + ".csv";
                            for (char& c : filename) {
                                if (c == ' ') c = '_';
                            }
                            currentPortfolio->saveToFile(filename);
                            
                            pauseScreen();
                            
                        } else if (action == 4) {
                            // View holdings
                            currentPortfolio->displayHoldings();
                            pauseScreen();
                            
                        } else if (action == 5) {
                            // View transactions
                            currentPortfolio->displayTransactions();
                            pauseScreen();
                            
                        } else if (action == 6) {
                            // View summary
                            if (stocks.empty()) {
                                cout << "\n⚠ Warning: No stocks loaded. Load stocks for accurate current values." << endl;
                            }
                            currentPortfolio->displayDetailedSummary(stocks);
                            pauseScreen();
                            
                        } else if (action == 7) {
                            // View performance analytics
                            if (stocks.empty()) {
                                cout << "\n⚠ Warning: No stocks loaded. Load stocks for analytics." << endl;
                            }
                            currentPortfolio->displayPerformanceAnalytics(stocks);
                            pauseScreen();
                            
                        } else if (action == 8) {
                            // Back
                            break;
                            
                        } else {
                            cout << "Invalid choice." << endl;
                        }
                    }
                    
                } else if (portfolioChoice == 4) {
                    // Back to main menu
                    break;
                    
                } else {
                    cout << "Invalid choice." << endl;
                }
            }
            
        } else if (choice == 2) {
            // ===== LOAD STOCK DATA =====
            string symbol, name, filename;
            
            cout << "\nEnter stock symbol: ";
            cin >> symbol;
            cout << "Enter company name: ";
            cin.ignore();
            getline(cin, name);
            cout << "Enter CSV filename: ";
            getline(cin, filename);
            
            Stock* newStock = new Stock(symbol, name);
            
                            if (newStock->loadFromCSV(filename)) {
                stocks[symbol] = newStock;
                cout << "✓ Successfully loaded " << symbol << "!" << endl;
                pauseScreen();
            } else {
                cout << "✗ Failed to load stock." << endl;
                delete newStock;
                pauseScreen();
            }
            
        } else if (choice == 3) {
            // ===== VIEW STOCK INFO =====
            if (stocks.empty()) {
                cout << "\nNo stocks loaded yet." << endl;
            } else {
                cout << "\n=== Loaded Stocks ===" << endl;
                for (const auto& pair : stocks) {
                    cout << "- " << pair.first << endl;
                }
                
                string symbol;
                cout << "Enter symbol to view: ";
                cin >> symbol;
                
                if (stocks.find(symbol) != stocks.end()) {
                    stocks[symbol]->displaySummary();
                    
                    int days;
                    cout << "\nShow recent days (0 to skip): ";
                    cin >> days;
                    
                    if (days > 0) {
                        stocks[symbol]->displayRecentData(days);
                    }
                    pauseScreen();
                } else {
                    cout << "Stock not found." << endl;
                    pauseScreen();
                }
            }
            
        } else if (choice == 4) {
            // ===== VIEW INDICATORS =====
            if (stocks.empty()) {
                cout << "\nNo stocks loaded yet." << endl;
            } else {
                cout << "\n=== Loaded Stocks ===" << endl;
                for (const auto& pair : stocks) {
                    cout << "- " << pair.first << endl;
                }
                
                string symbol;
                cout << "Enter symbol: ";
                cin >> symbol;
                
                if (stocks.find(symbol) != stocks.end()) {
                    Stock* stock = stocks[symbol];
                    int dataSize = stock->getDataSize();
                    
                    cout << "\n=== Technical Indicators for " << symbol << " ===" << endl;
                    
                    // Submenu for indicators
                    cout << "\n1. View Moving Averages (SMA)" << endl;
                    cout << "2. View RSI" << endl;
                    cout << "3. View MACD" << endl;
                    cout << "4. View Bollinger Bands" << endl;
                    cout << "5. View Momentum" << endl;
                    cout << "6. View All Indicators Summary" << endl;
                    cout << "Enter choice: ";
                    
                    int indicatorChoice;
                    cin >> indicatorChoice;
                    
                    int numDays;
                    cout << "How many recent days to display? ";
                    cin >> numDays;
                    
                    int start = max(0, dataSize - numDays);
                    
                    if (indicatorChoice == 1) {
                        // SMA
                        cout << "\n=== Moving Averages ===" << endl;
                        cout << "Day\tSMA-20\t\tSMA-50" << endl;
                        cout << "------------------------------------" << endl;
                        for (int i = start; i < dataSize; i++) {
                            cout << i << "\t";
                            double sma20 = stock->getSMA20(i);
                            double sma50 = stock->getSMA50(i);
                            
                            if (sma20 > 0) cout << "$" << sma20;
                            else cout << "N/A";
                            cout << "\t\t";
                            
                            if (sma50 > 0) cout << "$" << sma50;
                            else cout << "N/A";
                            cout << endl;
                        }
                        
                    } else if (indicatorChoice == 2) {
                        // RSI
                        cout << "\n=== RSI (Relative Strength Index) ===" << endl;
                        cout << "Day\tRSI\t\tSignal" << endl;
                        cout << "------------------------------------" << endl;
                        for (int i = start; i < dataSize; i++) {
                            cout << i << "\t";
                            double rsi = stock->getRSI(i);
                            
                            if (rsi > 0) {
                                cout << rsi << "\t\t";
                                if (rsi > 70) cout << "Overbought";
                                else if (rsi < 30) cout << "Oversold";
                                else cout << "Neutral";
                            } else {
                                cout << "N/A\t\tN/A";
                            }
                            cout << endl;
                        }
                        
                    } else if (indicatorChoice == 3) {
                        // MACD
                        cout << "\n=== MACD ===" << endl;
                        cout << "Day\tMACD\t\tSignal\t\tHistogram\tTrend" << endl;
                        cout << "------------------------------------------------------------" << endl;
                        for (int i = start; i < dataSize; i++) {
                            cout << i << "\t";
                            double macd = stock->getMACD(i);
                            double signal = stock->getMACDSignal(i);
                            double hist = stock->getMACDHistogram(i);
                            
                            if (macd != 0) {
                                cout << macd << "\t" << signal << "\t" << hist << "\t\t";
                                if (hist > 0) cout << "Bullish";
                                else if (hist < 0) cout << "Bearish";
                            } else {
                                cout << "N/A\t\tN/A\t\tN/A\t\tN/A";
                            }
                            cout << endl;
                        }
                        
                    } else if (indicatorChoice == 4) {
                        // Bollinger Bands
                        cout << "\n=== Bollinger Bands ===" << endl;
                        cout << "Day\tUpper\t\tMiddle\t\tLower\t\tPosition" << endl;
                        cout << "------------------------------------------------------------" << endl;
                        for (int i = start; i < dataSize; i++) {
                            cout << i << "\t";
                            double upper = stock->getBollingerUpper(i);
                            double middle = stock->getBollingerMiddle(i);
                            double lower = stock->getBollingerLower(i);
                            
                            if (upper > 0) {
                                cout << "$" << upper << "\t$" << middle << "\t$" << lower << "\t";
                                // Could add price position relative to bands
                                cout << "---";
                            } else {
                                cout << "N/A\t\tN/A\t\tN/A\t\tN/A";
                            }
                            cout << endl;
                        }
                        
                    } else if (indicatorChoice == 5) {
                        // Momentum
                        cout << "\n=== Momentum (10-day) ===" << endl;
                        cout << "Day\tMomentum %\tTrend" << endl;
                        cout << "------------------------------------" << endl;
                        for (int i = start; i < dataSize; i++) {
                            cout << i << "\t";
                            double mom = stock->getMomentum(i);
                            
                            if (mom != 0) {
                                cout << mom << "%\t\t";
                                if (mom > 5) cout << "Strong Up";
                                else if (mom > 0) cout << "Up";
                                else if (mom > -5) cout << "Down";
                                else cout << "Strong Down";
                            } else {
                                cout << "N/A\t\tN/A";
                            }
                            cout << endl;
                        }
                        
                    } else if (indicatorChoice == 6) {
                        // All indicators summary
                        cout << "\n=== All Indicators Summary ===" << endl;
                        cout << "Day\tSMA20\tRSI\tMACD\tMomentum" << endl;
                        cout << "----------------------------------------------------" << endl;
                        for (int i = start; i < dataSize; i++) {
                            cout << i << "\t";
                            
                            double sma20 = stock->getSMA20(i);
                            if (sma20 > 0) cout << "$" << (int)sma20;
                            else cout << "N/A";
                            cout << "\t";
                            
                            double rsi = stock->getRSI(i);
                            if (rsi > 0) cout << (int)rsi;
                            else cout << "N/A";
                            cout << "\t";
                            
                            double macd = stock->getMACD(i);
                            if (macd != 0) cout << (int)macd;
                            else cout << "N/A";
                            cout << "\t";
                            
                            double mom = stock->getMomentum(i);
                            if (mom != 0) cout << (int)mom << "%";
                            else cout << "N/A";
                            
                            cout << endl;
                        }
                    }
                    
                } else {
                    cout << "Stock not found." << endl;
                }
            }
            
        } else if (choice == 5) {
            // ===== VIEW ANALYTICS =====
            if (stocks.empty()) {
                cout << "\nNo stocks loaded yet." << endl;
            } else {
                cout << "\n=== Loaded Stocks ===" << endl;
                for (const auto& pair : stocks) {
                    cout << "- " << pair.first << endl;
                }
                
                string symbol;
                cout << "Enter symbol: ";
                cin >> symbol;
                
                if (stocks.find(symbol) != stocks.end()) {
                    auto range = getDateRange(stocks[symbol]);
                    Analytics::displayAnalyticsReport(stocks[symbol], range.first, range.second);
                    pauseScreen();
                } else {
                    cout << "Stock not found." << endl;
                    pauseScreen();
                }
            }
            
        } else if (choice == 6) {
            // ===== BACKTEST STRATEGY =====
            if (stocks.empty()) {
                cout << "\nNo stocks loaded yet." << endl;
            } else {
                cout << "\n=== Loaded Stocks ===" << endl;
                for (const auto& pair : stocks) {
                    cout << "- " << pair.first << endl;
                }
                
                string symbol;
                cout << "Enter symbol: ";
                cin >> symbol;
                
                if (stocks.find(symbol) != stocks.end()) {
                    cout << "\n=== Select Strategy ===" << endl;
                    cout << "1. RSI Strategy (Buy < 30, Sell > 70)" << endl;
                    cout << "2. Moving Average Crossover" << endl;
                    cout << "3. MACD Strategy" << endl;
                    cout << "4. Momentum Strategy" << endl;
                    cout << "5. Buy and Hold" << endl;
                    cout << "6. Compare All Strategies" << endl;
                    cout << "Enter choice: ";
                    
                    int stratChoice;
                    cin >> stratChoice;
                    
                    double initialCash;
                    cout << "Enter starting cash: $";
                    cin >> initialCash;
                    
                    // Get date range
                    auto range = getDateRange(stocks[symbol]);
                    
                    if (stratChoice == 6) {
                        // Compare all strategies
                        cout << "\n=== COMPARING ALL STRATEGIES ===" << endl;
                        
                        Strategy* strategies[] = {
                            new RSIStrategy(),
                            new MAStrategy(),
                            new MACDStrategy(),
                            new MomentumStrategy(),
                            new BuyHoldStrategy()
                        };
                        
                        vector<double> returns;
                        vector<string> names;
                        
                        for (int i = 0; i < 5; i++) {
                            Backtester backtester(stocks[symbol], strategies[i], initialCash, range.first, range.second);
                            backtester.run();
                            returns.push_back(backtester.getTotalReturn());
                            names.push_back(strategies[i]->getName());
                        }
                        
                        // Display comparison table
                        cout << "\n========================================" << endl;
                        cout << "    STRATEGY COMPARISON" << endl;
                        cout << "========================================" << endl;
                        cout << "Stock: " << symbol << endl;
                        cout << "Starting Capital: $" << initialCash << endl;
                        cout << "----------------------------------------" << endl;
                        cout << fixed << setprecision(2);
                        
                        // Find best strategy
                        int bestIdx = 0;
                        double bestReturn = returns[0];
                        for (int i = 1; i < returns.size(); i++) {
                            if (returns[i] > bestReturn) {
                                bestReturn = returns[i];
                                bestIdx = i;
                            }
                        }
                        
                        for (int i = 0; i < names.size(); i++) {
                            cout << "\n" << i + 1 << ". " << names[i] << endl;
                            cout << "   Return: " << returns[i] << "%";
                            if (i == bestIdx) {
                                cout << " ⭐ BEST";
                            }
                            cout << endl;
                            cout << "   Final Value: $" << (initialCash * (1 + returns[i]/100.0)) << endl;
                        }
                        
                        cout << "\n========================================" << endl;
                        cout << "Best Strategy: " << names[bestIdx] << endl;
                        cout << "Best Return: " << bestReturn << "%" << endl;
                        cout << "========================================" << endl;
                        
                        pauseScreen();
                        
                        // Clean up
                        for (int i = 0; i < 5; i++) {
                            delete strategies[i];
                        }
                        
                    } else {
                        // Single strategy
                        Strategy* strategy = nullptr;
                        
                        if (stratChoice == 1) {
                            strategy = new RSIStrategy();
                        } else if (stratChoice == 2) {
                            strategy = new MAStrategy();
                        } else if (stratChoice == 3) {
                            strategy = new MACDStrategy();
                        } else if (stratChoice == 4) {
                            strategy = new MomentumStrategy();
                        } else if (stratChoice == 5) {
                            strategy = new BuyHoldStrategy();
                        } else {
                            cout << "Invalid choice." << endl;
                            continue;
                        }
                        
                        // Run backtest
                        Backtester backtester(stocks[symbol], strategy, initialCash, range.first, range.second);
                        backtester.run();
                        backtester.displayResults();
                        
                        pauseScreen();
                        
                        delete strategy;
                    }
                } else {
                    cout << "Stock not found." << endl;
                }
            }
            
        } else if (choice == 7) {
            // ===== UPDATE STOCK DATA =====
            cout << "\n=== Update Stock Data ===" << endl;
            cout << "1. Update watchlist stocks" << endl;
            cout << "2. Update specific stock by symbol" << endl;
            cout << "Enter choice: ";
            
            int updateChoice;
            cin >> updateChoice;
            
            if (updateChoice == 1) {
                // Update all watchlist stocks
                DataFetcher::updateWatchlist("watchlist.txt");
                // Reload any already-loaded stocks with fresh data
                for (auto& pair : stocks) {
                    string filename = "data/" + pair.first + ".csv";
                    if (fs::exists(filename)) {
                        pair.second->loadFromCSV(filename);
                    }
                }
                pauseScreen();
                
            } else if (updateChoice == 2) {
                // Update specific stock by symbol
                string symbol;
                cout << "\nEnter stock symbol: ";
                cin >> symbol;
                
                if (DataFetcher::updateStock(symbol)) {
                    // If already loaded in memory, reload with fresh data
                    if (stocks.find(symbol) != stocks.end()) {
                        stocks[symbol]->loadFromCSV("data/" + symbol + ".csv");
                        cout << "✓ " << symbol << " reloaded with fresh data!" << endl;
                    } else {
                        // Not loaded yet — offer to load it
                        char load;
                        cout << "Load " << symbol << " into memory? (y/n): ";
                        cin >> load;
                        if (load == 'y' || load == 'Y') {
                            loadStockIfNeeded(symbol, stocks);
                        }
                    }
                }
                pauseScreen();
                
            } else {
                cout << "Invalid choice." << endl;
            }
            
        } else if (choice == 8) {
            // ===== EXIT =====
            cout << "\nThank you for using QuantLab!" << endl;
            
            // Clean up memory
            for (auto& pair : stocks) {
                delete pair.second;
            }
            for (Portfolio* p : portfolios) {
                delete p;
            }
            
            break;
            
        } else {
            cout << "Invalid choice." << endl;
        }
    }
    
    return 0;
}