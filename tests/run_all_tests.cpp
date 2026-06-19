#include "test_framework.h"
#include <iostream>

// --- Forward declarations cua tung module test ---
void runRecordTests();
void runTaskTests();
void runTaskManagerTests();
void runTransactionTests();
void runWalletTests();
void runBudgetTests();
void runEncryptionTests();
void runAuthTests();
void runInputValidatorTests();
void runPolymorphismTests();

int main() {
    std::cout << "\n";
    std::cout << "=================================================\n";
    std::cout << "         OPEP UNIT TEST SUITE v1.0              \n";
    std::cout << "         Nhom do an OOP C++ - OPEP              \n";
    std::cout << "=================================================\n";

    // Chay lan luot tung nhom test
    runRecordTests();
    runTaskTests();
    runTaskManagerTests();
    runTransactionTests();
    runWalletTests();
    runBudgetTests();
    runEncryptionTests();
    runAuthTests();
    runInputValidatorTests();
    runPolymorphismTests();

    // In tong ket cuoi
    printTestSummary();

    // Return non-zero neu co bat ky test nao that bai
    return (testFailed() > 0) ? 1 : 0;
}
